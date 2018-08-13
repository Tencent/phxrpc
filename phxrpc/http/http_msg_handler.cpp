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

#include "phxrpc/http/http_msg_handler.h"

#include "phxrpc/file/log_utils.h"
#include "phxrpc/http/http_msg.h"
#include "phxrpc/http/http_protocol.h"
#include "phxrpc/network/socket_stream_base.h"


namespace phxrpc {


using namespace std;


int HttpMessageHandler::RecvRequest(BaseTcpStream &socket, BaseRequest *&req) {
    HttpRequest *http_req{new HttpRequest};

    int ret{HttpProtocol::RecvReq(socket, http_req)};
    if (0 == ret) {
        req_ = req = http_req;
        version_ = (http_req->version() != nullptr ? http_req->version() : "");
        keep_alive_ = http_req->keep_alive();
    } else {
        delete http_req;
        http_req = nullptr;
    }

    return ret;
}

int HttpMessageHandler::RecvResponse(BaseTcpStream &socket, BaseResponse *&resp) {
    HttpResponse *http_resp{new HttpResponse};

    int ret{HttpProtocol::RecvResp(socket, http_resp)};
    if (0 == ret) {
        resp = http_resp;
    } else {
        delete http_resp;
        http_resp = nullptr;
    }

    return ret;
}

int HttpMessageHandler::GenRequest(BaseRequest *&req) {
    req = new HttpRequest;

    return 0;
}

int HttpMessageHandler::GenResponse(BaseResponse *&resp) {
    resp = req_->GenResponse();
    resp->Modify(keep_alive_, version_);

    return 0;
}

bool HttpMessageHandler::keep_alive() const {
    return keep_alive_;
}


}  // namespace phxrpc

