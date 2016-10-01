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

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <algorithm>

#include "phxrpc/rpc.h"
#include "redis_client_config.h"

namespace phxrpc {

RedisClientConfig::RedisClientConfig() {
    svr_cnt_ = 0;
    svr_port_ = 0;
}

RedisClientConfig::~RedisClientConfig() {
}

bool RedisClientConfig::Parse(Config & config) {
    int count = 0;
    bool succ = true;

    endpoints_.clear();
    nodes_.clear();

    succ &= config.ReadItem("Server", "ServerCount", &count);
    if (!succ) {
        log(LOG_ERR, "Config::%s key ServerCount not found", __func__);
        return false;
    }

    config.ReadItem("Server", "PackageName", package_name_, sizeof(package_name_));
    config.ReadItem("Server", "RedisCnt", &svr_cnt_, 0);
    config.ReadItem("Server", "RedisPort", &svr_port_, 0);

    int cnt = !svr_cnt_?count:(std::min(count, svr_cnt_));

    char buff[ 128 ] = { 0 };
    for (int i = 0; i < cnt; i++) {
        char section[64] = { 0 };
        snprintf(section, sizeof(section), "Server%d", i);

        Endpoint_t ep;
        bool succ = true;
        if(svr_port_) {
            succ &= config.ReadItem(section, "SVR_IP", ep.ip, sizeof(ep.ip));
            ep.port = svr_port_;
        } else {
            succ &= config.ReadItem(section, "IP", ep.ip, sizeof(ep.ip));
            succ &= config.ReadItem(section, "Port", &ep.port);
        }
        if (!succ) {
            continue;
        }

        snprintf( buff, sizeof( buff ), "%s:%d", ep.ip, ep.port );

        endpoints_.push_back(ep);

        if( i > 0 ) nodes_.append( "," );
        nodes_.append( buff );
    }

    config.ReadItem("ClientTimeout", "ConnectTimeoutMS", &connect_timeout_ms_, 200);
    config.ReadItem("ClientTimeout", "SocketTimeoutMS", &socket_timeout_ms_, 5000);


    if (endpoints_.size() == 0) {
        log(LOG_ERR, "Config::%s no endpoints", __func__);
    }
    return endpoints_.size() > 0;
}

std::string RedisClientConfig::GetNodes() {
    return nodes_;
}

}

