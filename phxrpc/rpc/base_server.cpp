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

#include "base_server.h"

#include <cassert>
#include <chrono>
#include <random>
#include <utility>
#include <sched.h>
#include <unistd.h>

#include "phxrpc/file.h"
#include "phxrpc/http.h"
#include "phxrpc/mqtt.h"
#include "phxrpc/msg.h"
#include "phxrpc/network.h"

#include "server_monitor.h"
#include "monitor_factory.h"


using namespace std;


namespace phxrpc {


DataFlow::DataFlow() {
}

DataFlow::~DataFlow() {
}

void DataFlow::PushRequest(void *args, BaseRequest *req) {
    in_queue_.push(make_pair(QueueExtData(args), req));
}

int DataFlow::PluckRequest(void *&args, BaseRequest *&req) {
    pair<QueueExtData, BaseRequest *> rp;
    bool succ = in_queue_.pluck(rp);
    if (!succ) {
        return 0;
    }
    args = rp.first.args;
    req = rp.second;

    auto now_time = Timer::GetSteadyClockMS();
    return now_time > rp.first.enqueue_time_ms ? now_time - rp.first.enqueue_time_ms : 0;
}

int DataFlow::PickRequest(void *&args, BaseRequest *&req) {
    pair<QueueExtData, BaseRequest *> rp;
    bool succ = in_queue_.pick(rp);
    if (!succ) {
        return 0;
    }
    args = rp.first.args;
    req = rp.second;

    auto now_time(Timer::GetSteadyClockMS());
    return now_time > rp.first.enqueue_time_ms ? now_time - rp.first.enqueue_time_ms : 0;
}

void DataFlow::PushResponse(void *args, BaseResponse *resp) {
    out_queue_.push(make_pair(QueueExtData(args), resp));
}

int DataFlow::PluckResponse(void *&args, BaseResponse *&resp) {
    pair<QueueExtData, BaseResponse *> rp;
    bool succ = out_queue_.pluck(rp);
    if (!succ) {
        return 0;
    }
    args = rp.first.args;
    resp = rp.second;

    auto now_time(Timer::GetSteadyClockMS());
    return now_time > rp.first.enqueue_time_ms ? now_time - rp.first.enqueue_time_ms : 0;
}

int DataFlow::PickResponse(void *&args, BaseResponse *&resp) {
    pair<QueueExtData, BaseResponse *> rp;
    bool succ = out_queue_.pick(rp);
    if (!succ) {
        return 0;
    }
    args = rp.first.args;
    resp = rp.second;

    auto now_time(Timer::GetSteadyClockMS());
    return now_time > rp.first.enqueue_time_ms ? now_time - rp.first.enqueue_time_ms : 0;
}

bool DataFlow::CanPushRequest(const int max_queue_length) {
    return in_queue_.size() < (size_t)max_queue_length;
}

bool DataFlow::CanPluckRequest() {
    return !in_queue_.empty();
}

bool DataFlow::CanPluckResponse() {
    return !out_queue_.empty();
}

void DataFlow::BreakOut() {
    in_queue_.break_out();
    out_queue_.break_out();
}


HshaServerStat::TimeCost::TimeCost() {
    now_time_ms_ = Timer::GetSteadyClockMS();
}

HshaServerStat::TimeCost::~TimeCost() {
}

int HshaServerStat::TimeCost::Cost() {
    auto now_time_ms =  Timer::GetSteadyClockMS();
    auto cost_time_ms = now_time_ms > now_time_ms_ ? now_time_ms - now_time_ms_ : 0;
    now_time_ms_ = now_time_ms;
    return cost_time_ms;
}

HshaServerStat::HshaServerStat(const HshaServerConfig *config, ServerMonitorPtr hsha_server_monitor) :
    /* config_(config), */ thread_(&HshaServerStat::CalFunc, this), break_out_(false),
    hsha_server_monitor_(hsha_server_monitor) {
    hold_fds_ = 0;
    accepted_fds_ = 0;
    accept_qps_ = 0;
    rejected_fds_ = 0;
    reject_qps_ = 0;
    queue_full_rejected_after_accepted_fds_ = 0;
    queue_full_rejected_after_accepted_qps_ = 0;
    accept_fail_ = 0;
    accept_fail_qps_ = 0;

    io_read_requests_ = 0;
    io_read_request_qps_ = 0;
    io_read_bytes_ = 0;

    io_write_responses_ = 0;
    io_write_response_qps_ = 0;
    io_write_bytes_ = 0;

    io_read_fails_ = 0;
    io_read_fail_qps_ = 0;
    io_write_fails_ = 0;
    io_write_fail_qps_ = 0;

    inqueue_push_requests_ = 0;
    inqueue_push_qps_ = 0;
    inqueue_pop_requests_ = 0;
    inqueue_pop_qps_ = 0;

    outqueue_push_responses_ = 0;
    outqueue_push_qps_ = 0;
    outqueue_pop_responses_ = 0;
    outqueue_pop_qps_ = 0;

    worker_timeouts_ = 0;
    worker_timeout_qps_ = 0;

    rpc_time_costs_ = 0;
    rpc_time_costs_count_ = 0;
    rpc_avg_time_cost_per_second_ = 0;
    rpc_time_cost_per_period_ = 0;

    inqueue_wait_time_costs_ = 0;
    inqueue_wait_time_costs_count_ = 0;
    inqueue_avg_wait_time_costs_per_second_ = 0;
    inqueue_avg_wait_time_costs_per_second_cal_seq_ = 0;
    inqueue_wait_time_costs_per_period_ = 0;

    outqueue_wait_time_costs_ = 0;
    outqueue_wait_time_costs_count_ = 0;
    outqueue_wait_time_costs_per_period_ = 0;
    outqueue_avg_wait_time_costs_per_second_ = 0;

    enqueue_fast_rejects_ = 0;
    enqueue_fast_reject_qps_ = 0;

    worker_idles_ = 0;

    worker_drop_requests_ = 0;
    worker_drop_reqeust_qps_ = 0;
    worker_time_costs_ = 0;
    worker_time_costs_count_ = 0;
    worker_avg_time_cost_per_second_ = 0;
    worker_time_cost_per_period_ = 0;
    worker_time_costs_per_second_ = 0;
}

HshaServerStat::~HshaServerStat() {
    break_out_ = true;
    cv_.notify_all();
    thread_.join();
}

void HshaServerStat::MonitorReport() {
    // accept
    hsha_server_monitor_->Accept(accept_qps_);
    hsha_server_monitor_->AcceptFail(accept_fail_qps_);
    hsha_server_monitor_->FastRejectAfterAccept(reject_qps_);

    // io
    hsha_server_monitor_->ReadError(io_read_fail_qps_);
    hsha_server_monitor_->SendError(io_write_fail_qps_);
    hsha_server_monitor_->OutOfQueue(queue_full_rejected_after_accepted_qps_);
    hsha_server_monitor_->QueueDelay(rpc_time_cost_per_period_);
    hsha_server_monitor_->FastRejectAfterRead(enqueue_fast_reject_qps_);
    hsha_server_monitor_->RecvBytes(io_read_bytes_qps_);
    hsha_server_monitor_->SendBytes(io_write_bytes_qps_);
    hsha_server_monitor_->WaitInInQueue(inqueue_wait_time_costs_per_period_);
    hsha_server_monitor_->WaitInOutQueue(outqueue_wait_time_costs_per_period_);

    // worker
    hsha_server_monitor_->RequestCount(accept_qps_);
    hsha_server_monitor_->ResponseCount(io_write_response_qps_);
    hsha_server_monitor_->RequestCost(worker_time_costs_per_second_);
    hsha_server_monitor_->WrokerInQueueTimeout(worker_drop_reqeust_qps_);
}

void HshaServerStat::CalFunc() {
    while (!break_out_) {
        unique_lock<mutex> lock(mutex_);
        cv_.wait_for(lock, chrono::seconds(1));

        //acceptor
        accept_qps_ = static_cast<int>(accepted_fds_);
        accepted_fds_ = 0;
        reject_qps_ = static_cast<int>(rejected_fds_);
        rejected_fds_ = 0;
        queue_full_rejected_after_accepted_qps_ = static_cast<int>(queue_full_rejected_after_accepted_fds_);
        queue_full_rejected_after_accepted_fds_ = 0;
        accept_fail_qps_ = static_cast<int>(accept_fail_);
        accept_fail_ = 0;

        //io
        io_read_request_qps_ = static_cast<int>(io_read_requests_);
        io_read_requests_ = 0;
        io_write_response_qps_ = static_cast<int>(io_write_responses_);
        io_write_responses_ = 0;

        io_read_bytes_qps_ = static_cast<int>(io_read_bytes_);
        io_read_bytes_ = 0;
        io_write_bytes_qps_ = static_cast<int>(io_write_bytes_);
        io_write_bytes_ = 0;

        io_read_fail_qps_ = static_cast<int>(io_read_fails_);
        io_read_fails_ = 0;
        io_write_fail_qps_ = static_cast<int>(io_write_fails_);
        io_write_fails_ = 0;

        //queue
        inqueue_push_qps_ = static_cast<int>(inqueue_push_requests_);
        inqueue_push_requests_ = 0;
        inqueue_pop_qps_ = static_cast<int>(inqueue_pop_requests_);
        inqueue_pop_requests_ = 0;

        outqueue_push_qps_ = static_cast<int>(outqueue_push_responses_);
        outqueue_push_responses_ = 0;
        outqueue_pop_qps_ = static_cast<int>(outqueue_pop_responses_);
        outqueue_pop_responses_ = 0;

        //worker
        worker_timeout_qps_ = static_cast<int>(worker_timeouts_);
        worker_timeouts_ = 0;

        //time cost
        rpc_time_cost_per_period_ = 0;
        if (rpc_time_costs_count_ >= RPC_TIME_COST_CAL_RATE) {
            rpc_avg_time_cost_per_second_ =
                static_cast<long>(rpc_time_costs_) / rpc_time_costs_count_;
            rpc_time_cost_per_period_ = static_cast<long>(rpc_time_costs_);
            rpc_time_costs_ = 0;
            rpc_time_costs_count_ = 0;
        }

        //worker time cost
        worker_time_cost_per_period_ = 0;
        if (worker_time_costs_count_ >= RPC_TIME_COST_CAL_RATE) {
            worker_avg_time_cost_per_second_ =
                static_cast<long>(worker_time_costs_) / worker_time_costs_count_;
            worker_time_cost_per_period_ = static_cast<long>(worker_time_costs_);
            worker_time_costs_ = 0;
            worker_time_costs_count_ = 0;
        }

        inqueue_wait_time_costs_per_period_ = 0;
        if (inqueue_wait_time_costs_count_ >= QUEUE_WAIT_TIME_COST_CAL_RATE) {
            inqueue_avg_wait_time_costs_per_second_ =
                static_cast<long>(inqueue_wait_time_costs_) / inqueue_wait_time_costs_count_;
            inqueue_wait_time_costs_per_period_ = static_cast<long>(inqueue_wait_time_costs_);
            inqueue_wait_time_costs_ = 0;
            inqueue_wait_time_costs_count_ = 0;
            inqueue_avg_wait_time_costs_per_second_cal_seq_++;
        }

        outqueue_wait_time_costs_per_period_ = 0;
        if (outqueue_wait_time_costs_count_ >= QUEUE_WAIT_TIME_COST_CAL_RATE) {
            outqueue_avg_wait_time_costs_per_second_ =
                static_cast<long>(outqueue_wait_time_costs_) / outqueue_wait_time_costs_count_;
            outqueue_wait_time_costs_per_period_ = static_cast<long>(outqueue_wait_time_costs_);
            outqueue_wait_time_costs_ = 0;
            outqueue_wait_time_costs_count_ = 0;
        }

        enqueue_fast_reject_qps_ = static_cast<int>(enqueue_fast_rejects_);
        enqueue_fast_rejects_ = 0;

        worker_drop_reqeust_qps_ = static_cast<int>(worker_drop_requests_);
        worker_drop_requests_ = 0;

        worker_time_costs_per_second_ = static_cast<long>(worker_time_costs_);
        worker_time_costs_ = 0;

        MonitorReport();

        phxrpc::log(LOG_NOTICE, "[SERVER_STAT] hold_fds %d accept_qps %d accept_reject_qps %d queue_full_reject_qps %d"
                " read_request_qps %d write_response_qps %d"
                " inqueue_push_qps %d rpc_time_cost_avg %d worker_time_cost_avg %d"
                " inqueue_wait_time_avg %d outqueue_wait_time_qvg %d"
                " fast_reject_qps %d"
                " worker_idles %d worker_drop_request_qps %d io_read_fails %d, io_write_fails %d",
                static_cast<int>(hold_fds_), accept_qps_, reject_qps_, queue_full_rejected_after_accepted_qps_,
                io_read_request_qps_, io_write_response_qps_,
                inqueue_push_qps_, rpc_avg_time_cost_per_second_, worker_avg_time_cost_per_second_,
                inqueue_avg_wait_time_costs_per_second_, outqueue_avg_wait_time_costs_per_second_,
                enqueue_fast_reject_qps_,
                static_cast<int>(worker_idles_), worker_drop_reqeust_qps_, io_read_fail_qps_, io_write_fail_qps_ );

    }
}


HshaServerQos::HshaServerQos(const HshaServerConfig *config, HshaServerStat *hsha_server_stat)
        : config_(config), hsha_server_stat_(hsha_server_stat),
          thread_(&HshaServerQos::CalFunc, this) {
}

HshaServerQos::~HshaServerQos() {
    break_out_ = true;
    cv_.notify_all();
    thread_.join();
}

bool HshaServerQos::CanAccept() {
    return static_cast<int>(hsha_server_stat_->hold_fds_) < config_->GetMaxConnections();
}

bool HshaServerQos::CanEnqueue() {
    static default_random_engine e_rand((int)time(nullptr));
    return ((int)(e_rand() % 100)) >= enqueue_reject_rate_;
}

void HshaServerQos::CalFunc() {
    while (!break_out_) {
        unique_lock<mutex> lock(mutex_);
        cv_.wait_for(lock, chrono::seconds(1));

        //fast reject
        if (hsha_server_stat_->inqueue_avg_wait_time_costs_per_second_cal_seq_
                != inqueue_avg_wait_time_costs_per_second_cal_last_seq_) {
            //inqueue avg wait time reflesh
            int avg_queue_wait_time = (hsha_server_stat_->inqueue_avg_wait_time_costs_per_second_
                    + hsha_server_stat_->outqueue_avg_wait_time_costs_per_second_) / 2;

            int rate = config_->GetFastRejectAdjustRate();
            if (avg_queue_wait_time > config_->GetFastRejectThresholdMS()) {
                if (enqueue_reject_rate_ != 99) {
                    enqueue_reject_rate_ = enqueue_reject_rate_ + rate > 99 ? 99 : enqueue_reject_rate_ + rate;
                }
            } else {
                if (enqueue_reject_rate_ != 0) {
                    enqueue_reject_rate_ = enqueue_reject_rate_ - rate < 0 ? 0 : enqueue_reject_rate_ - rate;
                }
            }
            inqueue_avg_wait_time_costs_per_second_cal_last_seq_ =
                hsha_server_stat_->inqueue_avg_wait_time_costs_per_second_cal_seq_;
        }

        phxrpc::log(LOG_NOTICE, "[SERVER_QOS] accept_reject_qps %d queue_full_reject_qps %d"
                " fast_reject_qps %d fast_reject_rate %d",
                hsha_server_stat_->reject_qps_, hsha_server_stat_->queue_full_rejected_after_accepted_qps_,
                hsha_server_stat_->enqueue_fast_reject_qps_, enqueue_reject_rate_);
    }
}


NotifierPoolRouter::NotifierId::NotifierId(const uint64_t session_id_value,
                                           const uint32_t packet_id_value)
        : session_id(session_id_value), packet_id(packet_id_value) {
}

__uint128_t NotifierPoolRouter::NotifierId::ToUint128() const {
    return (static_cast<__uint128_t>(session_id) << 64 | packet_id);
}

void NotifierPoolRouter::NotifierId::FromUint128(const __uint128_t &value) {
    session_id = static_cast<uint64_t>(value >> 64);
    packet_id = static_cast<uint32_t>(value);
}


void NotifierPoolRouter::Add(const NotifierId &notifier_id, const pair<int, int> idx) {
    Add(move(notifier_id.ToUint128()), idx);
}

pair<int, int> NotifierPoolRouter::Get(const NotifierId &notifier_id) const {
    return Get(move(notifier_id.ToUint128()));
}

void NotifierPoolRouter::Delete(const NotifierId &notifier_id) {
    Delete(move(notifier_id.ToUint128()));
}

void NotifierPoolRouter::Add(const __uint128_t &session_packet_id, const pair<int, int> idx) {
    lock_guard<mutex> lock(mutex_);
    session_packet_id2idx_map_[session_packet_id] = idx;
}

pair<int, int> NotifierPoolRouter::Get(const __uint128_t &session_packet_id) const {
    lock_guard<mutex> lock(mutex_);
    const auto &it(session_packet_id2idx_map_.find(session_packet_id));
    if (session_packet_id2idx_map_.end() == it) {
        return make_pair(-1, -1);
    }

    return it->second;
}

void NotifierPoolRouter::Delete(const __uint128_t &session_packet_id) {
    lock_guard<mutex> lock(mutex_);
    session_packet_id2idx_map_.erase(session_packet_id);
}


Worker::Worker(const int idx, WorkerPool *const pool,
               const int uthread_count, int uthread_stack_size)
        : idx_(idx), pool_(pool), uthread_count_(uthread_count),
          uthread_stack_size_(uthread_stack_size),
          thread_(&Worker::Func, this) {
}

Worker::~Worker() {
    thread_.join();
    delete worker_scheduler_;
}

void Worker::Func() {
    if (uthread_count_ == 0) {
        ThreadMode();
    } else {
        UThreadMode();
    }
}

void Worker::ThreadMode() {
    while (!shut_down_) {
        pool_->hsha_server_stat_->worker_idles_++;

        void *args{nullptr};
        BaseRequest *request{nullptr};
        int queue_wait_time_ms{pool_->data_flow_->PluckRequest(args, request)};
        if (request == nullptr) {
            //break out
            continue;
        }
        pool_->hsha_server_stat_->worker_idles_--;

        WorkerLogic(args, request, queue_wait_time_ms);
    }
}

void Worker::UThreadMode() {
    worker_scheduler_ = new UThreadEpollScheduler(uthread_stack_size_, uthread_count_, true);
    assert(worker_scheduler_ != nullptr);
    worker_scheduler_->SetHandlerNewRequestFunc(bind(&Worker::HandlerNewRequestFunc, this));
    notifier_pool_ = new UThreadNotifierPool(worker_scheduler_, pool_->config_->GetSocketTimeoutMS());
    worker_scheduler_->RunForever();
}

void Worker::HandlerNewRequestFunc() {
    if (worker_scheduler_->IsTaskFull()) {
        return;
    }

    void *args{nullptr};
    BaseRequest *request{nullptr};
    int queue_wait_time_ms{pool_->data_flow_->PickRequest(args, request)};
    if (!request) {
        return;
    }

    worker_scheduler_->AddTask(bind(&Worker::UThreadFunc, this, args,
                                    request, queue_wait_time_ms), nullptr);
}

void Worker::UThreadFunc(void *args, BaseRequest *req, int queue_wait_time_ms) {
    WorkerLogic(args, req, queue_wait_time_ms);
}

void Worker::WorkerLogic(void *args, BaseRequest *req, int queue_wait_time_ms) {
    pool_->hsha_server_stat_->inqueue_pop_requests_++;
    pool_->hsha_server_stat_->inqueue_wait_time_costs_ += queue_wait_time_ms;
    pool_->hsha_server_stat_->inqueue_wait_time_costs_count_++;

    BaseResponse *resp{req->GenResponse()};
    if (queue_wait_time_ms < MAX_QUEUE_WAIT_TIME_COST) {
        HshaServerStat::TimeCost time_cost;

        DispatcherArgs_t dispatcher_args(pool_->idx_, idx_, pool_->hsha_server_stat_->hsha_server_monitor_,
                worker_scheduler_, pool_->root_server_, pool_->base_server_unit_,
                notifier_pool_, pool_->notifier_pool_router_,
                pool_->args_, args);
        pool_->dispatch_(req, resp, &dispatcher_args);

        pool_->hsha_server_stat_->worker_time_costs_ += time_cost.Cost();
        pool_->hsha_server_stat_->worker_time_costs_count_++;
    } else {
        pool_->hsha_server_stat_->worker_drop_requests_++;
    }
    // fa should also PushResponse, otherwise session_id (which args points to) will memory leak
    pool_->data_flow_->PushResponse(args, resp);
    pool_->hsha_server_stat_->outqueue_push_responses_++;

    pool_->scheduler_->NotifyEpoll();

    delete req;
}

void Worker::NotifyEpoll() {
    if (uthread_count_ == 0) {
        return;
    }

    worker_scheduler_->NotifyEpoll();
}

int Worker::NotifyTarget(const NotifierPoolRouter::NotifierId &notifier_id, void *const data) {
    UThreadNotifier *notifier{nullptr};
    notifier_pool_->GetNotifier(move(notifier_id.ToUint128()), notifier);
    if (!notifier)
        return -1;

    return notifier->SendNotify(data);
}

void Worker::Shutdown() {
    shut_down_ = true;
    pool_->data_flow_->BreakOut();
}


WorkerPool::WorkerPool(const int idx,
                       UThreadEpollScheduler *scheduler,
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
                       void *args)
        : idx_(idx), scheduler_(scheduler), config_(config), root_server_(root_server), base_server_unit_(base_server_unit),
          notifier_pool_router_(notifier_pool_router),
          data_flow_(data_flow),
          hsha_server_stat_(hsha_server_stat), dispatch_(dispatch),
          args_(args), last_notify_idx_(0) {
    for (int i{0}; i < thread_count; ++i) {
        auto worker(new Worker(i, this, uthread_count_per_thread, uthread_stack_size));
        assert(worker != nullptr);
        worker_list_.push_back(worker);
    }
}

WorkerPool::~WorkerPool() {
    for (auto &worker : worker_list_) {
        worker->Shutdown();
        delete worker;
    }
}

void WorkerPool::NotifyEpoll() {
    lock_guard<mutex> lock(mutex_);
    if (last_notify_idx_ == worker_list_.size()) {
        last_notify_idx_ = 0;
    }

    worker_list_[last_notify_idx_++]->NotifyEpoll();
}

int WorkerPool::NotifyTarget(const int idx, const NotifierPoolRouter::NotifierId &notifier_id,
                             void *const data) {
    return worker_list_[idx]->NotifyTarget(notifier_id, data);
}


}  //namespace phxrpc

