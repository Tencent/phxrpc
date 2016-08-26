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

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "http_client.h"
#include "http_msg.h"
#include "http_proto.h"
#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"

namespace phxrpc {

int HttpClient::Get(BaseTcpStream & socket, const HttpRequest & req, HttpResponse * resp ) {
    int socket_ret = HttpProto::SendReqHeader(socket, "GET", req);

    if (socket_ret == 0) {
        socket_ret = HttpProto::RecvRespStartLine(socket, resp);
        if (socket_ret == 0)
            socket_ret = HttpProto::RecvHeaders(socket, resp);
        if (socket_ret == 0 && SC_NOT_MODIFIED != resp->GetStatusCode()) {
            socket_ret = HttpProto::RecvBody(socket, resp);
        }
    }

    return socket_ret;
}

int HttpClient::Post(BaseTcpStream & socket, const HttpRequest & req, HttpResponse * resp) {
    PostStat stat;
    int ret = Post( socket, req, resp, &stat );
    return ret; 
}

int HttpClient::Post(BaseTcpStream & socket, const HttpRequest & req, HttpResponse * resp,
                      PostStat * post_stat ) {
    int socket_ret = HttpProto::SendReqHeader(socket, "POST", req);

    if (socket_ret == 0) {
        socket << req.GetContent();
        socket_ret = socket.flush().good() ? 0 : socket.LastError();
    } else {
        if (socket_ret != SocketStreamError_Normal_Closed) {
            post_stat->send_error_ = true;
            phxrpc::log(LOG_ERR, "ERR: sendReqHeader fail");
        }
        return socket_ret;
    }

    if (socket_ret == 0) {
        socket_ret = HttpProto::RecvRespStartLine(socket, resp);
        if (socket_ret == 0) 
            socket_ret = HttpProto::RecvHeaders(socket, resp);

        if (socket_ret == 0 && SC_NOT_MODIFIED != resp->GetStatusCode()) {
            socket_ret = HttpProto::RecvBody(socket, resp);
        }

        if (socket_ret != 0 && socket_ret != SocketStreamError_Normal_Closed) {
            post_stat->recv_error_ = true;
        }
    } else {
        if (socket_ret != SocketStreamError_Normal_Closed) {
            post_stat->send_error_ = true;
            phxrpc::log(LOG_ERR, "ERR: sendReqBody fail");
        }
    }

    return socket_ret;
}

int HttpClient::Head(BaseTcpStream & socket, const HttpRequest & req, HttpResponse * resp) {
    int socket_ret = HttpProto::SendReqHeader(socket, "HEAD", req);

    if (socket_ret == 0)
        socket_ret = HttpProto::RecvRespStartLine(socket, resp);

    if (socket_ret == 0)
        socket_ret = HttpProto::RecvHeaders(socket, resp);

    return socket_ret;
}

}
