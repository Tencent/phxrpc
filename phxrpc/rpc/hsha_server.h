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

#include <map>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "server_config.h"
#include "thread_queue.h"

#include "phxrpc/network.h"
#include "phxrpc/http.h"
#include "server_base.h"
#include "server_monitor.h"

namespace phxrpc {

class WorkerPool;

class DataFlow {
public:
    DataFlow();
    ~DataFlow();

    void PushRequest(void * args, HttpRequest * request);
    int PluckRequest(void *& args, HttpRequest *& request);
    int PickRequest(void *& args, HttpRequest *& request);
    void PushResponse(void * args, HttpResponse * response);
    int PluckResponse(void *& args, HttpResponse *& response);
    bool CanPushRequest(const int max_queue_length);
    bool CanPluckResponse();

    void BreakOut();

private:
    struct QueueExtData {
        QueueExtData() {
            enqueue_time_ms = 0;
            args = nullptr;
        }
        QueueExtData(void * t_args) {
            enqueue_time_ms = Timer::GetSteadyClockMS();
            args = t_args;
        }
        uint64_t enqueue_time_ms;
        void * args;
    };
    ThdQueue<std::pair<QueueExtData, HttpRequest *> > in_queue_;
    ThdQueue<std::pair<QueueExtData, HttpResponse *> > out_queue_;
};

/////////////////////////////////

#define RPC_TIME_COST_CAL_RATE 1000 
#define QUEUE_WAIT_TIME_COST_CAL_RATE 1000 
#define MAX_QUEUE_WAIT_TIME_COST 500
#define MAX_ACCEPT_QUEUE_LENGTH 102400 

class HshaServerStat {
public:
    HshaServerStat(const HshaServerConfig * config, ServerMonitorPtr hsha_server_monitor);
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
    friend class WorkerPool;
    friend class Worker;
    friend class HshaServerQos;
    friend class HshaServerAcceptor;
    const HshaServerConfig * config_;
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
    long worker_time_costs_per_second_;
};


//////////////////////////////////

class HshaServerQos {
public:
    HshaServerQos(const HshaServerConfig * config, HshaServerStat * hsha_server_stat);
    ~HshaServerQos();

    void CalFunc();
    bool CanAccept();
    bool CanEnqueue();

private:
    const HshaServerConfig * config_;
    HshaServerStat * hsha_server_stat_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
    bool break_out_;
    int enqueue_reject_rate_;
    int inqueue_avg_wait_time_costs_per_second_cal_last_seq_;
};

//////////////////////////////////

class Worker {
public:
    Worker(WorkerPool * pool, int uthread_count, int utherad_stack_size);
    ~Worker();

    void Func(); 
    void Shutdown();

    void ThreadMode();
    void UThreadMode();
    void HandlerNewRequestFunc();
    void UThreadFunc(void * args, HttpRequest * request, int queue_wait_time_ms);
    void WorkerLogic(void * args, HttpRequest * request, int queue_wait_time_ms);
    void Notify();

private:
    WorkerPool * pool_;
    int uthread_count_;
    int utherad_stack_size_;
    bool shut_down_;
    UThreadEpollScheduler * worker_scheduler_;
    std::thread thread_;
};

/////////////////////////////////

typedef std::function< void(const HttpRequest &, HttpResponse *, DispatcherArgs_t *) > Dispatch_t;

class WorkerPool {
public:
    WorkerPool(UThreadEpollScheduler * scheduler, 
            int thread_count, 
            int uthread_count_per_thread,
            int utherad_stack_size,
            DataFlow * data_flow, 
            HshaServerStat * hsha_server_stat, 
            Dispatch_t dispatch, 
            void * args);
    ~WorkerPool();

    void Notify();

private:
    friend class Worker;
    UThreadEpollScheduler * scheduler_;
    DataFlow * data_flow_;
    HshaServerStat * hsha_server_stat_;
    Dispatch_t dispatch_;
    void * args_;
    std::vector<Worker *> worker_list_;
    size_t last_notify_idx_;
    std::mutex mutex_;
};

/////////////////////////////////

class HshaServerIO {
public:
    HshaServerIO(int idx, UThreadEpollScheduler * scheduler, const HshaServerConfig * config, 
            DataFlow * data_flow, HshaServerStat * hsha_server_stat, HshaServerQos * hsha_server_qos,
            WorkerPool * worker_pool);
    ~HshaServerIO();

    void RunForever();

    bool AddAcceptedFd(int accepted_fd);

    void HandlerAcceptedFd();

    void IOFunc(int accept_fd);

    UThreadSocket_t * ActiveSocketFunc();

private:
    int idx_;
    UThreadEpollScheduler * scheduler_;
    const HshaServerConfig * config_;
    DataFlow * data_flow_;
    int listen_fd_;
    HshaServerStat * hsha_server_stat_;
    HshaServerQos * hsha_server_qos_;
    WorkerPool * worker_pool_;

    std::queue<int> accepted_fd_list_;
    std::mutex queue_mutex_;
};

/////////////////////////////////

class HshaServer;
class HshaServerUnit {
public:
    HshaServerUnit(HshaServer * hsha_server, 
            int idx, 
            int worker_thread_count, 
            int worker_uthread_count_per_thread,
            int worker_utherad_stack_size,
            Dispatch_t dispatch, 
            void * args);
    ~HshaServerUnit();

    void RunFunc();
    bool AddAcceptedFd(int accepted_fd);

private:
    HshaServer * hsha_server_;
    UThreadEpollScheduler scheduler_;
    DataFlow data_flow_;
    WorkerPool worker_pool_;
    HshaServerIO hsha_server_io_;
    std::thread thread_;
};

/////////////////////////////////

class HshaServerAcceptor {
public:
    HshaServerAcceptor(HshaServer * hsha_server);
    ~HshaServerAcceptor();

    void LoopAccept(const char * bind_ip, const int port);

private:
    HshaServer * hsha_server_;
    size_t idx_;
};

/////////////////////////////////

class HshaServer {
public:
    HshaServer(const HshaServerConfig & config, Dispatch_t dispatch, void * args);
    ~HshaServer();

    void RunForever();

    const HshaServerConfig * config_;
    ServerMonitorPtr hsha_server_monitor_;
    HshaServerStat hsha_server_stat_;
    HshaServerQos hsha_server_qos_;
    HshaServerAcceptor hsha_server_acceptor_;

    std::vector<HshaServerUnit *> server_unit_list_;
};

} //namespace phxrpc
