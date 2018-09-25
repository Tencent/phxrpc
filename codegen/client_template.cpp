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

const char *PHXRPC_CLIENT_HPP_TEMPLATE =
        R"(

#include "$MessageFile$.h"
#include "phxrpc/rpc.h"


class $ClientClass$ {
  public:
    static bool Init(const char *config_file);

    static const char *GetPackageName();

    $ClientClass$();
    virtual ~$ClientClass$();

$ClientClassFuncDeclarations$};

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_UTHREAD_CLIENT_HPP_TEMPLATE =
        R"(

#include "$MessageFile$.h"
#include "phxrpc/rpc.h"
#include "phxrpc/network.h"


class $ClientClass$ {
  public:
    static bool Init(const char *config_file);

    static const char *GetPackageName();

    $ClientClass$(phxrpc::UThreadEpollScheduler *uthread_scheduler);
    virtual ~$ClientClass$();

$ClientClassFuncDeclarations$
  private:
    phxrpc::UThreadEpollScheduler *uthread_scheduler_;
};

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_CLIENT_CPP_TEMPLATE =
        R"(

#include "$ClientFile$.h"

#include <cstdlib>
#include <memory>
#include <mutex>

#include "phxrpc/http.h"
#include "phxrpc/rpc.h"

#include "$StubFile$.h"


using namespace std;


static phxrpc::ClientConfig global_$ClientClassLower$_config_;
static phxrpc::ClientMonitorPtr global_$ClientClassLower$_monitor_;


bool $ClientClass$::Init(const char *config_file) {
    return global_$ClientClassLower$_config_.Read(config_file);
}

const char *$ClientClass$::GetPackageName() {
    const char *ret = global_$ClientClassLower$_config_.GetPackageName();
    if (strlen(ret) == 0) {
        ret = "$PbPackageName$";
    }
    return ret;
}

$ClientClass$::$ClientClass$() {
    static mutex monitor_mutex;
    if (!global_$ClientClassLower$_monitor_.get()) {
        monitor_mutex.lock();
        if (!global_$ClientClassLower$_monitor_.get()) {
            global_$ClientClassLower$_monitor_ = phxrpc::MonitorFactory::GetFactory()->
                    CreateClientMonitor(GetPackageName());
        }
        global_$ClientClassLower$_config_.SetClientMonitor(global_$ClientClassLower$_monitor_);
        monitor_mutex.unlock();
    }
}

$ClientClass$::~$ClientClass$() {
}

$ClientClassFuncs$
)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_UTHREAD_CLIENT_CPP_TEMPLATE =
        R"(

#include "$ClientFile$_uthread.h"

#include <cstdlib>
#include <memory>
#include <mutex>

#include "phxrpc/http.h"
#include "phxrpc/rpc.h"

#include "$StubFile$.h"


using namespace std;


static phxrpc::ClientConfig global_$ClientClassLower$_config_;
static phxrpc::ClientMonitorPtr global_$ClientClassLower$_monitor_;


bool $ClientClass$::Init(const char *config_file) {
    return global_$ClientClassLower$_config_.Read(config_file);
}

const char *$ClientClass$::GetPackageName() {
    const char *ret{global_$ClientClassLower$_config_.GetPackageName()};
    if (strlen(ret) == 0) {
        ret = "$PbPackageName$";
    }
    return ret;
}

$ClientClass$::$ClientClass$(phxrpc::UThreadEpollScheduler *uthread_scheduler) {
    uthread_scheduler_ = uthread_scheduler;
    static mutex monitor_mutex;
    if (!global_$ClientClassLower$_monitor_.get()) {
        monitor_mutex.lock();
        if (!global_$ClientClassLower$_monitor_.get()) {
            global_$ClientClassLower$_monitor_ = phxrpc::MonitorFactory::GetFactory()->
                    CreateClientMonitor(GetPackageName());
        }
        global_$ClientClassLower$_config_.SetClientMonitor(global_$ClientClassLower$_monitor_);
        monitor_mutex.unlock();
    }
}

$ClientClass$::~$ClientClass$() {
}

$ClientClassFuncs$
)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_CLIENT_FUNC_TEMPLATE =
        R"(
{
    const phxrpc::Endpoint_t *ep{global_$ClientClassLower$_config_.GetRandom()};

    if (ep) {
        phxrpc::BlockTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(&socket, ep->ip, ep->port,
                global_$ClientClassLower$_config_.GetConnectTimeoutMS(), nullptr, 0,
                *(global_$ClientClassLower$_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_$ClientClassLower$_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            $StubClass$ stub(socket, *(global_$ClientClassLower$_monitor_.get()), http_msg_factory);
            return stub.$Func$;
        }

    }

    return -1;
}
)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_UTHREAD_CLIENT_FUNC_TEMPLATE =
        R"(
{
    const phxrpc::Endpoint_t *ep{global_$ClientClassLower$_config_.GetRandom()};

    if (uthread_scheduler_ && ep) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret{phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                global_$ClientClassLower$_config_.GetConnectTimeoutMS(),
                *(global_$ClientClassLower$_monitor_.get()))};
        if (open_ret) {
            socket.SetTimeout(global_$ClientClassLower$_config_.GetSocketTimeoutMS());
            phxrpc::HttpMessageHandlerFactory http_msg_factory;
            $StubClass$ stub(socket, *(global_$ClientClassLower$_monitor_.get()), http_msg_factory);
            return stub.$Func$;
        }
    }

    return -1;
}
)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_BATCH_CLIENT_FUNC_TEMPLATE =
        R"(
{
    int ret{-1};
    size_t echo_server_count{2};
    uthread_begin;
    for (size_t i{0}; echo_server_count > i; ++i) {
        uthread_t [=, &uthread_s, &ret](void *) {
            const phxrpc::Endpoint_t *ep = global_$ClientClassLower$_config_.GetByIndex(i);
            if (ep != nullptr) {
                phxrpc::UThreadTcpStream socket;
                if (phxrpc::PhxrpcTcpUtils::Open(&uthread_s, &socket, ep->ip, ep->port,
                    global_$ClientClassLower$_config_.GetConnectTimeoutMS(), *(global_$ClientClassLower$_monitor_.get()))) {
                    socket.SetTimeout(global_$ClientClassLower$_config_.GetSocketTimeoutMS());
                    phxrpc::HttpMessageHandlerFactory http_msg_factory;
                    $StubClass$ stub(socket, *(global_$ClientClassLower$_monitor_.get()), http_msg_factory);
                    int this_ret{stub.PHXEcho(req, resp)};
                    if (this_ret == 0) {
                        ret = this_ret;
                        uthread_s.Close();
                    }
                }
            }
        };
    }
    uthread_end;
    return ret;
}
)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_CLIENT_ETC_TEMPLATE =
        R"(

[ClientTimeout]
ConnectTimeoutMS = 100
SocketTimeoutMS = 30000

[Server]
ServerCount = 2
PackageName = $PbPackageName$

[Server0]
IP = 127.0.0.1
Port = 16161

[Server1]
IP = 127.0.0.1
Port = 16161

)";

