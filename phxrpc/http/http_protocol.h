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

#include "phxrpc/msg/base_protocol.h"


namespace phxrpc {


class BaseTcpStream;

class HttpMessage;
class HttpRequest;
class HttpResponse;

class HttpProtocol : public BaseProtocol {
  public:
    enum {
        MAX_RECV_LEN = 8192
    };

    HttpProtocol() = default;
    virtual ~HttpProtocol() override = default;

    static void FixRespHeaders(const HttpRequest &req, HttpResponse *resp);

    static void FixRespHeaders(bool is_keep_alive, const char *version, HttpResponse *resp);

    static ReturnCode SendReqHeader(BaseTcpStream &socket, const char *method, const HttpRequest &req);

    static ReturnCode RecvRespStartLine(BaseTcpStream &socket, HttpResponse *resp);

    static ReturnCode RecvReqStartLine(BaseTcpStream &socket, HttpRequest *req);

    static ReturnCode RecvHeaders(BaseTcpStream &socket, HttpMessage *msg);

    static ReturnCode RecvBody(BaseTcpStream &socket, HttpMessage *msg);

    static ReturnCode RecvReq(BaseTcpStream &socket, HttpRequest *req);

    virtual ReturnCode ServerRecv(BaseTcpStream &socket,
                                  BaseRequest *&req) override;
};


}  // namespace phxrpc

