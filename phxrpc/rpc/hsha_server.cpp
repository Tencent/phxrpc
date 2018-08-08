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

#include "hsha_server.h"

#include <cassert>
#include <random>

#include "server_monitor.h"
#include "monitor_factory.h"


namespace phxrpc {


using namespace std;


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

bool DataFlow::CanPushResponse(const int max_queue_length) {
    return out_queue_.size() < (size_t)max_queue_length;
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
    auto now_time_ms = Timer::GetSteadyClockMS();
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

        // acceptor
        accept_qps_ = static_cast<int>(accepted_fds_);
        accepted_fds_ = 0;
        reject_qps_ = static_cast<int>(rejected_fds_);
        rejected_fds_ = 0;
        queue_full_rejected_after_accepted_qps_ = static_cast<int>(queue_full_rejected_after_accepted_fds_);
        queue_full_rejected_after_accepted_fds_ = 0;
        accept_fail_qps_ = static_cast<int>(accept_fail_);
        accept_fail_ = 0;

        // io
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

        // queue
        inqueue_push_qps_ = static_cast<int>(inqueue_push_requests_);
        inqueue_push_requests_ = 0;
        inqueue_pop_qps_ = static_cast<int>(inqueue_pop_requests_);
        inqueue_pop_requests_ = 0;

        outqueue_push_qps_ = static_cast<int>(outqueue_push_responses_);
        outqueue_push_responses_ = 0;
        outqueue_pop_qps_ = static_cast<int>(outqueue_pop_responses_);
        outqueue_pop_responses_ = 0;

        // worker
        worker_timeout_qps_ = static_cast<int>(worker_timeouts_);
        worker_timeouts_ = 0;

        // time cost
        rpc_time_cost_per_period_ = 0;
        if (rpc_time_costs_count_ >= RPC_TIME_COST_CAL_RATE) {
            rpc_avg_time_cost_per_second_ =
                static_cast<long>(rpc_time_costs_) / rpc_time_costs_count_;
            rpc_time_cost_per_period_ = static_cast<long>(rpc_time_costs_);
            rpc_time_costs_ = 0;
            rpc_time_costs_count_ = 0;
        }

        // worker time cost
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

        // fast reject
        if (hsha_server_stat_->inqueue_avg_wait_time_costs_per_second_cal_seq_
                != inqueue_avg_wait_time_costs_per_second_cal_last_seq_) {
            // inqueue avg wait time reflesh
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
            // break out
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

        DispatcherArgs_t dispatcher_args(pool_->hsha_server_stat_->hsha_server_monitor_,
                worker_scheduler_, pool_->args_, args);
        pool_->dispatch_(*req, resp, &dispatcher_args);

        pool_->hsha_server_stat_->worker_time_costs_ += time_cost.Cost();
        pool_->hsha_server_stat_->worker_time_costs_count_++;
    } else {
        pool_->hsha_server_stat_->worker_drop_requests_++;
    }
    // event loop server should also PushResponse, otherwise session_id (which args points to) will memory leak
    pool_->data_flow_->PushResponse(args, resp);
    pool_->hsha_server_stat_->outqueue_push_responses_++;

    pool_->scheduler_->NotifyEpoll();

    if (req) {
        delete req;
        req = nullptr;
    }
}

void Worker::NotifyEpoll() {
    if (uthread_count_ == 0) {
        return;
    }

    worker_scheduler_->NotifyEpoll();
}

void Worker::Shutdown() {
    shut_down_ = true;
    pool_->data_flow_->BreakOut();
}


WorkerPool::WorkerPool(const int idx,
                       UThreadEpollScheduler *const scheduler,
                       const HshaServerConfig *const config,
                       const int thread_count,
                       const int uthread_count_per_thread,
                       const int uthread_stack_size,
                       DataFlow *const data_flow,
                       HshaServerStat *const hsha_server_stat,
                       Dispatch_t dispatch,
                       void *args)
        : idx_(idx), scheduler_(scheduler), config_(config),
          data_flow_(data_flow), hsha_server_stat_(hsha_server_stat),
          dispatch_(dispatch), args_(args), last_notify_idx_(0) {
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


HshaServerIO::HshaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
                           const HshaServerConfig *config,
                           DataFlow *data_flow, HshaServerStat *hsha_server_stat,
                           HshaServerQos *hsha_server_qos, WorkerPool *worker_pool,
                           phxrpc::BaseMessageHandlerFactoryCreateFunc msg_handler_factory_create_func)
        : idx_(idx), scheduler_(scheduler), config_(config),
          data_flow_(data_flow), hsha_server_stat_(hsha_server_stat),
          hsha_server_qos_(hsha_server_qos), worker_pool_(worker_pool),
          msg_handler_factory_(move(msg_handler_factory_create_func())) {
}

HshaServerIO::~HshaServerIO() {
}

bool HshaServerIO::AddAcceptedFd(const int accepted_fd) {
    lock_guard<mutex> lock(queue_mutex_);
    if (accepted_fd_list_.size() > MAX_ACCEPT_QUEUE_LENGTH) {
        return false;
    }
    accepted_fd_list_.push(accepted_fd);
    if (static_cast<int>(hsha_server_stat_->io_read_request_qps_) < 5000 &&
        static_cast<int>(hsha_server_stat_->accept_qps_) < 5000) {
        scheduler_->NotifyEpoll();
    }
    return true;
}

void HshaServerIO::HandlerAcceptedFd() {
    lock_guard<mutex> lock(queue_mutex_);
    while (!accepted_fd_list_.empty()) {
        int accepted_fd{accepted_fd_list_.front()};
        accepted_fd_list_.pop();
        scheduler_->AddTask(bind(&HshaServerIO::IOFunc, this, accepted_fd), nullptr);
    }
}

void HshaServerIO::IOFunc(int accepted_fd) {
    UThreadSocket_t *socket{scheduler_->CreateSocket(accepted_fd)};
    UThreadTcpStream stream;
    stream.Attach(socket);
    UThreadSetSocketTimeout(*socket, config_->GetSocketTimeoutMS());

    while (true) {
        HshaServerStat::TimeCost time_cost;

        hsha_server_stat_->io_read_requests_++;

        auto msg_handler(msg_handler_factory_->Create());
        if (!msg_handler) {
            log(LOG_ERR, "%s Create err, client closed or no msg handler accept", __func__);

            break;
        }

        // will be deleted by worker
        BaseRequest *req{nullptr};
        int ret{msg_handler->RecvRequest(stream, req)};
        if (0 != ret) {
            if (req) {
                delete req;
                req = nullptr;
            }
            hsha_server_stat_->io_read_fails_++;
            hsha_server_stat_->rpc_time_costs_count_++;
            hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();
            log(LOG_ERR, "%s read request fail fd %d", __func__, accepted_fd);

            break;
        }
        char client_ip[128]{'\0'};
        stream.GetRemoteHost(client_ip, sizeof(client_ip));
        phxrpc::log(LOG_DEBUG, "%s RecvRequest ret %d client_ip %s", __func__,
                    static_cast<int>(ret), client_ip);

        hsha_server_stat_->io_read_bytes_ += req->size();

        if (!data_flow_->CanPushRequest(config_->GetMaxQueueLength())) {
            if (req) {
                delete req;
                req = nullptr;
            }
            hsha_server_stat_->queue_full_rejected_after_accepted_fds_++;
            phxrpc::log(LOG_ERR, "%s overflow can't enqueue fd %d", __func__, accepted_fd);

            break;
        }

        if (!hsha_server_qos_->CanEnqueue()) {
            // fast reject don't cal rpc_time_cost;
            if (req) {
                delete req;
                req = nullptr;
            }
            hsha_server_stat_->enqueue_fast_rejects_++;
            log(LOG_ERR, "%s fast reject can't enqueue fd %d", __func__, accepted_fd);

            break;
        }

        // if have enqueue, request will be deleted after pop.
        hsha_server_stat_->inqueue_push_requests_++;
        data_flow_->PushRequest(socket, req);
        // if is uthread worker mode, need notify.
        // req deleted by worker after this line
        worker_pool_->NotifyEpoll();
        UThreadSetArgs(*socket, nullptr);

        UThreadWait(*socket, config_->GetSocketTimeoutMS());
        if (UThreadGetArgs(*socket) == nullptr) {
            // timeout
            hsha_server_stat_->worker_timeouts_++;
            hsha_server_stat_->rpc_time_costs_count_++;
            hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();

            // because have enqueue, so socket will be closed after pop.
            socket = stream.DetachSocket();
            UThreadLazyDestory(*socket);

            log(LOG_ERR, "%s timeout, fd %d socket_timeout_ms %d",
                        __func__, accepted_fd, config_->GetSocketTimeoutMS());
            break;
        }

        hsha_server_stat_->io_write_responses_++;
        {
            BaseResponse *resp{(BaseResponse *)UThreadGetArgs(*socket)};
            if (!resp->fake()) {
                ret = resp->Send(stream);
                if (0 != ret) {
                    log(LOG_ERR, "%s Send err %d fd %d", __func__,
                        static_cast<int>(ret), accepted_fd);
                } else {
                    phxrpc::log(LOG_DEBUG, "%s Send ret %d idx %d", __func__,
                                static_cast<int>(ret), idx_);
                }
                hsha_server_stat_->io_write_bytes_ += resp->size();
            }
            delete resp;
        }

        hsha_server_stat_->rpc_time_costs_count_++;
        hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();

        if (0 != ret) {
            hsha_server_stat_->io_write_fails_++;
        }

        if (!msg_handler->keep_alive() || (0 != ret)) {
            break;
        }
    }

    hsha_server_stat_->hold_fds_--;
}

UThreadSocket_t *HshaServerIO::ActiveSocketFunc() {
    while (data_flow_->CanPluckResponse()) {
        void *args{nullptr};
        BaseResponse *resp{nullptr};
        int queue_wait_time_ms{data_flow_->PluckResponse(args, resp)};
        if (!resp) {
            // break out
            return nullptr;
        }
        hsha_server_stat_->outqueue_wait_time_costs_ += queue_wait_time_ms;
        hsha_server_stat_->outqueue_wait_time_costs_count_++;

        UThreadSocket_t *socket{(UThreadSocket_t *)args};
        if (socket != nullptr && IsUThreadDestory(*socket)) {
            // socket aready timeout
            //log(LOG_ERR, "%s socket aready timeout", __func__);
            UThreadClose(*socket);
            free(socket);
            delete resp;

            continue;
        }

        UThreadSetArgs(*socket, (void *)resp);

        return socket;
    }

    return nullptr;
}

void HshaServerIO::RunForever() {
    scheduler_->SetHandlerAcceptedFdFunc(bind(&HshaServerIO::HandlerAcceptedFd, this));
    scheduler_->SetActiveSocketFunc(bind(&HshaServerIO::ActiveSocketFunc, this));
    scheduler_->RunForever();
}


HshaServerUnit::HshaServerUnit(const int idx, HshaServer *const hsha_server,
        int worker_thread_count, int worker_uthread_count_per_thread,
        int worker_uthread_stack_size, Dispatch_t dispatch, void *args)
        : hsha_server_(hsha_server),
#ifndef __APPLE__
          scheduler_(8 * 1024, 1000000, false),
#else
          scheduler_(32 * 1024, 1000000, false),
#endif
          worker_pool_(idx, &scheduler_, hsha_server_->config_,
                       worker_thread_count, worker_uthread_count_per_thread,
                       worker_uthread_stack_size, &data_flow_,
                       &hsha_server_->hsha_server_stat_, dispatch, args),
          hsha_server_io_(idx, &scheduler_, hsha_server_->config_,
                          &data_flow_, &hsha_server_->hsha_server_stat_,
                          &hsha_server_->hsha_server_qos_, &worker_pool_,
                          hsha_server_->msg_handler_factory_create_func_),
          thread_(&HshaServerUnit::RunFunc, this) {
}

HshaServerUnit::~HshaServerUnit() {
    thread_.join();
}

void HshaServerUnit::RunFunc() {
    hsha_server_io_.RunForever();
}

bool HshaServerUnit::AddAcceptedFd(const int accepted_fd) {
    return hsha_server_io_.AddAcceptedFd(accepted_fd);
}


HshaServerAcceptor::HshaServerAcceptor(HshaServer *hsha_server)
        : hsha_server_(hsha_server) {
}

HshaServerAcceptor::~HshaServerAcceptor() {
}

void HshaServerAcceptor::LoopAccept(const char *const bind_ip, const int port) {
    int listen_fd{-1};
    if (!BlockTcpUtils::Listen(&listen_fd, bind_ip, port)) {
        printf("listen %s:%d err\n", bind_ip, port);
        exit(-1);
    }

    printf("listen %s:%d ok\n", bind_ip, port);

#ifndef __APPLE__
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    pid_t thread_id = 0;
    int ret{sched_setaffinity(thread_id, sizeof(mask), &mask)};
    if (0 != ret) {
        printf("sched_setaffinity err\n");
    }
#endif

    while (true) {
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(addr);
        int accepted_fd{accept(listen_fd, (struct sockaddr *) &addr, &socklen)};
        if (accepted_fd >= 0) {
            if (!hsha_server_->hsha_server_qos_.CanAccept()) {
                hsha_server_->hsha_server_stat_.rejected_fds_++;
                log(LOG_ERR, "%s too many connection, reject accept, fd %d", __func__, accepted_fd);
                close(accepted_fd);
                continue;
            }

            idx_ %= hsha_server_->server_unit_list_.size();
            if (!hsha_server_->server_unit_list_[idx_++]->AddAcceptedFd(accepted_fd)) {
                hsha_server_->hsha_server_stat_.rejected_fds_++;
                log(LOG_ERR, "%s accept queue full, reject accept, fd %d", __func__, accepted_fd);
                close(accepted_fd);
                continue;
            }

            hsha_server_->hsha_server_stat_.accepted_fds_++;
            hsha_server_->hsha_server_stat_.hold_fds_++;
        } else {
            hsha_server_->hsha_server_stat_.accept_fail_++;
        }
    }

    close(listen_fd);
}


HshaServer::HshaServer(const HshaServerConfig &config, const Dispatch_t &dispatch, void *args,
                       phxrpc::BaseMessageHandlerFactoryCreateFunc msg_handler_factory_create_func)
        : config_(&config), msg_handler_factory_create_func_(msg_handler_factory_create_func),
          hsha_server_monitor_(MonitorFactory::GetFactory()->
                               CreateServerMonitor(config.GetPackageName())),
          hsha_server_stat_(&config, hsha_server_monitor_),
          hsha_server_qos_(&config, &hsha_server_stat_),
          hsha_server_acceptor_(this) {
    size_t io_count{(size_t)config.GetIOThreadCount()};
    size_t worker_thread_count{(size_t)config.GetMaxThreads()};
    assert(worker_thread_count > 0);
    if (worker_thread_count < io_count) {
        io_count = worker_thread_count;
    }

    int worker_uthread_stack_size{config.GetWorkerUThreadStackSize()};
    size_t worker_thread_count_per_io{worker_thread_count / io_count};
    for (size_t i{0}; i < io_count; ++i) {
        if (i == io_count - 1) {
            worker_thread_count_per_io = worker_thread_count -
                    (worker_thread_count_per_io * (io_count - 1));
        }
        auto hsha_server_unit =
            new HshaServerUnit(i, this, (int)worker_thread_count_per_io,
                    config.GetWorkerUThreadCount(), worker_uthread_stack_size,
                    dispatch, args);
        assert(hsha_server_unit != nullptr);
        server_unit_list_.push_back(hsha_server_unit);
    }
    printf("server already started, %zu io threads %zu workers\n", io_count, worker_thread_count);
    if (config.GetWorkerUThreadCount() > 0) {
        printf("server in uthread mode, %d uthread per worker\n", config.GetWorkerUThreadCount());
    }
}

HshaServer::~HshaServer() {
    for (auto &hsha_server_unit : server_unit_list_) {
        delete hsha_server_unit;
    }
}

void HshaServer::RunForever() {
    hsha_server_acceptor_.LoopAccept(config_->GetBindIP(), config_->GetPort());
}


}  //namespace phxrpc

