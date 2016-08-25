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
#include <assert.h>

#include "socket_stream_block.h"
#include "phxrpc/file/log_utils.h"

namespace phxrpc {

BlockTcpStreamBuf::BlockTcpStreamBuf(int socket, size_t buf_size)
        : BaseTcpStreamBuf(buf_size),
          socket_(socket) {
}

BlockTcpStreamBuf::~BlockTcpStreamBuf() {
}

ssize_t BlockTcpStreamBuf::precv(void * buf, size_t len, int flags) {
    return recv(socket_, buf, len, flags);
}

ssize_t BlockTcpStreamBuf::psend(const void *buf, size_t len, int flags) {
    return send(socket_, buf, len, flags);
}

////////////////////////////////////////////////////////////

BlockTcpStream::BlockTcpStream(size_t buf_size)
        : BaseTcpStream(buf_size),
          socket_(-1) {
}

BlockTcpStream::~BlockTcpStream() {
    if (socket_ >= 0) {
        close(socket_);
    }
}

void BlockTcpStream::Attach(int socket) {
    NewRdbuf(new BlockTcpStreamBuf(socket, buf_size_));
    socket_ = socket;
}

bool BlockTcpStream::SetTimeout(int socket_timeout_ms) {
    if (SocketFd() < 0)
        return false;

    struct timeval to;
    to.tv_sec = socket_timeout_ms / 1000;
    to.tv_usec = (socket_timeout_ms % 1000) * 1000;

    if (setsockopt(SocketFd(), SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) < 0) {
        phxrpc::log(LOG_ERR, "setsockopt(%d) errno %d, %s", SocketFd(), errno, strerror(errno));
        return false;
    }

    if (setsockopt(SocketFd(), SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) < 0) {
        phxrpc::log(LOG_ERR, "setsockopt(%d) errno %d, %s", SocketFd(), errno, strerror(errno));
        return false;
    }

    return true;
}

int BlockTcpStream::SocketFd() {
    return socket_;
}

int BlockTcpStream :: LastError() {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //timeout
        return SocketStreamError_Timeout; 
    } else {
        return SocketStreamError_Refused;
    }
}

/////////////////////////////////////////////////////////////
//
bool BlockTcpUtils::Open(BlockTcpStream * stream, const char * ip, unsigned short port, int connect_timeout_ms,
                    const char * bind_addr, int bind_port) {
    if (INADDR_NONE == inet_addr(ip)) {
        phxrpc::log(LOG_ERR, "Invalid ip [%s]", ip);
        return false;
    }

    if (NULL != bind_addr && INADDR_NONE == inet_addr(bind_addr)) {
        phxrpc::log(LOG_ERR, "Invalid bind_addr [%s]", bind_addr);
        return false;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (sockfd < 0) {
        phxrpc::log(LOG_WARNING, "TcpSocket::Connect().socket()=%d", sockfd);
        return false;
    }

    struct sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET;

    if (NULL != bind_addr && '\0' != *bind_addr) {
        in_addr.sin_addr.s_addr = inet_addr(bind_addr);
        in_addr.sin_port = bind_port;
        if (bind(sockfd, (struct sockaddr *) &in_addr, sizeof(in_addr)) < 0) {
            phxrpc::log(LOG_WARNING, "WARN: bind( %d, %s, ... ) fail", sockfd, bind_addr);
        }
    }

    in_addr.sin_addr.s_addr = inet_addr(ip);
    in_addr.sin_port = htons(port);

    BaseTcpUtils::SetNonBlock(sockfd, true);

    int error = 0;
    int ret = connect(sockfd, (struct sockaddr*) &in_addr, sizeof(in_addr));

    if (0 != ret && ((errno != EINPROGRESS) && (errno != EAGAIN))) {
        phxrpc::log(LOG_ERR, "connect(%d{%s:%d}) errno %d, %s", sockfd, ip, port, errno, strerror(errno));
        error = -1;
    }

    if (0 == error && 0 != ret) {
        int revents = 0;

        ret = Poll(sockfd, POLLOUT, &revents, connect_timeout_ms);
        if ((POLLOUT & revents)) {
            socklen_t len = sizeof(error);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*) &error, &len) < 0) {
                phxrpc::log(LOG_ERR, "Socket(%d)::connectNonblock().getsockopt() < 0", sockfd);
                error = -1;
            }
        } else {
            //phxrpc::log(LOG_ERR, "Socket(%d)::connectNonblock().poll()=%d, revents=%d", sockfd, ret, revents);
            error = -1;
        }
    }

    if (0 == error) {
        if (BaseTcpUtils::SetNonBlock(sockfd, false)
                && BaseTcpUtils::SetNoDelay(sockfd, true)) {
            stream->Attach(sockfd);
        } else {
            phxrpc::log(LOG_ERR, "set nonblock fail");
            error = -1;
            close(sockfd);
        }
    } else {
        close(sockfd);
    }
    return 0 == error;
}

bool BlockTcpUtils::Listen(int * listenfd, const char * ip, unsigned short port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        phxrpc::log(LOG_WARNING, "socket failed, errno %d, %s", errno, strerror(errno));
        return false;
    }

    int ret = 0;

    int flags = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &flags, sizeof(flags)) < 0) {
        phxrpc::log(LOG_WARNING, "failed to set setsock to reuseaddr");
    }

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if ('\0' != *ip) {
        addr.sin_addr.s_addr = inet_addr(ip);
        if (INADDR_NONE == addr.sin_addr.s_addr) {
            phxrpc::log(LOG_CRIT, "failed to convert %s to inet_addr", ip);
            ret = -1;
        }
    }

    if (0 == ret) {
        if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
            phxrpc::log(LOG_CRIT, "bind failed, errno %d, %s", errno, strerror(errno));
            ret = -1;
        }
    }

    if (0 == ret) {
        if (listen(sockfd, 1024) < 0) {
            phxrpc::log(LOG_CRIT, "listen failed, errno %d, %s", errno, strerror(errno));
            ret = -1;
        }
    }

    if (0 != ret && sockfd >= 0)
        close(sockfd);

    if (0 == ret) {
        *listenfd = sockfd;
        phxrpc::log(LOG_NOTICE, "Listen on port [%d]", port);
    }

    return 0 == ret;
}

int BlockTcpUtils::Poll(int fd, int events, int * revents, int timeout_ms) {
    int ret = -1;

    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    pfd.events = events;

    errno = 0;

    // retry again for EINTR
    for (int i = 0; i < 2; i++) {
        ret = ::poll(&pfd, 1, timeout_ms);
        if (-1 == ret && EINTR == errno)
            continue;
        break;
    }

    if (0 == ret)
        errno = ETIMEDOUT;

    *revents = pfd.revents;

    return ret;
}

}  //namespace phxrpc

