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

#include "socket_stream_phxrpc.h"


namespace phxrpc {


bool PhxrpcTcpUtils::Open(BlockTcpStream *stream, const char *ip,
                          unsigned short port, int connect_timeout_ms,
                          const char *bind_addr, int bind_port,
                          ClientMonitor &client_monitor) {
    bool ret = BlockTcpUtils::Open(stream, ip, port, connect_timeout_ms,
                                   bind_addr, bind_port);
    client_monitor.ClientConnect(ret);

    return ret;
}


bool PhxrpcTcpUtils::Open(UThreadEpollScheduler *tt, UThreadTcpStream *stream,
                          const char *ip, unsigned short port,
                          int connect_timeout_ms,
                          ClientMonitor &client_monitor) {
    bool ret = UThreadTcpUtils::Open(tt, stream, ip, port, connect_timeout_ms);
    if (!ret && errno == 0) {
        //normal active close
        client_monitor.ClientConnect(true);
    } else {
        client_monitor.ClientConnect(ret);
    }

    return ret;
}


}  // namespace phxrpc

