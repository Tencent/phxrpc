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
#include <vector>

#include "phxrpc/file.h"
#include "priority_config.h"

namespace phxrpc {


BusinessPriorityConfig::BusinessPriorityConfig() {
    priorit_cnt_ = 0;
}

BusinessPriorityConfig::~BusinessPriorityConfig() {
}

bool BusinessPriorityConfig::Read(const char * config_file) {

    log(LOG_DEBUG, "Config::%s config_file %s", __func__, config_file);

    Config config;
    if (!config.InitConfig(config_file)) {
        return false;
    }

    int count = 0;
    bool succ = true;
    succ &= config.ReadItem("BusinessPriority", "PriorityCount", &count);
    if (!succ) {
        log(LOG_ERR, "Config::%s key PriorityCount not found", __func__);
        return false;
    }

    priorit_cnt_ = count;
    log(LOG_DEBUG, "Config::%s PriorityCount %d", __func__, count);

    for(int i = 0; i < count; i++) {
        char name[64] = { 0 };
        snprintf(name, sizeof(name), "BusinessPriority%d", i + 1);
        std::vector<std::string> section;
        if(config.GetSection(name, &section)) {
            for(auto & it : section) {
                priority_map_[it] = i + 1;
                log(LOG_DEBUG, "Config::%s bussiness %s priority %d", __func__,
                        it.c_str(), i + 1);
            }
        }
    }
    return true;
}

int BusinessPriorityConfig::GetPriority(const char * business_name) {
    auto it = priority_map_.find(std::string(business_name));
    if(it == priority_map_.end()) {
        return priorit_cnt_;
    } else {
        return it->second;
    }
}

int BusinessPriorityConfig::GetPriorityCnt() {
    return priorit_cnt_;
}

}

