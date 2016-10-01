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


#include <string>
#include "phxrpc/file.h"
#include "r3c/r3c.h"
#include "redis_config_loader.h"

namespace phxrpc {

static RedisClientConfigLoader g_redis_client_config_loader;

RedisClientConfigLoader::RedisClientConfigLoader() {

    phxrpc::RedisClientConfig config;

    if( config.Read("/home/qspace/etc/route/shanghai/mmminichat_route.conf") ) {
        client_ = new r3c::CRedisClient( config.GetNodes() );
    } else {
        log(LOG_ERR, "RedisClientConfigLoader::%s read redis client config %s failed",
                __func__, "/home/qspace/etc/minichat/client/redis_client.conf");
        client_ = NULL;
    }

    ClientConfigRegistry::GetDefault()->SetClientConfigLoader(this);
}

RedisClientConfigLoader::~RedisClientConfigLoader() {

}

int RedisClientConfigLoader::GetConfigContent(const char * package_name, std::string * content) {
    if(!content) {
        return -1;
    }

    if(!client_) {
        return -1;
    }


    std::string key = std::string("cliconf:") + std::string(package_name);

    if(client_->get(key, content)) {
        log(LOG_DEBUG, "RedisClientConfigLoader::%s get content for %s success len %zu",
                __func__, package_name, content->size());

        return 0;
    } else {
        log(LOG_ERR, "RedisClientConfigLoader::%s get content for %s failed",
                __func__, package_name);
        content->clear();
        return -1;
    }
}

}

