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
#include "uthread_epoll.h"

namespace phxrpc {

class UThreadTcpStreamBuf : public BaseTcpStreamBuf {
 public:
    UThreadTcpStreamBuf(UThreadSocket_t * socket, size_t buf_size);
    virtual ~UThreadTcpStreamBuf();

    ssize_t precv(void * buf, size_t len, int flags);
    ssize_t psend(const void *buf, size_t len, int flags);

 private:
    UThreadSocket_t * uthread_socket_;
};

//////////////////////////////////////////////////////

class UThreadTcpStream : public BaseTcpStream {
 public:
    UThreadTcpStream(size_t buf_size = 1024);
    ~UThreadTcpStream();

    void Attach(UThreadSocket_t * socket);

    UThreadSocket_t * DetachSocket();

    bool SetTimeout(int socket_timeout_ms);

    int SocketFd();

    int LastError();

 private:
    UThreadSocket_t * uthread_socket_;
};

///////////////////////////////////////////////////////

class UThreadTcpUtils {
 public:
    static bool Open(UThreadEpollScheduler * tt, UThreadTcpStream* stream, const char * ip, unsigned short port,
                     int connect_timeout_ms);
};

}

