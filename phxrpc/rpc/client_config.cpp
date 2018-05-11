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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "client_config.h"
#include "client_monitor.h"
#include "monitor_factory.h"

#include "phxrpc/file.h"


namespace phxrpc {


ClientConfig::ClientConfig() {
    connect_timeout_ms_ = 200;
    socket_timeout_ms_ = 5000;
    memset(package_name_, 0, sizeof(package_name_));
}

ClientConfig::~ClientConfig() {
}

void ClientConfig::SetClientMonitor(ClientMonitorPtr client_monitor) {
    client_monitor_ = client_monitor;
}

ClientMonitorPtr ClientConfig::GetClientMonitor() {
    return client_monitor_;
}

bool ClientConfig::Read(const char *config_file) {
    Config config;
    if (!config.InitConfig(config_file)) {
        return false;
    }

    int count{0};
    bool succ{true};
    succ &= config.ReadItem("Server", "ServerCount", &count);
    if (!succ) {
        log(LOG_ERR, "Config::%s key ServerCount not found", __func__);
        return false;
    }

    config.ReadItem("Server", "PackageName", package_name_, sizeof(package_name_));

    for (int i{0}; count > i; ++i) {
        char section[64]{0};
        snprintf(section, sizeof(section), "Server%d", i);

        Endpoint_t ep;
        bool succ{true};
        succ &= config.ReadItem(section, "IP", ep.ip, sizeof(ep.ip));
        succ &= config.ReadItem(section, "Port", &(ep.port));
        if (!succ) {
            continue;
        }

        endpoints_.push_back(ep);
    }

    config.ReadItem("ClientTimeout", "ConnectTimeoutMS", &connect_timeout_ms_);
    config.ReadItem("ClientTimeout", "SocketTimeoutMS", &socket_timeout_ms_);

    if (endpoints_.size() == 0) {
        log(LOG_ERR, "Config::%s no endpoints", __func__);
    }
    return endpoints_.size() > 0;
}

const Endpoint_t *ClientConfig::GetRandom() const {
    const Endpoint_t *ret{nullptr};

    if (endpoints_.size() > 0) {
        ret = &(endpoints_[random() % endpoints_.size()]);
    }

    if (!ret) {
        if (client_monitor_.get()) {
            client_monitor_->GetEndpointFail();
        }

        log(LOG_ERR, "GetRandom fail, list.size %lu", endpoints_.size());
    }
    return ret;
}

const Endpoint_t *ClientConfig::GetByIndex(const size_t index) const {
    const Endpoint_t *ret{nullptr};

    if (index < endpoints_.size()) {
        ret = &(endpoints_[index]);
    }

    if (!ret) {
        if (client_monitor_.get()) {
            client_monitor_->GetEndpointFail();
        }
    }
    return ret;
}

int ClientConfig::GetConnectTimeoutMS() {
    return connect_timeout_ms_;
}

int ClientConfig::GetSocketTimeoutMS() {
    return socket_timeout_ms_;
}

const char *ClientConfig::GetPackageName() const {
    return package_name_;
}


}  // namespace phxrpc

