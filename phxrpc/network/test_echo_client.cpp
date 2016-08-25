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

#include <assert.h>
#include <memory>

#include <errno.h>
#include <string.h>

#include "socket_stream_block.h"

void test(int times) {
    int step = (times < 20 ? 20 : times) / 20;

    for (int i = 0; i < 20; i++) {
        phxrpc::BlockTcpStream stream;
        if (!phxrpc::BlockTcpUtils::Open(&stream, "127.0.0.1", 16161, 100, NULL, 0)) {
            printf("Connect fail\n");
            return;
        }

        char line[128] = { 0 };
        if (!stream.getlineWithTrimRight(line, sizeof(line)).good()) {
            printf("Welcome message fail, errno %d, %s\n", errno, strerror(errno));
            return;
        }

        for (int i = 0; i < step; i++) {
            stream << i << std::endl;

            if (stream.getline(line, sizeof(line)).good()) {
                assert(i == atoi(line));
            } else {
                break;
            }
        }

        stream << "quit" << std::endl;

        printf("#");
        fflush (stdout);
    }

    printf("\n");
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: %s <run times>\n", argv[0]);
        return 0;
    }

    test(atoi(argv[1]));

    return 0;
}

