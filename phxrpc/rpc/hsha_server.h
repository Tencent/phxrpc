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

#include "base_server.h"


namespace phxrpc {


class HshaServerIO final {
  public:
    HshaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
                 const HshaServerConfig *config,
                 DataFlow *data_flow, HshaServerStat *hsha_server_stat,
                 HshaServerQos *hsha_server_qos, WorkerPool *worker_pool);
    ~HshaServerIO();

    void RunForever();

    bool AddAcceptedFd(int accepted_fd);

    void HandlerAcceptedFd();

    void IOFunc(int accept_fd);

    UThreadSocket_t *ActiveSocketFunc();

  private:
    int idx_{-1};
    UThreadEpollScheduler *scheduler_{nullptr};
    const HshaServerConfig *config_{nullptr};
    DataFlow *data_flow_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    HshaServerQos *hsha_server_qos_{nullptr};
    WorkerPool *worker_pool_{nullptr};

    std::queue<int> accepted_fd_list_;
    std::mutex queue_mutex_;
};


class HshaServer;

class HshaServerUnit : public BaseServerUnit {
  public:
    HshaServerUnit(const int idx,
            Server *const root_server,
            HshaServer *const hsha_server,
            int worker_thread_count,
            int worker_uthread_count_per_thread,
            int worker_uthread_stack_size,
            NotifierPoolRouter *const notifier_pool_router,
            Dispatch_t dispatch,
            void *args);
    virtual ~HshaServerUnit() override;

    void RunFunc();
    bool AddAcceptedFd(int accepted_fd);
    int NotifyTargetWorker(const int idx, const NotifierPoolRouter::NotifierId &notifier_id,
                           void *const data);

  private:
    Server *root_server_{nullptr};
    HshaServer *hsha_server_{nullptr};
    UThreadEpollScheduler scheduler_;
    WorkerPool worker_pool_;
    HshaServerIO hsha_server_io_;
    std::thread thread_;
};


class HshaServerAcceptor final {
  public:
    HshaServerAcceptor(HshaServer *hsha_server);
    ~HshaServerAcceptor();

    void LoopAccept(const char *bind_ip, const int port);

  private:
    HshaServer *hsha_server_{nullptr};
    size_t idx_{};
};


class HshaServer : public BaseServer {
  public:
    HshaServer(const HshaServerConfig &config, const Dispatch_t &dispatch, void *args,
               Server *const root_server);
    virtual ~HshaServer() override;

    virtual void DoRunForever() override;

    int SendNotify(const NotifierPoolRouter::NotifierId &notifier_id, void *const data);
    int WaitNotify(UThreadNotifierPool *const notifier_pool, const int pool_idx, const int worker_idx,
                   const NotifierPoolRouter::NotifierId &notifier_id, void *&data);

    Server *root_server() const { return root_server_; }

    const HshaServerConfig *config_{nullptr};
    ServerMonitorPtr hsha_server_monitor_;
    HshaServerStat hsha_server_stat_;
    HshaServerQos hsha_server_qos_;
    HshaServerAcceptor hsha_server_acceptor_;

    std::vector<HshaServerUnit *> server_unit_list_;

    std::thread hsha_accept_thread_;

  private:
    void LoopReadCrossUnitResponse();

    Server *root_server_{nullptr};
    NotifierPoolRouter notifier_pool_router_;
};


}  //namespace phxrpc

