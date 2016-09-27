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

const char * PHXRPC_CLIENT_HPP_TEMPLATE =
        R"(

#include <string>
#include "$MessageFile$.h"
#include "phxrpc/rpc.h"

class $ClientClass$Register
{
public:

    static $ClientClass$Register * GetDefault();

    $ClientClass$Register();
    ~$ClientClass$Register();

    int Register();
private:
    bool is_registered_;
};

class $ClientClass$
{
public:
    static bool Init( const char * config_file );

    static const char * GetPackageName();

public:
    $ClientClass$();
    ~$ClientClass$();

$ClientClassFuncDeclarations$
private:
    void CheckConfig();

    std::string package_name_;
    phxrpc::ClientConfig * config_;
};

)";

//////////////////////////////////////////////////////////////////////

const char * PHXRPC_CLIENT_CPP_TEMPLATE =
        R"(

#include <iostream>
#include <memory>
#include <stdlib.h>
#include <mutex>

#include "$ClientFile$.h"
#include "$StubFile$.h"

#include "phxrpc/file.h"
#include "phxrpc/rpc.h"

static phxrpc::ClientConfig global_$ClientClassLower$_config_;
static phxrpc::ClientMonitorPtr global_$ClientClassLower$_monitor_;
static bool g_$ClientClassLower$_is_use_registry = true;

$ClientClass$Register * $ClientClass$Register::GetDefault() {
    static $ClientClass$Register $ClientClassLower$_register;
    return &$ClientClassLower$_register;
}

$ClientClass$Register::$ClientClass$Register() {
    is_registered_ = false;
}

$ClientClass$Register::~$ClientClass$Register() {

}

int $ClientClass$Register::Register() {
    if(is_registered_) {
        return 0;
    }

    int ret = phxrpc::ClientConfigRegistry::GetDefault()->Register("search");
    if(0 != ret) {
        phxrpc::log(LOG_ERR, "$ClientClass$Register::%s Register %s failed", __func__, "search");
        return -1;
    }

    is_registered_ = true;
    return 0;
}

bool $ClientClass$ :: Init( const char * config_file )
{
    g_$ClientClassLower$_is_use_registry = false;
    return global_$ClientClassLower$_config_.Read( config_file );
}

const char * $ClientClass$ :: GetPackageName() {
    const char * ret = global_$ClientClassLower$_config_.GetPackageName();
    if (strlen(ret) == 0) {
        ret = "$PackageName$";
    }
    return ret;
}

void $ClientClass$ :: CheckConfig() {
    if(g_$ClientClassLower$_is_use_registry) {
        if(0 == $ClientClass$Register::GetDefault()->Register()) { 
            config_ = phxrpc::ClientConfigRegistry::GetDefault()->GetConfig("$PackageName$");
        } else {
            config_ = NULL;
        }
    } else {
        config_ = &global_$ClientClassLower$_config_;
    }
}

$ClientClass$ :: $ClientClass$()
{
    if(g_$ClientClassLower$_is_use_registry) {
        package_name_ = std::string("$PackageName$");
    } else {
        package_name_ = std::string(GetPackageName());
    }
    CheckConfig();
    if(!config_) {
        return;
    }

    static std::mutex monitor_mutex;
    if ( !global_$ClientClassLower$_monitor_.get() ) { 
        monitor_mutex.lock();
        if ( !global_$ClientClassLower$_monitor_.get() ) {
            global_$ClientClassLower$_monitor_ = phxrpc::MonitorFactory::GetFactory()
                ->CreateClientMonitor(package_name_.c_str());
        }
        config_->SetClientMonitor( global_$ClientClassLower$_monitor_ );
        monitor_mutex.unlock();
    }
}

$ClientClass$ :: ~$ClientClass$()
{
}

$ClientClassFuncs$
)";

//////////////////////////////////////////////////////////////////////

const char * PHXRPC_CLIENT_FUNC_TEMPLATE =
        R"(
{
    CheckConfig();
    if(!config_) {
        phxrpc::log(LOG_ERR, "%s %s config is NULL", __func__, package_name_.c_str());
        return -1;
    }
    const phxrpc::Endpoint_t * ep = config_->GetRandom();

    if(ep != nullptr) {
        phxrpc::BlockTcpStream socket;
        bool open_ret = phxrpc::PhxrpcTcpUtils::Open(&socket, ep->ip, ep->port,
                    config_->GetConnectTimeoutMS(), NULL, 0, 
                    *(global_$ClientClassLower$_monitor_.get()));
        if ( open_ret ) {
            socket.SetTimeout(config_->GetSocketTimeoutMS());

            $StubClass$ stub(socket, *(global_$ClientClassLower$_monitor_.get()));
            stub.SetConfig(config_);
            return stub.$Func$(req, resp);
        } 
    }

    return -1;
}
)";

//////////////////////////////////////////////////////////////////////

const char * PHXRPC_BATCH_CLIENT_FUNC_TEMPLATE =
        R"(
{
    CheckConfig();
    if(!config_) {
        phxrpc::log(LOG_ERR, "%s %s config is NULL", __func__, package_name_.c_str());
        return -1;
    }
    int ret = -1; 
    size_t echo_server_count = 2;
    uthread_begin;
    for (size_t i = 0; i < echo_server_count; i++) {
        uthread_t [=, &uthread_s, &ret](void *) {
            const phxrpc::Endpoint_t * ep = config_->GetByIndex(i);
            if (ep != nullptr) {
                phxrpc::UThreadTcpStream socket;
                if(phxrpc::PhxrpcTcpUtils::Open(&uthread_s, &socket, ep->ip, ep->port,
                            config_->GetConnectTimeoutMS(), *(global_$ClientClassLower$_monitor_.get()))) { 
                    socket.SetTimeout(config_->GetSocketTimeoutMS());
                    $StubClass$ stub(socket, *(global_$ClientClassLower$_monitor_.get()));
                    stub.SetConfig(config_);
                    int this_ret = stub.PHXEcho(req, resp);
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

const char * PHXRPC_CLIENT_ETC_TEMPLATE =
        R"(

[ClientTimeout]
ConnectTimeoutMS = 100
SocketTimeoutMS = 5000

[Server]
ServerCount = 2
PackageName=$PackageName$

[Server0]
IP = 127.0.0.1
Port = 16161

[Server1]
IP = 127.0.0.1
Port = 16161

)";

