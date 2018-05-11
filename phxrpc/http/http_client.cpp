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
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "http_client.h"
#include "http_msg.h"
#include "http_protocol.h"
#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"


namespace phxrpc {


int HttpClient::Get(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp) {
    ReturnCode ret{HttpProtocol::SendReqHeader(socket, "GET", req)};

    if (ReturnCode::OK == ret) {
        ret = HttpProtocol::RecvRespStartLine(socket, resp);
        if (ReturnCode::OK == ret)
            ret = HttpProtocol::RecvHeaders(socket, resp);
        if (ReturnCode::OK == ret && SC_NOT_MODIFIED != resp->GetStatusCode()) {
            ret = HttpProtocol::RecvBody(socket, resp);
        }
    }

    return static_cast<int>(ret);
}

int HttpClient::Post(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp) {
    PostStat stat;
    int ret{Post(socket, req, resp, &stat)};
    return ret;
}

int HttpClient::Post(BaseTcpStream &socket, const HttpRequest &req, HttpResponse *resp,
                     PostStat *post_stat) {
    ReturnCode ret{HttpProtocol::SendReqHeader(socket, "POST", req)};

    if (ReturnCode::OK == ret) {
        socket << req.GetContent();
        if(!socket.flush().good())
            ret = static_cast<ReturnCode>(socket.LastError());
    } else {
        if (ReturnCode::ERROR_SOCKET_STREAM_NORMAL_CLOSED != ret) {
            post_stat->send_error_ = true;
            phxrpc::log(LOG_ERR, "ERR: sendReqHeader fail");
        }
        return static_cast<int>(ret);
    }

    if (ReturnCode::OK == ret) {
        ret = HttpProtocol::RecvRespStartLine(socket, resp);
        if (ReturnCode::OK == ret)
            ret = HttpProtocol::RecvHeaders(socket, resp);

        if (ReturnCode::OK == ret && SC_NOT_MODIFIED != resp->GetStatusCode()) {
            ret = HttpProtocol::RecvBody(socket, resp);
        }

        if (ReturnCode::OK != ret && ReturnCode::ERROR_SOCKET_STREAM_NORMAL_CLOSED != ret) {
            post_stat->recv_error_ = true;
        }
    } else {
        if (ReturnCode::ERROR_SOCKET_STREAM_NORMAL_CLOSED != ret) {
            post_stat->send_error_ = true;
            phxrpc::log(LOG_ERR, "ERR: sendReqBody fail");
        }
    }

    return static_cast<int>(ret);
}

int HttpClient::Head(BaseTcpStream & socket, const HttpRequest &req, HttpResponse *resp) {
    ReturnCode ret{HttpProtocol::SendReqHeader(socket, "HEAD", req)};

    if (ReturnCode::OK == ret)
        ret = HttpProtocol::RecvRespStartLine(socket, resp);

    if (ReturnCode::OK == ret)
        ret = HttpProtocol::RecvHeaders(socket, resp);

    return static_cast<int>(ret);
}


}  // namespace phxrpc

