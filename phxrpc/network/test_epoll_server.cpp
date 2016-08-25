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
#include <unistd.h>

#include <queue>

using namespace phxrpc;

void echoserver(void * args) {
    UThreadEpollArgs_t * tt = (UThreadEpollArgs_t*) args;

    UThreadSocket_t * socket = tt->first->CreateSocket(tt->second);

    char line[1024] = { 0 };

    const char * welcome = "Input quit to stop!\n";

    printf("start %d\n", tt->second);

    UThreadSend(*socket, welcome, strlen(welcome), 0);

    for (;;) {
        memset(line, 0, sizeof(line));
        UThreadRecv(*socket, line, sizeof(line), 0);

        if (0 == strncasecmp(line, "quit", 4))
            break;

        UThreadSend(*socket, line, strlen(line), 0);
    }

    printf("end %d\n", tt->second);

    UThreadClose(*socket);

    free(socket);

    free(tt);
}

void echoaccept(void * args) {
    UThreadEpollArgs_t * tt = (UThreadEpollArgs_t*) args;

    UThreadSocket_t * acceptor = tt->first->CreateSocket(tt->second);

    for (;;) {
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(addr);

        int fd = UThreadAccept(*acceptor, (struct sockaddr*) &addr, &socklen);

        printf("accept %d\n", fd);

        if (fd >= 0) {
            UThreadEpollArgs_t * ct = (UThreadEpollArgs_t*) calloc(1, sizeof(UThreadEpollArgs_t));
            ct->first = tt->first;
            ct->second = fd;

            tt->first->AddTask(echoserver, ct);
        }
    }

    UThreadClose(*acceptor);

    free(acceptor);

    free(tt);
}

void test(int count) {
    int listenfd = -1;

    if (!BlockTcpUtils::Listen(&listenfd, "127.0.0.1", 16161))
        return;

    UThreadEpollScheduler scheduler(64 * 1024, count + 1);

    // accept
    {
        UThreadEpollArgs_t * args = (UThreadEpollArgs_t*) calloc(1, sizeof(UThreadEpollArgs_t));
        args->first = &scheduler;
        args->second = listenfd;

        scheduler.AddTask(echoaccept, args);
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

