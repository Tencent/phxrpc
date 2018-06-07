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


#include <cinttypes>
#include <memory>
#include <sys/types.h>


namespace phxrpc {


class ClientMonitor {
  public:
    ClientMonitor();

    virtual ~ClientMonitor();

    virtual void ClientConnect(bool result);

    virtual void SendBytes(size_t bytes);

    virtual void SendError();

    virtual void SendCount();

    virtual void RecvBytes(size_t bytes);

    virtual void RecvCount();

    virtual void RecvError();

    virtual void RequestCost(uint64_t begin_time, uint64_t end_time);

    virtual void GetEndpointFail();

    virtual void ClientCall(const int cmd_id, const char *method_name);
};

typedef std::shared_ptr<ClientMonitor> ClientMonitorPtr;

}  // namespace phxrpc

