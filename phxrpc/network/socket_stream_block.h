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

#pragma once

#include <iostream>
#include "socket_stream_base.h"

namespace phxrpc {

class BlockTcpStreamBuf : public BaseTcpStreamBuf {
public:
    BlockTcpStreamBuf(int socket, size_t buf_size);
    virtual ~BlockTcpStreamBuf();

    ssize_t precv(void * buf, size_t len, int flags);
    ssize_t psend(const void *buf, size_t len, int flags);

private:
    int socket_;
};

//////////////////////////////////////////////////////

class BlockTcpStream : public BaseTcpStream {
public:
    BlockTcpStream(size_t buf_size = 1024);
    ~BlockTcpStream();

    void Attach(int socket);

    bool SetTimeout(int socket_timeout_ms);

    int SocketFd();

    int LastError();

private:
    int socket_;
};

///////////////////////////////////////////////////////

class BlockTcpUtils {
public:
    static bool Open(BlockTcpStream * stream, const char * ip, unsigned short port, int connect_timeout_ms,
                     const char * bind_addr, int bind_port);

    static bool Listen(int * listenfd, const char * ip, unsigned short port);

    /**
     * return > 0 : how many events
     * return 0 : timeout,
     * return -1 : error, and errno is set appropriately
     */
    static int Poll(int fd, int events, int * revents, int timeout_ms);
};

}

