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

#include <thread>
#include <mutex>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "rwlock_mgr.h"
#include "client_config.h"

namespace phxrpc {

typedef struct _CliConfigItem {
    ClientConfig * config_1;
    ClientConfig * config_2;
    ClientConfig * curr_config;

    _CliConfigItem() {
        config_1 = NULL;
        config_2 = NULL;
        curr_config = NULL;
    }
    void Reset() {
        delete config_1, config_1 = NULL;
        delete config_2, config_2 = NULL;
        curr_config = NULL;
    }

} CliConfigItem;


class ClientConfigLoader
{
public:
    ClientConfigLoader() {};
    virtual ~ClientConfigLoader() {};
    virtual int GetConfigContent(const char * package_name, std::string * content) = 0;
};

class ClientConfigRegistry
{
public:
    static ClientConfigRegistry * GetDefault();
    void SetClientConfigLoader(ClientConfigLoader * loader);

    ClientConfigRegistry();
    ~ClientConfigRegistry();

    void SetReloadInterval(int reload_interval);

    virtual int Register(const char * package_name);
    virtual ClientConfig * GetConfig(const char * package_name);

    virtual void Run();
    virtual void Stop();

protected:
    virtual int LoadConfig(const char * package_name, CliConfigItem * item);

    virtual void Reset();

private:
    bool is_init_;
    std::thread thread_;

    std::unordered_map<std::string, CliConfigItem*> config_map_;
    int stop_;
    int reload_interval_;
    ClientConfigLoader * loader_;
    std::mutex mutex_;
};


class LocalFileClienctConfigLoader : public ClientConfigLoader
{
public:
    LocalFileClienctConfigLoader();
    virtual ~LocalFileClienctConfigLoader();

    void SetClientConfigFileLocation(const char * location);

protected:
    virtual int GetConfigContent(const char * package_name, std::string * content);

private:
    std::string location_;
};

}


