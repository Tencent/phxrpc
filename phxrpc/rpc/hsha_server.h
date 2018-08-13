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

#include <memory>
#include <thread>

#include "phxrpc/http.h"
#include "phxrpc/msg.h"

#include "phxrpc/rpc/server_base.h"
#include "phxrpc/rpc/server_config.h"
#include "phxrpc/rpc/server_monitor.h"
#include "phxrpc/rpc/thread_queue.h"


namespace phxrpc {


class DataFlow final {
  public:
    DataFlow();
    ~DataFlow();

    void PushRequest(void *args, BaseRequest *req);
    int PluckRequest(void *&args, BaseRequest *&req);
    int PickRequest(void *&args, BaseRequest *&req);
    void PushResponse(void *args, BaseResponse *resp);
    int PluckResponse(void *&args, BaseResponse *&resp);
    int PickResponse(void *&args, BaseResponse *&resp);
    bool CanPushRequest(const int max_queue_length);
    bool CanPushResponse(const int max_queue_length);
    bool CanPluckRequest();
    bool CanPluckResponse();

    void BreakOut();

  private:
    struct QueueExtData {
        QueueExtData() {
            enqueue_time_ms = 0;
            args = nullptr;
        }
        QueueExtData(void *t_args) {
            enqueue_time_ms = Timer::GetSteadyClockMS();
            args = t_args;
        }
        uint64_t enqueue_time_ms;
        void *args;
    };

    ThdQueue<std::pair<QueueExtData, BaseRequest *>> in_queue_;
    ThdQueue<std::pair<QueueExtData, BaseResponse *>> out_queue_;
};


#define RPC_TIME_COST_CAL_RATE 1000
#define QUEUE_WAIT_TIME_COST_CAL_RATE 1000
#define MAX_QUEUE_WAIT_TIME_COST 500
#define MAX_ACCEPT_QUEUE_LENGTH 102400


class WorkerPool;

class HshaServerStat final {
  public:
    HshaServerStat(const HshaServerConfig *config, ServerMonitorPtr hsha_server_monitor);
    ~HshaServerStat();

    void CalFunc();

    class TimeCost {
      public:
        TimeCost();
        ~TimeCost();
        int Cost();
      private:
        uint64_t now_time_ms_;
    };

  private:
    void MonitorReport();

    friend class HshaServerIO;
    friend class FaServerIO;
    friend class WorkerPool;
    friend class Worker;
    friend class HshaServerQos;
    friend class HshaServerAcceptor;
    friend class FaServerAcceptor;
    //const HshaServerConfig *config_;

  public:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
    bool break_out_;
    ServerMonitorPtr hsha_server_monitor_;

    std::atomic_int hold_fds_;
    std::atomic_int accepted_fds_;
    int accept_qps_;
    std::atomic_int rejected_fds_;
    int reject_qps_;
    std::atomic_int queue_full_rejected_after_accepted_fds_;
    int queue_full_rejected_after_accepted_qps_;
    std::atomic_int accept_fail_;
    int accept_fail_qps_;

    std::atomic_int io_read_requests_;
    int io_read_request_qps_;
    std::atomic_int io_write_responses_;
    int io_write_response_qps_;

    std::atomic_int io_read_bytes_;
    int io_read_bytes_qps_;
    std::atomic_int io_write_bytes_;
    int io_write_bytes_qps_;

    std::atomic_int io_read_fails_;
    int io_read_fail_qps_;
    std::atomic_int io_write_fails_;
    int io_write_fail_qps_;

    std::atomic_int inqueue_push_requests_;
    int inqueue_push_qps_;
    std::atomic_int inqueue_pop_requests_;
    int inqueue_pop_qps_;

    std::atomic_int outqueue_push_responses_;
    int outqueue_push_qps_;
    std::atomic_int outqueue_pop_responses_;
    int outqueue_pop_qps_;

    std::atomic_int worker_timeouts_;
    int worker_timeout_qps_;

    std::atomic_long rpc_time_costs_;
    std::atomic_int rpc_time_costs_count_;
    int rpc_avg_time_cost_per_second_;
    int rpc_time_cost_per_period_;

    std::atomic_long inqueue_wait_time_costs_;
    std::atomic_int inqueue_wait_time_costs_count_;
    int inqueue_avg_wait_time_costs_per_second_;
    int inqueue_avg_wait_time_costs_per_second_cal_seq_;
    long inqueue_wait_time_costs_per_period_;

    std::atomic_long outqueue_wait_time_costs_;
    std::atomic_int outqueue_wait_time_costs_count_;
    int outqueue_avg_wait_time_costs_per_second_;
    long outqueue_wait_time_costs_per_period_;

    std::atomic_int enqueue_fast_rejects_;
    int enqueue_fast_reject_qps_;

    std::atomic_int worker_idles_;

    std::atomic_int worker_drop_requests_;
    int worker_drop_reqeust_qps_;

