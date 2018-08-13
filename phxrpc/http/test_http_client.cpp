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
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "phxrpc/comm.h"
#include "phxrpc/file/file_utils.h"
#include "phxrpc/file/opt_map.h"
#include "phxrpc/http/http_client.h"
#include "phxrpc/http/http_msg.h"
#include "phxrpc/network/socket_stream_block.h"


using namespace phxrpc;


void ShowUsage(const char *program) {
    printf("\n%s [-h host] [-p port] [-r POST|GET] [-u URI] [-f file] [-v]\n", program);

    printf("\t-h http host\n");
    printf("\t-p http port\n");
    printf("\t-r http method, only support POST/GET/HEAD\n");
    printf("\t-u http URI\n");
    printf("\t-f the file for POST body, only need for POST\n");
    printf("\t-v show this usage\n");
    printf("\n");

    exit(0);
}

int main(int argc, char **argv) {

    PHXRPC_ASSERT(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    OptMap optMap("h:p:r:u:f:ov");

    if ((!optMap.Parse(argc, argv)) || optMap.Has('v'))
        ShowUsage(argv[0]);

    int port{0};
    const char *host{optMap.Get('h')};
    const char *method{optMap.Get('r')};
    const char *uri{optMap.Get('u')};
    const char *file{optMap.Get('f')};

    if ((nullptr == host) || (!optMap.GetInt('p', &port))) {
        printf("\nPlease specify host and port!\n");
        ShowUsage(argv[0]);
    }

    if (nullptr == method || nullptr == uri) {
        printf("\nPlease specify URI and method!\n");
        ShowUsage(argv[0]);
    }

    if (0 == strcasecmp(method, "POST") && nullptr == file) {
        printf("\nPlease specify the file for POST body!\n");
        ShowUsage(argv[0]);
    }

    HttpRequest request;
    request.set_uri(uri);
    request.set_version("HTTP/1.1");
    request.set_method(method);
    request.AddHeader("Connection", "Keep-Alive");
    request.AddHeader("Host", "127.0.0.1");

    if (0 == strcasecmp(method, "POST")) {
        if (!FileUtils::ReadFile(file, &(request.GetContent()))) {
            printf("Cannot read %s", file);
            exit(0);
        }
    }

    BlockTcpStream socket;
    if (!BlockTcpUtils::Open(&socket, host, port, 100, nullptr, 0)) {
        printf("Connect %s:%d fail\n", host, port);
        exit(-1);
    }

    HttpResponse response;

    int ret{0};

    if (request.IsMethod("GET")) {
        ret = HttpClient::Get(socket, request, &response);
    } else if (request.IsMethod("POST")) {
        ret = HttpClient::Post(socket, request, &response);
    } else if (request.IsMethod("HEAD")) {
        ret = HttpClient::Head(socket, request, &response);
    } else {
        printf("unsupport method %s\n", request.method());
    }

    if (0 == ret) {
        printf("response:\n");

        printf("%s %d %s\n", response.GetVersion(), response.GetStatusCode(), response.GetReasonPhrase());

        printf("%zu headers\n", response.GetHeaderCount());
        for (size_t i{0}; response.GetHeaderCount() > i; ++i) {
            const char *name{response.GetHeaderName(i)};
            const char *val{response.GetHeaderValue(i)};
            printf("%s: %s\r\n", name, val);
        }

        printf("%zu bytes body\n", response.GetContent().size());
        if (response.GetContent().size() > 0) {
            //printf("%s\n", (char*)response.getContent());
        }
    } else {
        printf("http request fail\n");
    }

    return 0;
}

