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

#include "socket_stream_block.h"
#include "uthread_epoll.h"

#include <string.h>
#include <assert.h>
#include <unistd.h>

using namespace phxrpc;

void echoclient(void * args) {
    const char * ip = "127.0.0.1";
    int port = 16161;

    UThreadEpollArgs_t * tt = (UThreadEpollArgs_t*) args;

    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    UThreadSocket_t * socket = tt->first->CreateSocket(fd);

    printf("start %d\n", fd);

    struct sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));

    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = inet_addr(ip);
    in_addr.sin_port = htons(port);

    UThreadSetConnectTimeout(*socket, 1000);

    int ret = UThreadConnect(*socket, (struct sockaddr*) &in_addr, sizeof(in_addr));
    printf("connect %d\n", ret);
    if (ret != 0) {
        UThreadClose(*socket);
        free(socket);
        free(tt);

        return;
    }

    char line[1024] = { 0 };

    UThreadRecv(*socket, line, sizeof(line), 0);

    for (int i = 0; i < 10; i++) {
        snprintf(line, sizeof(line), "%d\n", i);
        UThreadSend(*socket, line, strlen(line), 0);

        memset(line, 0, sizeof(line));
        UThreadRecv(*socket, line, sizeof(line), 0);

        assert(i == atoi(line));
    }

    UThreadSend(*socket, "quit\n", 5, 0);

    printf("end %d\n", fd);

    UThreadClose(*socket);

    free(socket);

    free(tt);
}

void test(int count) {
    UThreadEpollScheduler scheduler(64 * 1024, count);

    for (int i = 0; i < count; i++) {
        UThreadEpollArgs_t * args = (UThreadEpollArgs_t*) calloc(1, sizeof(UThreadEpollArgs_t));
        args->first = &scheduler;

        scheduler.AddTask(echoclient, args);
    }

    scheduler.Run();
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: %s <count>\n", argv[0]);
        return -1;
    }

    test(atoi(argv[1]));

    return 0;
}

