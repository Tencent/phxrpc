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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <errno.h>
#include <string.h>

#include "socket_stream_base.h"
#include "phxrpc/file/log_utils.h"

namespace phxrpc {

BaseTcpStreamBuf::BaseTcpStreamBuf(size_t buf_size)
        : buf_size_(buf_size) {
    char * gbuf = new char[buf_size_];
    char * pbuf = new char[buf_size_];

    setg(gbuf, gbuf, gbuf);
    setp(pbuf, pbuf + buf_size_);
}

BaseTcpStreamBuf::~BaseTcpStreamBuf() {
    delete[] eback();
    delete[] pbase();
}

int BaseTcpStreamBuf::underflow() {
    int ret = precv(eback(), buf_size_, 0);
    if (ret > 0) {
        setg(eback(), eback(), eback() + ret);
        return traits_type::to_int_type(*gptr());
    } else {
        //phxrpc::log(LOG_ERR, "ret %d errno %d,%s", ret, errno, strerror(errno));
        return traits_type::eof();
    }
}

int BaseTcpStreamBuf::sync() {
    int sent = 0;
    int total = pptr() - pbase();
    while (sent < total) {
        int ret = psend(pbase() + sent, total - sent, 0);
        if (ret > 0) {
            sent += ret;
        } else {
            //phxrpc::log(LOG_ERR, "sync ret %d errno %d,%s", ret, errno, strerror(errno));
            return -1;
        }
    }

    setp(pbase(), pbase() + buf_size_);
    pbump(0);

    return 0;
}

int BaseTcpStreamBuf::overflow(int c) {
    if (-1 == sync()) {
        return traits_type::eof();
    } else {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            sputc(traits_type::to_char_type(c));
        }

        return traits_type::not_eof(c);
    }
}

//---------------------------------------------------------

BaseTcpStream::BaseTcpStream(size_t buf_size)
        : std::iostream(NULL),
          buf_size_(buf_size) {
}

BaseTcpStream::~BaseTcpStream() {
    delete rdbuf();
}

void BaseTcpStream::NewRdbuf(BaseTcpStreamBuf * buf) {
    std::streambuf * old = rdbuf(buf);
    delete old;
}

bool BaseTcpStream::GetRemoteHost(char * ip, size_t size, int * port) {
    struct sockaddr_in addr;
    socklen_t slen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    int ret = getpeername(SocketFd(), (struct sockaddr*) &addr, &slen);

    if (0 == ret) {
        inet_ntop(AF_INET, &addr, ip, size);
        if (NULL != port)
            *port = ntohs(addr.sin_port);
    }

    return 0 == ret;
}

std::istream & BaseTcpStream::getlineWithTrimRight(char * line, size_t size) {
    if (getline(line, size).good()) {
        for (char * pos = line + gcount() - 1; pos >= line; pos--) {
            if ('\0' == *pos || '\r' == *pos || '\n' == *pos) {
                *pos = '\0';
            } else {
                break;
            }
        }
    }

    return *this;
}

//---------------------------------------------------------

bool BaseTcpUtils::SetNonBlock(int fd, bool flag) {
    int ret = 0;

    int tmp = fcntl(fd, F_GETFL, 0);

    if (flag) {
        ret = fcntl(fd, F_SETFL, tmp | O_NONBLOCK);
    } else {
        ret = fcntl(fd, F_SETFL, tmp & (~O_NONBLOCK));
    }

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "SetBlock(%d) fail, errno %d, %s", fd, errno, strerror(errno));
    }

    return 0 == ret;
}

bool BaseTcpUtils::SetNoDelay(int fd, bool flag) {
    int tmp = flag ? 1 : 0;

    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*) &tmp, sizeof(tmp));

    if (0 != ret) {
        phxrpc::log(LOG_WARNING, "SetDelay(%d) fail, errno %d, %s", fd, errno, strerror(errno));
    }

    return 0 == ret;
}

}  //namespace phxrpc

