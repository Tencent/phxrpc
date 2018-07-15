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

#include "phxrpc/rpc/caller.h"

#include <syslog.h>

#include <google/protobuf/message_lite.h>

#include "monitor_factory.h"

#include "phxrpc/network.h"
#include "phxrpc/file.h"


namespace phxrpc {


Caller::Caller(BaseTcpStream &socket, ClientMonitor &client_monitor,
               BaseMessageHandlerFactory &msg_handler_factory)
        : socket_(socket), client_monitor_(client_monitor), cmd_id_(-1),
          msg_handler_factory_(msg_handler_factory) {
}

Caller::~Caller() {
}

BaseRequest *Caller::GetRequest() {
    return req_.get();
}

BaseResponse *Caller::GetResponse() {
    return resp_.get();
}

void Caller::MonitorReport(ClientMonitor &client_monitor, bool send_error,
                           bool recv_error, size_t send_size,
                           size_t recv_size, uint64_t call_begin,
                           uint64_t call_end) {
    if (send_error) {
        client_monitor.SendError();
    }

    if (recv_error) {
        client_monitor.RecvError();
    }

    client_monitor.SendBytes(send_size);
    client_monitor.RecvBytes(recv_size);
    client_monitor.RequestCost(call_begin, call_end);
    if (0 < cmd_id_) {
        client_monitor.ClientCall(cmd_id_, req_->uri());
    }
}

int Caller::Call(const google::protobuf::Message &req,
                 google::protobuf::Message *resp) {
    auto msg_handler(msg_handler_factory_.Create());
    BaseRequest *tmp_req{nullptr};
    int ret{msg_handler->GenRequest(tmp_req)};
    if (0 != ret || !tmp_req) {
        log(LOG_ERR, "GenRequest err %d", ret);

        return -1;
    }
    req_.reset(tmp_req);

    ret = req_->FromPb(req);
    if (0 != ret) {
        log(LOG_ERR, "FromPb err %d", ret);

        return ret;
    }

    req_->set_uri(uri_.c_str());
    req_->set_keep_alive(keep_alive_);

    bool send_error{false}, recv_error{false};
    uint64_t call_begin{Timer::GetSteadyClockMS()};
    ret = req_->Send(socket_);
    if (0 != ret && SocketStreamError_Normal_Closed != ret) {
        send_error = true;
        log(LOG_ERR, "Send err %d", ret);
    }

    if (0 == ret) {
        BaseResponse *tmp_resp{nullptr};
        ret = msg_handler->RecvResponse(socket_, tmp_resp);
        if ((0 != ret && SocketStreamError_Normal_Closed != ret) || !tmp_resp) {
            recv_error = true;
            log(LOG_ERR, "RecvResponse err %d", ret);
        }
        resp_.reset(tmp_resp);
    }
    MonitorReport(client_monitor_, send_error,
                  recv_error, req_->size(),
                  resp_ ? resp_->size() : 0, call_begin,
                  Timer::GetSteadyClockMS());

    if (0 != ret) {
        log(LOG_ERR, "call err %d", ret);

        return ret;
    }

    ret = resp_->ToPb(resp);
    if (0 != ret) {
        log(LOG_ERR, "ToPb err %d", ret);

        return ret;
    }

    ret = resp_->result();
    if (0 > ret) {
        log(LOG_ERR, "call %s err %d", req_->uri(), ret);
    }

    return ret;
}

void Caller::set_uri(const char *const uri, const int cmd_id) {
    cmd_id_ = cmd_id;
    uri_ = uri;
}

void Caller::set_keep_alive(const bool keep_alive) {
    keep_alive_ = keep_alive;
}


}  // namespace phxrpc

