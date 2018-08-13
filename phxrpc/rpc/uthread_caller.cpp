/*
Tencent is pleased to support the open source community by making
PhxRPC available.
Copyright (C) 2016 THL A29 Limited, a Tencent company.
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may
not use this file except in compliance with the License. You may
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.

See the AUTHORS file for names of contributors.
*/

#include <cassert>

#include "caller.h"
#include "client_monitor.h"
#include "uthread_caller.h"

#include "phxrpc/network.h"


namespace phxrpc {


using namespace std;


UThreadCaller::UThreadCaller(UThreadEpollScheduler *uthread_scheduler,
                             google::protobuf::Message &request,
                             google::protobuf::Message *response,
                             ClientMonitor &client_monitor,
                             BaseMessageHandlerFactory &msg_handler_factory,
                             const string &uri, const int cmd_id, const Endpoint_t &ep,
                             const int connect_timeout_ms, const int socket_timeout_ms,
                             UThreadCallback callback, void *args)
        : uthread_scheduler_(uthread_scheduler),
          request_(&request),
          response_(response),
          client_monitor_(client_monitor),
          msg_handler_factory_(msg_handler_factory),
          uri_(uri),
          cmd_id_(cmd_id),
          ep_(ep),
          mconnect_timeout_ms(connect_timeout_ms),
          msocket_timeout_ms(socket_timeout_ms),
          call_ret_(-1),
          callback_(callback),
          args_(args) {
}

UThreadCaller::~UThreadCaller() {
}

google::protobuf::Message &UThreadCaller::GetRequest() {
    return *request_;
}

google::protobuf::Message *UThreadCaller::GetResponse() {
    return response_;
}

const string &UThreadCaller::uri() {
    return uri_;
}

int UThreadCaller::GetCmdID() {
    return cmd_id_;
}

UThreadEpollScheduler *UThreadCaller::Getuthread_scheduler() {
    return uthread_scheduler_;
}

Endpoint_t *UThreadCaller::GetEP() {
    return &ep_;
}

const int UThreadCaller::GetRet() {
    return call_ret_;
}

void UThreadCaller::SetRet(const int ret) {
    call_ret_ = ret;
}

void UThreadCaller::Callback() {
    if (nullptr != callback_) {
        callback_(this, args_);
    }
}

void UThreadCaller::Call(void *args) {
    UThreadCaller *uthread_caller = (UThreadCaller *)args;

    UThreadTcpStream socket;
    Endpoint_t *ep = uthread_caller->GetEP();
    bool open_ret = phxrpc::UThreadTcpUtils::Open(
            uthread_caller->Getuthread_scheduler(), &socket, ep->ip, ep->port,
            uthread_caller->mconnect_timeout_ms);
    if (open_ret) {
        socket.SetTimeout(uthread_caller->msocket_timeout_ms);
        phxrpc::Caller caller(socket, uthread_caller->client_monitor_,
                              uthread_caller->msg_handler_factory_);
        caller.GetRequest()->set_uri(uthread_caller->uri().c_str());
        uthread_caller->SetRet(caller.Call(uthread_caller->GetRequest(),
                                           uthread_caller->GetResponse()));
    } else {
        uthread_caller->SetRet(-1);
    }
    uthread_caller->client_monitor_.ClientConnect(open_ret);

    uthread_caller->Callback();
}

void UThreadCaller::Close() {
    uthread_scheduler_->Close();
}

///////////////////////////////////////////////////////////////////

UThreadMultiCaller::UThreadMultiCaller(ClientMonitor &client_monitor,
                                       BaseMessageHandlerFactory &msg_handler_factory)
        : uthread_scheduler_(64 * 1024, 300), client_monitor_(client_monitor),
          msg_handler_factory_(msg_handler_factory) {
}

UThreadMultiCaller::~UThreadMultiCaller() {
    for (size_t i{0}; i < uthread_caller_list_.size(); ++i) {
        if (nullptr != uthread_caller_list_[i]) {
            delete uthread_caller_list_[i];
        }
    }

    uthread_caller_list_.clear();
}

const int UThreadMultiCaller::GetRet(size_t index) {
    if (index >= uthread_caller_list_.size()) {
        return -1;
    }
    return uthread_caller_list_[index]->GetRet();
}

void UThreadMultiCaller::AddCaller(google::protobuf::Message &request,
                                   google::protobuf::Message *response,
                                   const string &uri, const int cmd_id, const Endpoint_t &ep,
                                   const int connect_timeout_ms, const int socket_timeout_ms,
                                   UThreadCallback callback, void *args) {
    UThreadCaller *caller = new UThreadCaller(&uthread_scheduler_,
            request, response, client_monitor_, msg_handler_factory_, uri, cmd_id, ep,
            connect_timeout_ms, socket_timeout_ms, callback, args);
    assert(nullptr != caller);
    uthread_caller_list_.push_back(caller);

    uthread_scheduler_.AddTask(UThreadCaller::Call, (void *)caller);
}

void UThreadMultiCaller::MultiCall() {
    uthread_scheduler_.Run();
}


}


