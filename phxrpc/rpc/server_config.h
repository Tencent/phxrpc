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

#include <string>
#include <memory>
#include "phxrpc/file.h"

namespace phxrpc {

class ServerConfig {
public:
    ServerConfig();
    virtual ~ServerConfig();

    bool Read(const char * config_file);

    virtual bool DoRead(Config & config);

    void SetBindIP(const char * ip);
    const char * GetBindIP() const;

    void SetPort(int port);
    int GetPort() const;

    void SetMaxThreads(int max_threads);
    int GetMaxThreads() const;

    void SetSocketTimeoutMS(int socket_timeout_ms);
    int GetSocketTimeoutMS() const;

    void SetPackageName(const char * package_name);
    const char * GetPackageName() const;

    const char * GetLogDir() const;

    void SetLogLevel( int log_level );
    int GetLogLevel() const;

private:
    char bind_ip_[32];
    int port_;
    int max_threads_;
    int socket_timeout_ms_;
    char package_name_[64];
    char log_dir_[128];
    int log_level_;
};

class HshaServerConfig : public ServerConfig {
public:

    enum {
        FASTREJECT_TYPE_RANDOM = 0,
        FASTREJECT_TYPE_QOS = 1,
    };


    HshaServerConfig();
    ~HshaServerConfig();

    bool DoRead(Config & config);

    void SetMaxConnections(const int max_connections);
    int GetMaxConnections() const;

    void SetMaxQueueLength(const int max_queue_length);
    int GetMaxQueueLength() const;

    void SetFastRejectThresholdMS(const int fast_reject_threshold_ms);
    int GetFastRejectThresholdMS() const;

    void SetFastRejectAdjustRate(const int fast_reject_adjust_rate);
    int GetFastRejectAdjustRate() const;

    void SetIOThreadCount(const int io_thread_count);
    int GetIOThreadCount() const;

    void SetQoSBusinessPriorityConfFile(const char * qos_business_priority_conf_file);
    const char * GetQoSBusinessPriorityConfFile() const;

    void SetUserPriorityCnt(const int qos_user_priority_cnt);
    int GetUserPriorityCnt() const;

    void SetUserPriorityElevatePercent(const int qos_user_priority_elevate_cnt);
    int GetUserPriorityElevatePercent() const;

    void SetUserPriorityLowerPercent(const int qos_user_priority_lower_percent);
    int GetUserPriorityLowerPercent() const;

    void SetFastRejectType(const int fast_reject_type);
    int GetFastRejectType() const;


private:
    int max_connections_;
    int max_queue_length_;
    int fast_reject_threshold_ms_;
    int fast_reject_adjust_rate_;
    int io_thread_count_;
    int fast_reject_type_;
    char qos_business_priority_conf_file_[1024];
    int qos_user_priority_cnt_;
    int qos_user_priority_elevate_percent_;
    int qos_user_priority_lower_percent_;

};

}

