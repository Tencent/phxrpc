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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "server_config.h"
#include "thread_queue.h"

#include "phxrpc/network.h"
#include "phxrpc/msg.h"
#include "server_base.h"
#include "server_monitor.h"


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


class NotifierPoolRouter final {
  public:
    struct NotifierId {
        NotifierId(const uint64_t session_id_value, const uint32_t packet_id_value);

        __uint128_t ToUint128() const;
        void FromUint128(const __uint128_t &value);

        uint64_t session_id{0uLL};
        uint32_t packet_id{0u};
    };

    void Add(const NotifierId &notifier_id, const std::pair<int, int> idx);
    std::pair<int, int> Get(const NotifierId &notifier_id) const;
    void Delete(const NotifierId &notifier_id);

  private:
    void Add(const __uint128_t &session_packet_id, const std::pair<int, int> idx);
    std::pair<int, int> Get(const __uint128_t &session_packet_id) const;
    void Delete(const __uint128_t &session_packet_id);

    mutable std::mutex mutex_;
    std::map<__uint128_t, std::pair<int, int>> session_packet_id2idx_map_;
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
    int NotifyTarget(const NotifierPoolRouter::NotifierId &notifier_id, void *const data);

    UThreadNotifierPool *notifier_pool() { return notifier_pool_; }

  private:
    int idx_{-1};
    WorkerPool *pool_{nullptr};
    int uthread_count_;
    int uthread_stack_size_;
    bool shut_down_{false};
    UThreadEpollScheduler *worker_scheduler_{nullptr};
    // TODO: support thread mode
    UThreadNotifierPool *notifier_pool_{nullptr};
    std::thread thread_;
};


typedef std::function<void(const BaseRequest *, BaseResponse *, DispatcherArgs_t *)> Dispatch_t;

class Server;
class BaseServerUnit;

class WorkerPool final {
  public:
    WorkerPool(const int idx, UThreadEpollScheduler *scheduler,
               const HshaServerConfig *config,
               const int thread_count,
               const int uthread_count_per_thread,
               const int uthread_stack_size,
               Server *const root_server,
               BaseServerUnit *const base_server_unit,
               NotifierPoolRouter *const notifier_pool_router,
               DataFlow *const data_flow,
               HshaServerStat *const hsha_server_stat,
               Dispatch_t dispatch,
               void *args);
    ~WorkerPool();

    void NotifyEpoll();
    int NotifyTarget(const int idx, const NotifierPoolRouter::NotifierId &notifier_id, void *const data);

  private:
    friend class Worker;
    int idx_{-1};
    UThreadEpollScheduler *scheduler_{nullptr};
    const HshaServerConfig *config_{nullptr};
    Server *root_server_{nullptr};
    BaseServerUnit *base_server_unit_{nullptr};
    NotifierPoolRouter *notifier_pool_router_{nullptr};
    DataFlow *data_flow_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    Dispatch_t dispatch_;
    void *args_{nullptr};
    std::vector<Worker *> worker_list_;
    size_t last_notify_idx_;
    std::mutex mutex_;
};


class BaseServerUnit {
  public:
    BaseServerUnit() = default;
    virtual ~BaseServerUnit() = default;

  protected:
    DataFlow data_flow_;
};


class BaseServer {
  public:
    BaseServer() = default;
    virtual ~BaseServer() = default;

    virtual void DoRunForever() = 0;
};


}  //namespace phxrpc

