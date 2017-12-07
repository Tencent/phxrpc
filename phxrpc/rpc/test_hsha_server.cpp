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

#include <cstdio>

#include "phxrpc/rpc.h"


using namespace phxrpc;


void Dispatch(const BaseRequest *req, BaseResponse *resp, void *args) {
    printf("dispatch args %p\n", args);
    resp->SetPhxRpcResult(0);
}

int main(int argc, char **argv) {
    HshaServerConfig config;
    config.SetBindIP("127.0.0.1");
    config.SetPort(26161);
    config.SetMaxThreads(2);
    //config.SetLogDir("~/log");
    //config.SetLogLevel(3);

    printf("args %p\n", &config);

    phxrpc::openlog(argv[0], config.GetLogDir(), config.GetLogLevel());

    HshaServer server(config, Dispatch, &config);
    server.RunForever();

    phxrpc::closelog();

    return 0;
}

