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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "socket_stream_block.h"

void test() {
    int listenfd = -1;
    if (!phxrpc::BlockTcpUtils::Listen(&listenfd, "127.0.0.1", 16161)) {
        printf("Listen fail\n");
        return;
    }

    char line[1024] = { 0 };

    for (;;) {
        int fd = -1;
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(addr);

        fd = accept(listenfd, (struct sockaddr*) &addr, &socklen);

        if (fd >= 0) {
            phxrpc::BlockTcpStream stream;
            stream.Attach(fd);
            stream << "Input quit to stop!" << std::endl;
            for (;;) {
                if (stream.getlineWithTrimRight(line, sizeof(line)).good()) {
                    if (0 == strncasecmp(line, "quit", 4))
                        break;
                    stream << line << std::endl;
                } else {
                    break;
                }
            }
        }
    }

    close(listenfd);
}

int main(int argc, char * argv[]) {
    test();

    return 0;
}

