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

#pragma once

#include <vector>
#include <string>

#include "phxrpc/network.h"

#include "client_config.h"


namespace google {

namespace protobuf {


class Message;


}

}


namespace phxrpc {


class BaseMessageHandlerFactory;
class ClientMonitor;
class UThreadCaller;
class UThreadEpollScheduler;

typedef void (*UThreadCallback)(UThreadCaller *caller, void *args);

class UThreadCaller {
  public:
    UThreadCaller(UThreadEpollScheduler *uthread_scheduler,
                  google::protobuf::Message &request,
                  google::protobuf::Message *response,
                  ClientMonitor &client_monitor,
                  BaseMessageHandlerFactory &msg_handler_factory,
                  const std::string &uri, const int cmd_id, const Endpoint_t &ep,
                  const int connect_timeout_ms, const int socket_timeout_ms,
                  UThreadCallback callback, void *args);
    virtual ~UThreadCaller();

    void Close();

    virtual google::protobuf::Message &GetRequest();
    virtual google::protobuf::Message *GetResponse();
    const std::string &uri();
    int GetCmdID();
    UThreadEpollScheduler *Getuthread_scheduler();
    Endpoint_t *GetEP();

    const int GetRet();
    void SetRet(const int ret);

    void Callback();

    static void Call(void *args);

    int mconnect_timeout_ms;
    int msocket_timeout_ms;

  private:
    UThreadEpollScheduler *uthread_scheduler_;
    google::protobuf::Message *request_;
    google::protobuf::Message *response_;
    ClientMonitor &client_monitor_;
    BaseMessageHandlerFactory &msg_handler_factory_;
    std::string uri_;
    int cmd_id_;
    Endpoint_t ep_;

    int call_ret_;
    UThreadCallback callback_;
    void *args_;
};

///////////////////////////////////////////////////////

class UThreadMultiCaller {
  public:
    UThreadMultiCaller(ClientMonitor &client_monitor,
                       BaseMessageHandlerFactory &msg_handler_factory);
    virtual ~UThreadMultiCaller();

    void AddCaller(google::protobuf::Message &request,
                   google::protobuf::Message *response,
                   const std::string &uri, const int cmd_id, const Endpoint_t &ep,
                   const int connect_timeout_ms, const int socket_timeout_ms,
                   UThreadCallback callback = nullptr, void *args = nullptr);

    void MultiCall();

    const int GetRet(size_t index);

  private:
    UThreadEpollScheduler uthread_scheduler_;
    std::vector<UThreadCaller *> uthread_caller_list_;
    ClientMonitor &client_monitor_;
    BaseMessageHandlerFactory &msg_handler_factory_;
};


}

