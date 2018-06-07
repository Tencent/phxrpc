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

#include "client_monitor.h"
#include "phxrpc/http.h"


namespace google {

namespace protobuf {


class MessageLite;


}

}


namespace phxrpc {


class BaseTcpStream;

class HttpCaller {
  public:
    HttpCaller(BaseTcpStream &socket, ClientMonitor &client_monitor);

    virtual ~HttpCaller();

    HttpRequest &GetRequest();

    HttpResponse &GetResponse();

    int Call(const google::protobuf::MessageLite &req,
             google::protobuf::MessageLite *resp);

    void SetURI(const char *const uri, const int cmdid);

    void SetKeepAlive(const bool keep_alive);

  private:
    void MonitorReport(phxrpc::ClientMonitor &client_monitor, bool send_error,
                       bool recv_error, size_t send_size, size_t recv_size,
                       uint64_t call_begin, uint64_t call_end);

    BaseTcpStream &socket_;
    ClientMonitor &client_monitor_;
    int cmd_id_;

    HttpRequest req_;
    HttpResponse resp_;
};


}  // namespace phxrpc

