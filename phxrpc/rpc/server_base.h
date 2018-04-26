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

#include "server_monitor.h"
#include "phxrpc/network.h"


namespace phxrpc {


struct SessionAttribute {
    std::string client_identifier;
    bool clean_session{false};
    uint32_t keep_alive{10};
    std::string user_name;
    std::string password;
    std::string will_topic;
    std::string will_message;
};

struct ServiceContext {
    uint64_t session_id{0uL};
    bool init_session{false};
    bool heartbeat_session{false};
    bool destroy_session{false};
    SessionAttribute session_attribute;
};


class NotifierPoolRouter;
class DataFlow;

typedef struct tagDispatcherArgs {
    int pool_idx{-1};
    int worker_idx{-1};
    ServerMonitorPtr server_monitor;
    UThreadEpollScheduler *server_worker_uthread_scheduler{nullptr};
    UThreadNotifierPool *notifier_pool{nullptr};
    NotifierPoolRouter *notifier_pool_router{nullptr};
    DataFlow *data_flow{nullptr};
    DataFlow *cross_unit_data_flow{nullptr};
    void *service_args{nullptr};
    void *context{nullptr};

    tagDispatcherArgs() : service_args(nullptr) {
    }

    tagDispatcherArgs(const int pool_idx_value, const int worker_idx_value,
                      ServerMonitorPtr server_monitor_value,
                      UThreadEpollScheduler *const server_worker_uthread_scheduler_value,
                      UThreadNotifierPool *notifier_pool_value,
                      NotifierPoolRouter *notifier_pool_router_value,
                      DataFlow *const data_flow_value,
                      DataFlow *const cross_unit_data_flow_value,
                      void *const service_args_value, void *const context_value)
            : pool_idx(pool_idx_value), worker_idx(worker_idx_value),
              server_monitor(server_monitor_value),
              server_worker_uthread_scheduler(server_worker_uthread_scheduler_value),
              notifier_pool(notifier_pool_value),
              notifier_pool_router(notifier_pool_router_value),
              data_flow(data_flow_value),
              cross_unit_data_flow(cross_unit_data_flow_value),
              service_args(service_args_value), context(context_value) {
    }
} DispatcherArgs_t;


class ServerUtils {
  public:
    static void Daemonize();
};


}

