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

namespace phxrpc {

class BaseTcpStream;

class HttpMessage;
class HttpRequest;
class HttpResponse;

class HttpProto {
 public:
    enum {
        MAX_RECV_LEN = 8192
    };

    static void URLEncode(const char * source, char * dest, size_t length);

    static char * strsep(char ** s, const char * del);

    // @return 0: socket ok, !=0: socket error
    static int SendResp(BaseTcpStream & socket, const HttpResponse & resp);

    // @return 0: socket ok, !=0: socket error
    static int SendReqHeader(BaseTcpStream & socket, const char * method, const HttpRequest & req);

 public:

    static void FixRespHeaders(const HttpRequest & req, HttpResponse * resp);

    static void FixRespHeaders(bool is_keep_alive, const char * version, HttpResponse * resp);

    // @return 0: socket ok, !=0: socket error
    static int RecvReq(BaseTcpStream & socket, HttpRequest * req);

    // @return 0: socket ok, !=0: socket error
    static int RecvRespStartLine(BaseTcpStream & socket, HttpResponse * resp);

    // @return 0: socket ok, !=0: socket error
    static int RecvReqStartLine(BaseTcpStream & socket, HttpRequest * req);

    // @return 0: socket ok, !=0: socket error
    static int RecvHeaders(BaseTcpStream & socket, HttpMessage * msg);

    // @return 0: socket ok, !=0: socket error
    static int RecvBody(BaseTcpStream & socket, HttpMessage * msg);

 private:
    HttpProto();
};

}

