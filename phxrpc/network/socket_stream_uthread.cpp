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

#include "socket_stream_uthread.h"
#include <assert.h>
#include <string.h>

namespace phxrpc {

UThreadTcpStreamBuf::UThreadTcpStreamBuf(UThreadSocket_t * socket, size_t buf_size)
        : BaseTcpStreamBuf(buf_size),
          uthread_socket_(socket) {
}

UThreadTcpStreamBuf::~UThreadTcpStreamBuf() {
}

ssize_t UThreadTcpStreamBuf::precv(void * buf, size_t len, int flags) {
    return UThreadRecv(*uthread_socket_, buf, len, flags);
}

ssize_t UThreadTcpStreamBuf::psend(const void *buf, size_t len, int flags) {
    return UThreadSend(*uthread_socket_, buf, len, flags);
}

////////////////////////////////////////////////////////////

UThreadTcpStream::UThreadTcpStream(size_t buf_size)
        : BaseTcpStream(buf_size),
          uthread_socket_(nullptr) {
}

UThreadTcpStream::~UThreadTcpStream() {
    if (uthread_socket_ != nullptr) {
        UThreadClose(*uthread_socket_);
        free(uthread_socket_);
    }
}

void UThreadTcpStream::Attach(UThreadSocket_t * socket) {
    NewRdbuf(new UThreadTcpStreamBuf(socket, buf_size_));
    uthread_socket_ = socket;
}

UThreadSocket_t * UThreadTcpStream::DetachSocket() {
    auto socket = uthread_socket_;
    uthread_socket_ = nullptr;
    return socket;
}

bool UThreadTcpStream::SetTimeout(int socket_timeout_ms) {
    UThreadSetSocketTimeout(*uthread_socket_, socket_timeout_ms);
    return true;
}

int UThreadTcpStream::SocketFd() {
    return UThreadSocketFd(*uthread_socket_);
}

int UThreadTcpStream :: LastError() {
    if (errno == ETIMEDOUT) {
        return SocketStreamError_Timeout;
    } else if (errno == EINVAL || errno == ECONNREFUSED) {
        return SocketStreamError_Refused;
    } else if (errno == 0) {
        return SocketStreamError_Normal_Closed;
    }

    return -1;
}

/////////////////////////////////////////////////////////////

bool UThreadTcpUtils::Open(UThreadEpollScheduler * tt, UThreadTcpStream * stream, const char * ip, unsigned short port,
                           int connect_timeout_ms) {
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    UThreadSocket_t * socket = tt->CreateSocket(fd);
    assert(NULL != socket);
    UThreadSetConnectTimeout(*socket, connect_timeout_ms);

    struct sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));

    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = inet_addr(ip);
    in_addr.sin_port = htons(port);

    int ret = UThreadConnect(*socket, (struct sockaddr*) &in_addr, sizeof(in_addr));
    if (0 == ret) {
        stream->Attach(socket);
    } else {
        UThreadClose(*socket);
        free(socket);
    }
    return 0 == ret;
}

}

