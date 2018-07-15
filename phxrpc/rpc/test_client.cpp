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

#include <stdio.h>
#include <syslog.h>
#include <google/protobuf/message_lite.h>

#include "caller.h"
#include "monitor_factory.h"

#include "phxrpc/network.h"
#include "phxrpc/http.h"


using namespace phxrpc;


int main(int argc, char **argv) {
    for (size_t i{0}; 20 > i; ++i) {
        phxrpc::BlockTcpStream socket;
        if(phxrpc::BlockTcpUtils::Open(&socket, "127.0.0.1", 26161, 200, nullptr, 0)) {
            socket.SetTimeout(5000);
            HttpRequest request;
            HttpResponse response;

            *(request.mutable_content()) = "hello grpc";
            request.set_uri("abc");
            request.AddHeader(HttpMessage::HEADER_CONTENT_LENGTH, request.GetContent().size());
            int ret = HttpClient::Post(socket, request, &response);
            if (ret != 0) {
                printf("post fail, %zu, ret %d\n", i, ret);
                continue;
            }

            const char *result = response.GetHeaderValue(HttpMessage::HEADER_X_PHXRPC_RESULT);
            ret = atoi(nullptr == result ? "-1" : result);
            printf("post ret %d\n", ret);
        }
    }

    return 0;
}

