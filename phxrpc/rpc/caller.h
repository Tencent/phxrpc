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

#include "phxrpc/rpc/client_monitor.h"

#include "phxrpc/msg.h"


namespace phxrpc {


class BaseTcpStream;

class Caller {
  public:
    Caller(BaseTcpStream &socket, ClientMonitor &client_monitor,
           BaseMessageHandlerFactory &msg_handler_factory);

    virtual ~Caller();

    BaseRequest *GetRequest();

    BaseResponse *GetResponse();

    int Call(const google::protobuf::Message &req,
             google::protobuf::Message *resp);

    void set_uri(const char *const uri, const int cmd_id);

    void set_keep_alive(const bool keep_alive);

  protected:
    void MonitorReport(ClientMonitor &client_monitor, bool send_error,
                       bool recv_error, size_t send_size, size_t recv_size,
                       uint64_t call_begin, uint64_t call_end);

    BaseTcpStream &socket_;
    ClientMonitor &client_monitor_;
    int cmd_id_;
    std::string uri_;
    bool keep_alive_{false};

    std::unique_ptr<BaseRequest> req_;
    std::unique_ptr<BaseResponse> resp_;

    BaseMessageHandlerFactory &msg_handler_factory_;
};


}  // namespace phxrpc

