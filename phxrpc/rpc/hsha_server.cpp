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


HshaServerIO::HshaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
                           const HshaServerConfig *config,
                           DataFlow *data_flow, HshaServerStat *hsha_server_stat,
                           HshaServerQos *hsha_server_qos, WorkerPool *worker_pool)
        : idx_(idx), scheduler_(scheduler), config_(config), data_flow_(data_flow),
          hsha_server_stat_(hsha_server_stat),
          hsha_server_qos_(hsha_server_qos), worker_pool_(worker_pool) {
}

HshaServerIO::~HshaServerIO() {
}

bool HshaServerIO::AddAcceptedFd(int accepted_fd) {
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
        int accepted_fd = accepted_fd_list_.front();
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

        unique_ptr<BaseProtocolFactory> factory(
                BaseProtocolFactory::CreateFactory(stream));
        unique_ptr<BaseProtocol> protocol(factory->GenProtocol());
        // will be deleted by worker
        BaseRequest *req{nullptr};
        ReturnCode ret{protocol->ServerRecv(stream, req)};
        if (ReturnCode::OK != ret) {
            delete req;
            hsha_server_stat_->io_read_fails_++;
            hsha_server_stat_->rpc_time_costs_count_++;
            hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();
            phxrpc::log(LOG_ERR, "%s read request fail, fd %d", __func__, accepted_fd);

            break;
        }

        hsha_server_stat_->io_read_bytes_ += req->GetContent().size();

        if (!data_flow_->CanPushRequest(config_->GetMaxQueueLength())) {
            delete req;
            hsha_server_stat_->queue_full_rejected_after_accepted_fds_++;

            break;
        }

        if (!hsha_server_qos_->CanEnqueue()) {
            // fast reject don't cal rpc_time_cost;
            delete req;
            hsha_server_stat_->enqueue_fast_rejects_++;
            phxrpc::log(LOG_ERR, "%s fast reject, can't enqueue, fd %d",
                        __func__, accepted_fd);

            break;
        }

        // if have enqueue, request will be deleted after pop.
        const bool is_keep_alive{0 != req->IsKeepAlive()};
        const string version(req->GetVersion() != nullptr ? req->GetVersion() : "");

        hsha_server_stat_->inqueue_push_requests_++;
        data_flow_->PushRequest((void *)socket, req);
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

            phxrpc::log(LOG_ERR, "%s timeout, fd %d socket_timeout_ms %d",
                        __func__, accepted_fd, config_->GetSocketTimeoutMS());
            break;
        }

        hsha_server_stat_->io_write_responses_++;
        {
            BaseResponse *resp((BaseResponse *)UThreadGetArgs(*socket));
            if (!resp->fake()) {
                ret = resp->ModifyResp(is_keep_alive, version);
                ret = resp->Send(stream);
                hsha_server_stat_->io_write_bytes_ += resp->GetContent().size();
            }
            delete resp;
        }

        hsha_server_stat_->rpc_time_costs_count_++;
        hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();

        if (ReturnCode::OK != ret) {
            hsha_server_stat_->io_write_fails_++;
        }

        if (!is_keep_alive || (ReturnCode::OK != ret)) {
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
            //break out
            return nullptr;
        }
        hsha_server_stat_->outqueue_wait_time_costs_ += queue_wait_time_ms;
        hsha_server_stat_->outqueue_wait_time_costs_count_++;

        UThreadSocket_t *socket = (UThreadSocket_t *)args;
        if (socket != nullptr && IsUThreadDestory(*socket)) {
            //socket aready timeout.
            //phxrpc::log(LOG_ERR, "%s socket aready timeout", __func__);
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


HshaServerUnit::HshaServerUnit(
        const int idx,
        Server *const root_server,
        HshaServer *const hsha_server,
        int worker_thread_count,
        int worker_uthread_count_per_thread,
        int worker_uthread_stack_size,
        NotifierPoolRouter *const notifier_pool_router,
        Dispatch_t dispatch,
        void *args)
        : root_server_(root_server),
          hsha_server_(hsha_server),
#ifndef __APPLE__
          scheduler_(8 * 1024, 1000000, false),
#else
          scheduler_(32 * 1024, 1000000, false),
#endif
          worker_pool_(idx, &scheduler_, hsha_server_->config_,
                       worker_thread_count, worker_uthread_count_per_thread,
                       worker_uthread_stack_size, root_server, this, notifier_pool_router,
                       &data_flow_, &hsha_server_->hsha_server_stat_, dispatch, args),
          hsha_server_io_(idx, &scheduler_, hsha_server_->config_, &data_flow_,
                          &hsha_server_->hsha_server_stat_,
                          &hsha_server_->hsha_server_qos_, &worker_pool_),
          thread_(&HshaServerUnit::RunFunc, this) {
}

HshaServerUnit::~HshaServerUnit() {
    thread_.join();
}

void HshaServerUnit::RunFunc() {
    hsha_server_io_.RunForever();
}

bool HshaServerUnit::AddAcceptedFd(int accepted_fd) {
    return hsha_server_io_.AddAcceptedFd(accepted_fd);
}

int HshaServerUnit::NotifyTargetWorker(const int idx,
                                       const NotifierPoolRouter::NotifierId &notifier_id,
                                       void *const data) {
    return worker_pool_.NotifyTarget(idx, notifier_id, data);
}


HshaServerAcceptor::HshaServerAcceptor(HshaServer *hsha_server)
        : hsha_server_(hsha_server) {
}

HshaServerAcceptor::~HshaServerAcceptor() {
}

void HshaServerAcceptor::LoopAccept(const char *bind_ip, const int port) {
    int listen_fd{-1};
    if (!BlockTcpUtils::Listen(&listen_fd, bind_ip, port)) {
        printf("listen fail, ip %s port %d\n", bind_ip, port);
        exit(-1);
    }

    printf("listen succ, ip %s port %d\n", bind_ip, port);

#ifndef __APPLE__
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    pid_t thread_id = 0;
    int ret{sched_setaffinity(thread_id, sizeof(mask), &mask)};
    if (ret != 0) {
        printf("sched_setaffinity fail\n");
    }
#endif

    while (true) {
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(addr);
        int accepted_fd{accept(listen_fd, (struct sockaddr *) &addr, &socklen)};
        if (accepted_fd >= 0) {
            if (!hsha_server_->hsha_server_qos_.CanAccept()) {
                hsha_server_->hsha_server_stat_.rejected_fds_++;
                phxrpc::log(LOG_ERR, "%s too many connection, reject accept, fd %d", __func__, accepted_fd);
                close(accepted_fd);
                continue;
            }

            idx_ %= hsha_server_->server_unit_list_.size();
            if (!hsha_server_->server_unit_list_[idx_++]->AddAcceptedFd(accepted_fd)) {
                hsha_server_->hsha_server_stat_.rejected_fds_++;
                phxrpc::log(LOG_ERR, "%s accept queue full, reject accept, fd %d", __func__, accepted_fd);
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
                       Server *const root_server)
        : config_(&config),
          hsha_server_monitor_(MonitorFactory::GetFactory()->
                               CreateServerMonitor(config.GetPackageName())),
          hsha_server_stat_(&config, hsha_server_monitor_),
          hsha_server_qos_(&config, &hsha_server_stat_),
          hsha_server_acceptor_(this), root_server_(root_server) {
    size_t io_count = (size_t)config.GetIOThreadCount();
    size_t worker_thread_count = (size_t)config.GetMaxThreads();
    assert(worker_thread_count > 0);
    if (worker_thread_count < io_count) {
        io_count = worker_thread_count;
    }

    int worker_uthread_stack_size = config.GetWorkerUThreadStackSize();
    size_t worker_thread_count_per_io = worker_thread_count / io_count;
    for (size_t i{0}; i < io_count; ++i) {
        if (i == io_count - 1) {
            worker_thread_count_per_io = worker_thread_count - (worker_thread_count_per_io * (io_count - 1));
        }
        auto hsha_server_unit =
            new HshaServerUnit(i, root_server, this, (int)worker_thread_count_per_io,
                    config.GetWorkerUThreadCount(), worker_uthread_stack_size,
                    &notifier_pool_router_, dispatch, args);
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

    hsha_accept_thread_.join();
}

void HshaServer::DoRunForever() {
    hsha_accept_thread_ = thread(&HshaServerAcceptor::LoopAccept, hsha_server_acceptor_,
                                 config_->GetBindIP(), config_->GetPort());
}

int HshaServer::SendNotify(const NotifierPoolRouter::NotifierId &notifier_id, void *const data) {
    auto idx{notifier_pool_router_.Get(notifier_id)};
    return server_unit_list_[idx.first]->
            NotifyTargetWorker(idx.second, notifier_id, data);
}

int HshaServer::WaitNotify(UThreadNotifierPool *const notifier_pool, const int pool_idx, const int worker_idx,
                           const NotifierPoolRouter::NotifierId &notifier_id, void *&data) {
    notifier_pool_router_.Add(notifier_id, make_pair(pool_idx, worker_idx));
    // TODO: remove
    auto temp(notifier_pool_router_.Get(notifier_id));
    printf("%s notifier_pool_router idx %d:%d\n", __func__, temp.first, temp.second);
    UThreadNotifier *notifier{nullptr};
    int ret{notifier_pool->GetNotifier(move(notifier_id.ToUint128()), notifier)};
    if (0 != ret) {
        // TODO: remove
        printf("%s GetNotifier err %d\n", __func__, ret);
        notifier_pool_router_.Delete(notifier_id);

        return ret;
    }

    // yield and wait
    ret = notifier->WaitNotify(data);
    if (0 != ret) {
        // TODO: remove
        printf("%s WaitNotify err %d\n", __func__, ret);
        notifier_pool_router_.Delete(notifier_id);
        notifier_pool->ReleaseNotifier(move(notifier_id.ToUint128()));

        return ret;
    }

    notifier_pool_router_.Delete(notifier_id);
    notifier_pool->ReleaseNotifier(move(notifier_id.ToUint128()));

    return 0;
}


}  //namespace phxrpc

