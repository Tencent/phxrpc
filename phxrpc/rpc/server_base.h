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


class DataFlow;

typedef struct tagDispatcherArgs {
    ServerMonitorPtr server_monitor;
    UThreadEpollScheduler *server_worker_uthread_scheduler{nullptr};
    void *service_args{nullptr};
    void *data_flow_args{nullptr};

    tagDispatcherArgs(ServerMonitorPtr server_monitor_value,
                      UThreadEpollScheduler *const server_worker_uthread_scheduler_value,
                      void *const service_args_value, void *const data_flow_args_value)
            : server_monitor(server_monitor_value),
              server_worker_uthread_scheduler(server_worker_uthread_scheduler_value),
              service_args(service_args_value), data_flow_args(data_flow_args_value) {
    }
} DispatcherArgs_t;


class ServerUtils {
  public:
    static void Daemonize();
};


}

