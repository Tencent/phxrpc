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

namespace phxrpc {

enum SocketStreamError {
    SocketStreamError_Refused = -1,
    SocketStreamError_Timeout = -202,
    SocketStreamError_Normal_Closed = -303,
};

class BaseTcpStreamBuf : public std::streambuf {
public:
    BaseTcpStreamBuf(size_t buf_size);
    virtual ~BaseTcpStreamBuf();

    int underflow();
    int overflow(int c = traits_type::eof());
    int sync();

protected:
    virtual ssize_t precv(void * buf, size_t len, int flags) = 0;
    virtual ssize_t psend(const void *buf, size_t len, int flags) = 0;

    const size_t buf_size_;
};

class BaseTcpStream : public std::iostream {
public:
    BaseTcpStream(size_t buf_size = 4096);

    virtual ~BaseTcpStream();

    virtual bool SetTimeout(int socket_timeout_ms) = 0;

    void NewRdbuf(BaseTcpStreamBuf * buf);

    bool GetRemoteHost(char * ip, size_t size, int * port = NULL);

    std::istream & getlineWithTrimRight(char * line, size_t size);

    virtual int LastError() = 0;

protected:
    virtual int SocketFd() = 0;

    const size_t buf_size_;
};

class BaseTcpUtils {
public:
    static bool SetNonBlock(int fd, bool flag);

    static bool SetNoDelay(int fd, bool flag);
};

}  //namespace phxrpc