    std::atomic_long worker_time_costs_;
    std::atomic_int worker_time_costs_count_;
    int worker_avg_time_cost_per_second_;
    int worker_time_cost_per_period_;
    long worker_time_costs_per_second_;
};


class HshaServerQos final {
  public:
    HshaServerQos(const HshaServerConfig *config, HshaServerStat *hsha_server_stat);
    ~HshaServerQos();

    void CalFunc();
    bool CanAccept();
    bool CanEnqueue();

  private:
    const HshaServerConfig *config_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
    bool break_out_{false};
    int enqueue_reject_rate_{0};
    int inqueue_avg_wait_time_costs_per_second_cal_last_seq_{0};
};


class Worker final {
  public:
    Worker(const int idx, WorkerPool *const pool,
           const int uthread_count, const int uthread_stack_size);
    ~Worker();

    void Func();
    void Shutdown();

    void ThreadMode();
    void UThreadMode();
    void HandlerNewRequestFunc();
    void UThreadFunc(void *args, BaseRequest *req, int queue_wait_time_ms);
    void WorkerLogic(void *args, BaseRequest *req, int queue_wait_time_ms);
    void NotifyEpoll();

  private:
    int idx_{-1};
    WorkerPool *pool_{nullptr};
    int uthread_count_;
    int uthread_stack_size_;
    bool shut_down_{false};
    UThreadEpollScheduler *worker_scheduler_{nullptr};
    std::thread thread_;
};


typedef std::function<void (const BaseRequest &, BaseResponse *const, DispatcherArgs_t *const)> Dispatch_t;

class ServerMgr;

class WorkerPool final {
  public:
    WorkerPool(const int idx, UThreadEpollScheduler *const scheduler,
               const HshaServerConfig *const config,
               const int thread_count,
               const int uthread_count_per_thread,
               const int uthread_stack_size,
               DataFlow *const data_flow,
               HshaServerStat *const hsha_server_stat,
               Dispatch_t dispatch,
               void *args);
    ~WorkerPool();

    void NotifyEpoll();

  private:
    friend class Worker;
    int idx_{-1};
    UThreadEpollScheduler *scheduler_{nullptr};
    const HshaServerConfig *config_{nullptr};
    DataFlow *data_flow_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    Dispatch_t dispatch_;
    void *args_{nullptr};
    std::vector<Worker *> worker_list_;
    size_t last_notify_idx_;
    std::mutex mutex_;
};


class HshaServerIO final {
  public:
    HshaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
                 const HshaServerConfig *config,
                 DataFlow *data_flow, HshaServerStat *hsha_server_stat,
                 HshaServerQos *hsha_server_qos, WorkerPool *worker_pool,
                 phxrpc::BaseMessageHandlerFactoryCreateFunc msg_handler_factory_create_func);
    ~HshaServerIO();

    void RunForever();
    bool AddAcceptedFd(const int accepted_fd);
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
    std::unique_ptr<BaseMessageHandlerFactory> msg_handler_factory_;
    std::queue<int> accepted_fd_list_;
    std::mutex queue_mutex_;
};


class HshaServer;

class HshaServerUnit {
  public:
    HshaServerUnit(const int idx,
                   HshaServer *const hsha_server,
                   int worker_thread_count,
                   int worker_uthread_count_per_thread,
                   int worker_uthread_stack_size,
                   Dispatch_t dispatch, void *args);
    virtual ~HshaServerUnit();

    void RunFunc();
    bool AddAcceptedFd(const int accepted_fd);

  private:
    HshaServer *hsha_server_{nullptr};
    UThreadEpollScheduler scheduler_;
    DataFlow data_flow_;
    WorkerPool worker_pool_;
    HshaServerIO hsha_server_io_;

    std::thread thread_;
};


class HshaServerAcceptor final {
  public:
    HshaServerAcceptor(HshaServer *hsha_server);
    ~HshaServerAcceptor();

    void LoopAccept(const char *const bind_ip, const int port);

  private:
    HshaServer *hsha_server_{nullptr};
    size_t idx_{0};
};


class HshaServer {
  public:
    HshaServer(const HshaServerConfig &config, const Dispatch_t &dispatch, void *args,
               phxrpc::BaseMessageHandlerFactoryCreateFunc msg_handler_factory_create_func =
               []()->std::unique_ptr<phxrpc::HttpMessageHandlerFactory> {
        return std::unique_ptr<phxrpc::HttpMessageHandlerFactory>(new phxrpc::HttpMessageHandlerFactory);
    });
    virtual ~HshaServer();

    void RunForever();

  private:
    friend class HshaServerAcceptor;
    friend class HshaServerUnit;

    const HshaServerConfig *config_{nullptr};
    phxrpc::BaseMessageHandlerFactoryCreateFunc msg_handler_factory_create_func_;
    ServerMonitorPtr hsha_server_monitor_;
    HshaServerStat hsha_server_stat_;
    HshaServerQos hsha_server_qos_;
    HshaServerAcceptor hsha_server_acceptor_;

    std::vector<HshaServerUnit *> server_unit_list_;

    void LoopReadCrossUnitResponse();
};


}  //namespace phxrpc

