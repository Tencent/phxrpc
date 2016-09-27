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


#include <memory>
#include <unistd.h>
#include <linux/limits.h>

#include "phxrpc/file.h"
#include "phxrpc/redis.h"
#include "client_config_registry.h"
#include "r3c/r3c.h"

namespace phxrpc {

static ClientConfigRegistry * g_default_registry = NULL;

ClientConfigRegistry * ClientConfigRegistry::GetDefault() {
    return g_default_registry;

}

void ClientConfigRegistry::SetDefaultClientConfigRegistry(ClientConfigRegistry * registry) {
    g_default_registry = registry;
}

ClientConfigRegistry::ClientConfigRegistry() {
    stop_ = 0;
    reload_interval_ = 60;
}

ClientConfigRegistry::~ClientConfigRegistry() {

}

void ClientConfigRegistry::SetReloadInterval(int reload_interval) {
    reload_interval_ = reload_interval;
}

void ClientConfigRegistry::Reset() {
    RWLockGuard lock_guard(&rwlock_, RWLockWriteLock);
    for(auto & it : config_map_) {
        CliConfigItem * item = it.second;
        item->Reset();
        delete item;
        it.second = NULL;
    }
    config_map_.clear();
}

int ClientConfigRegistry::Register(const char * package_name) {
    RWLockGuard lock_guard(&rwlock_, RWLockWriteLock);
    auto it = config_map_.find(std::string(package_name));
    if(it == config_map_.end()) {
        CliConfigItem * item = new CliConfigItem();
        item->config_1 = new ClientConfig();
        item->config_2 = new ClientConfig();
        item->curr_config = NULL;
        int ret = LoadConfig(package_name, item);
        if(0 != ret) {
            log(LOG_ERR, "ClientConfigRegistry::%s LoadConfig for %s failed", __func__, package_name);
            item->Reset();
            delete item;
            return -1;
        }
        config_map_[std::string(package_name)] = item;
    }
    return 0;
}

ClientConfig * ClientConfigRegistry::GetConfig(const char * package_name) {
    RWLockGuard lock_guard(&rwlock_, RWLockReadLock);
    auto it = config_map_.find(std::string(package_name));
    if(it == config_map_.end()) {
        log(LOG_ERR, "ClientConfigRegistry::%s package %s not registered", __func__, package_name);
        return NULL;
    } else {
        log(LOG_ERR, "ClientConfigRegistry::%s for %s success", __func__, package_name);
        return it->second->curr_config;
    }
}

int ClientConfigRegistry::LoadConfig(const char * package_name, CliConfigItem * item) {
    std::string content;
    int ret = GetConfigContent(package_name, &content);
    if(0 != ret) {
        log(LOG_ERR, "ClientConfigRegistry::%s GetConfigContent for %s failed", __func__, package_name);
        return -1;
    }

    if(!item->curr_config) {
        item->config_1->Read(content);
        item->config_2->Read(content);
        item->curr_config = item->config_1;
    } else {
        ClientConfig * tmp_config = item->curr_config == item->config_1?item->config_2:item->config_1;
        tmp_config->Read(content);
        item->curr_config = tmp_config;
    }
    return 0;
}

void ClientConfigRegistry::Run() {

    time_t last_reload_timestamp = time(NULL);

    while(!stop_) {
        sleep(1);
        time_t curr_time = time(NULL);
        if((curr_time - last_reload_timestamp) > reload_interval_) {

            std::unordered_map<std::string, CliConfigItem*> config_map;
            {
                RWLockGuard lock_guard(&rwlock_, RWLockReadLock);
                config_map = config_map_;
            }

            int ret = 0;
            for(auto & it : config_map) {
                const std::string & package_name = it.first;
                CliConfigItem * item = it.second;

                ret = LoadConfig(package_name.c_str(), item);
                log(LOG_ERR, "ClientConfigRegistry::%s LoadConfig for %s ret %d",
                        __func__, package_name.c_str(), ret);
            }

            last_reload_timestamp = curr_time;
        }
    }
}

void ClientConfigRegistry::Stop() {
    stop_ = 1;
} 

//----------------------------------------------------------------------------

LocalFileClienctConfigRegistry * LocalFileClienctConfigRegistry::GetDefault() {
    static LocalFileClienctConfigRegistry registry;
    return & registry;
}

LocalFileClienctConfigRegistry::LocalFileClienctConfigRegistry() {

}

LocalFileClienctConfigRegistry::~LocalFileClienctConfigRegistry() {

}

void LocalFileClienctConfigRegistry::SetClientConfigFileLocation(const char * location) {
    location_ = std::string(location);
}


int LocalFileClienctConfigRegistry::GetConfigContent(const char * package_name, std::string * content) {
    if(!content) {
        return -1;
    }
    std::unique_ptr<char []> file_path(new char[PATH_MAX]);
    if(!file_path.get()) {
        log(LOG_ERR, "LocalFileClienctConfigRegistry::%s new file_path failed errno %d", __func__, errno);
        return -1;
    }

    snprintf(file_path.get(), PATH_MAX, "%s/%s_client.conf", location_.c_str(),
            package_name);

    if(!FileUtils::ReadFile(file_path.get(), content)) {
        content->clear();
        log(LOG_ERR, "LocalFileClienctConfigRegistry::%s read file %s failed", __func__, file_path.get());
        return -1;
    } else {
        log(LOG_DEBUG, "LocalFileClienctConfigRegistry::%s read file %s success", __func__, file_path.get());
    }
    return 0;
}

//----------------------------------------------------------------------------

RedisClientConfigRegistry * RedisClientConfigRegistry::GetDefault() {
   static RedisClientConfigRegistry registry;
   return &registry;
}

RedisClientConfigRegistry::RedisClientConfigRegistry() {

}

RedisClientConfigRegistry::~RedisClientConfigRegistry() {

}

int RedisClientConfigRegistry::GetConfigContent(const char * package_name, std::string * content) {
    if(!content) {
        return -1;
    }

    r3c::CRedisClient * client = RedisClientFactory::GetDefault()->Get();

    std::string key = std::string("cliconf:") + std::string(package_name);

    if(client->get(key, content)) {
        log(LOG_DEBUG, "RedisClientConfigRegistry::%s get content for %s success len %zu",
                __func__, package_name, content->size());

        return 0;
    } else {
        log(LOG_ERR, "RedisClientConfigRegistry::%s get content for %s failed",
                __func__, package_name);
        content->clear();
        return -1;
    }
}


}


