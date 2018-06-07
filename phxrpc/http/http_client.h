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
class HttpRequest;
class HttpResponse;
class ClientMonitor;

class HttpClient {
  public:
    struct PostStat {
        bool send_error_{false};
        bool recv_error_{false};

        PostStat() = default;

        PostStat(bool send_error, bool recv_error)
                : send_error_(send_error), recv_error_(recv_error) {
        }
    };

    enum {
        SC_NOT_MODIFIED = 304
    };

    // @return true : socket ok, false : socket error
    static int Get(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp);

    // @return true : socket ok, false : socket error
    static int Post(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp,
                    PostStat *post_stat);
    static int Post(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp);

    // @return true : socket ok, false : socket error
    static int Head(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp);

  private:
    HttpClient();
};


}  // namespace phxrpc

