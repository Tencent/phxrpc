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

#include "fa_server.h"

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


//void MqttSession::Heartbeat() {
//    if (0 >= session_attribute.keep_alive) {
//        expire_time_ms_ = -1;
//    } else {
//        expire_time_ms_ = session_attribute.keep_alive * 1000 + Timer::GetSteadyClockMS();
//    }
//}
//
//bool MqttSession::IsExpired() {
//    return expire_time_ms_ <= Timer::GetSteadyClockMS();
//}


MqttSession *SessionManager::Create(const int fd, UThreadEpollScheduler *const scheduler,
                                    const int socket_timeout_ms) {
    MqttSession session;
    session.session_id = (Timer::GetTimestampMS() << 32) | (++s_session_num);
    session.socket = scheduler->CreateSocket(fd);
    UThreadSetSocketTimeout(*(session.socket), socket_timeout_ms);
    session.stream.reset(new UThreadTcpStream);
    session.stream->Attach(session.socket);

    sessions_.emplace_back(move(session));

    return &(sessions_.back());
}

//MqttSession *SessionManager::GetByClientId(const string &client_id) {
//    for (auto &&session : sessions_) {
//        if (session.session_attribute.client_identifier == client_id)
//            return &session;
//    }
//
//    return nullptr;
//}

MqttSession *SessionManager::GetBySessionId(const uint64_t session_id) {
    for (auto &&session : sessions_) {
        if (session.session_id == session_id)
            return &session;
    }

    return nullptr;
}

MqttSession *SessionManager::GetByFd(const int fd) {
    for (auto &&session : sessions_) {
        if (UThreadSocketFd(*(session.socket)) == fd)
            return &session;
    }

    return nullptr;
}

void SessionManager::DeleteBySessionId(const uint64_t session_id) {
    for (auto it(sessions_.begin()); sessions_.end() != it; ++it) {
        if (it->session_id == session_id) {
            sessions_.erase(it);

            return;
        }
    }
}

atomic_uint32_t SessionManager::s_session_num{0};


void SessionRouter::Add(const uint64_t session_id, const int idx) {
    lock_guard<mutex> lock(mutex_);
    session_id2thread_index_map_[session_id] = idx;
}

int SessionRouter::Get(const uint64_t session_id) const {
    lock_guard<mutex> lock(mutex_);
    const auto &it(session_id2thread_index_map_.find(session_id));
    if (session_id2thread_index_map_.end() == it) {
        return -1;
    }

    return it->second;
}

void SessionRouter::Delete(const uint64_t session_id) {
    lock_guard<mutex> lock(mutex_);
    session_id2thread_index_map_.erase(session_id);
}


FaServerIO::FaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
                       const HshaServerConfig *config, DataFlow *data_flow,
                       HshaServerStat *hsha_server_stat, HshaServerQos *hsha_server_qos,
                       WorkerPool *worker_pool, SessionManager *session_mgr,
                       SessionRouter *session_router)
        : idx_(idx), scheduler_(scheduler), config_(config), data_flow_(data_flow),
          hsha_server_stat_(hsha_server_stat),
          hsha_server_qos_(hsha_server_qos), worker_pool_(worker_pool),
          session_mgr_(session_mgr), session_router_(session_router) {
}

FaServerIO::~FaServerIO() {
}

bool FaServerIO::AddAcceptedFd(int accepted_fd) {
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

void FaServerIO::HandlerAcceptedFd() {
    lock_guard<mutex> lock(queue_mutex_);
    while (!accepted_fd_list_.empty()) {
        int accepted_fd = accepted_fd_list_.front();
        accepted_fd_list_.pop();
        scheduler_->AddTask(bind(&FaServerIO::UThreadIFunc, this, accepted_fd), nullptr);
    }
}

void FaServerIO::UThreadIFunc(int accepted_fd) {
    MqttSession *session{session_mgr_->Create(accepted_fd, scheduler_, config_->GetSocketTimeoutMS())};
    if (!session) {
        phxrpc::log(LOG_ERR, "invalid fd %d", accepted_fd);

        return;
    }

    unique_ptr<BaseProtocolFactory> factory(
            BaseProtocolFactory::CreateFactory(*(session->stream)));
    unique_ptr<BaseProtocol> protocol(factory->GenProtocol());

    while (true) {
        HshaServerStat::TimeCost time_cost;

        hsha_server_stat_->io_read_requests_++;

        // will be deleted by worker
        BaseRequest *req{nullptr};
        ReturnCode ret{protocol->ServerRecv(*(session->stream), req)};
        // TODO: remove
        printf("session_id %" PRIx64 " ServerRecv ret %d idx %d fd %d\n",
               session->session_id, ret, idx_, accepted_fd);
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
        //const bool is_keep_alive{0 != req->IsKeepAlive()};
        //const string version(req->GetVersion() != nullptr ? req->GetVersion() : "");

        hsha_server_stat_->inqueue_push_requests_++;
        ServiceContext *context{new ServiceContext};
        context->session_id = session->session_id;
        data_flow_->PushRequest((void *)context, req);
        // if is uthread worker mode, need notify.
        // req deleted by worker after this line
        worker_pool_->NotifyEpoll();
        //UThreadSetArgs(*socket, nullptr);

        //UThreadWait(*socket, config_->GetSocketTimeoutMS());
        //if (UThreadGetArgs(*socket) == nullptr) {
        //    // timeout
        //    hsha_server_stat_->worker_timeouts_++;
        //    hsha_server_stat_->rpc_time_costs_count_++;
        //    hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();

        //    // because have enqueue, so socket will be closed after pop.
        //    socket = stream.DetachSocket();
        //    UThreadLazyDestory(*socket);

        //    phxrpc::log(LOG_ERR, "%s timeout, fd %d sockettimeoutms %d",
        //                __func__, accepted_fd, config_->GetSocketTimeoutMS());
        //    break;
        //}

        //hsha_server_stat_->io_write_responses_++;
        //{
        //    BaseResponse *resp((BaseResponse *)UThreadGetArgs(*socket));
        //    if (!resp->fake()) {
        //        ret = resp->ModifyResp(is_keep_alive, version);
        //        ret = resp->Send(stream);
        //        hsha_server_stat_->io_write_bytes_ += resp->GetContent().size();
        //    }
        //    delete resp;
        //}

        //hsha_server_stat_->rpc_time_costs_count_++;
        //hsha_server_stat_->rpc_time_costs_ += time_cost.Cost();

        //if (ReturnCode::OK != ret) {
        //    hsha_server_stat_->io_write_fails_++;
        //}

        //if (!is_keep_alive || (ReturnCode::OK != ret)) {
        //    break;
        //}
    }

    hsha_server_stat_->hold_fds_--;
}

//UThreadSocket_t *FaServerIO::ActiveSocketFunc() {
//    while (data_flow_->CanPluckResponse()) {
//        void *args{nullptr};
//        BaseResponse *resp{nullptr};
//        int queue_wait_time_ms{data_flow_->PluckResponse(args, resp)};
//        if (!resp) {
//            //break out
//            return nullptr;
//        }
//        hsha_server_stat_->outqueue_wait_time_costs_ += queue_wait_time_ms;
//        hsha_server_stat_->outqueue_wait_time_costs_count_++;
//
//        UThreadSocket_t *socket = (UThreadSocket_t *)args;
//        if (socket != nullptr && IsUThreadDestory(*socket)) {
//            //socket aready timeout.
//            //phxrpc::log(LOG_ERR, "%s socket aready timeout", __func__);
//            UThreadClose(*socket);
//            free(socket);
//            delete resp;
//            continue;
//        }
//
//        UThreadSetArgs(*socket, (void *)resp);
//        return socket;
//    }
//
//    return nullptr;
//}

void FaServerIO::HandlerNewResponseFunc() {
    while (data_flow_->CanPluckResponse()) {
        void *args{nullptr};
        BaseResponse *resp{nullptr};
        int queue_wait_time_ms{data_flow_->PluckResponse(args, resp)};
        if (!resp) {
            return;
        }
        hsha_server_stat_->outqueue_wait_time_costs_ += queue_wait_time_ms;
        hsha_server_stat_->outqueue_wait_time_costs_count_++;

        scheduler_->AddTask(bind(&FaServerIO::UThreadOFunc, this, args,
                                 resp, queue_wait_time_ms), nullptr);
    }
}

void FaServerIO::UThreadOFunc(void *args, BaseResponse *resp, int queue_wait_time_ms) {
    ServiceContext *context{(ServiceContext *)args};
    if (!context) {
        phxrpc::log(LOG_ERR, "context nullptr");
        delete resp;

        return;
    }

    // 1. update session
    const auto &session(session_mgr_->GetBySessionId(context->session_id));
    if (!session) {
        phxrpc::log(LOG_ERR, "invalid session_id %" PRIx64, context->session_id);
        delete context;
        delete resp;

        return;
    }
    if (context->destroy_session) {
        // mqtt disconnect
        session_router_->Delete(session->session_id);
        session_mgr_->DeleteBySessionId(session->session_id);

        delete context;
        delete resp;

        return;
    }

    if (context->init_session) {
        //// mqtt connect: if client_id exist, close old session
        //const auto &old_session(session_mgr_->GetByClientId(
        //        context->session_attribute.client_identifier));
        //if (old_session) {
        //    if (old_session->session_id == session->session_id) {
        //        // mqtt-3.1.0-2: disconnect current connection

        //        session_router_->Delete(session->session_id);
        //        session_mgr_->DeleteBySessionId(session->session_id);

        //        delete context;
        //        delete resp;

        //        return;
        //    } else {
        //        // mqtt-3.1.4-2: disconnect other connection with same client_id
        //        session_router_->Delete(old_session->session_id);
        //        session_mgr_->DeleteBySessionId(old_session->session_id);
        //    }
        //}

        //// mqtt connect: set client_id and init
        //session->session_attribute = context->session_attribute;
        //session->Heartbeat();
        session_router_->Add(session->session_id, idx_);
    }

    if (context->heartbeat_session) {
        // mqtt ping
        //session->Heartbeat();
    }

    // 2. send response
    if (!resp->fake()) {
        ReturnCode ret{resp->Send(*(session->stream))};
        // TODO: remove
        printf("session_id %" PRIx64 " Send ret %d idx %d\n", context->session_id, ret, idx_);
        //printf("session_id %" PRIx64 " Send client_id \"%s\" ret %d idx %d\n",
        //       context->session_id, session->session_attribute.client_identifier.c_str(), ret, idx_);
        hsha_server_stat_->io_write_bytes_ += resp->GetContent().size();
    }
    delete context;
    delete resp;
}

void FaServerIO::RunForever() {
    scheduler_->SetHandlerAcceptedFdFunc(bind(&FaServerIO::HandlerAcceptedFd, this));
    // TODO: don't hack HandlerNewRequestFunc
    scheduler_->SetHandlerNewRequestFunc(bind(&FaServerIO::HandlerNewResponseFunc, this));
    //scheduler_->SetActiveSocketFunc(bind(&FaServerIO::ActiveSocketFunc, this));
    scheduler_->RunForever();
}


FaServerUnit::FaServerUnit(
        const int idx,
        Server *const root_server,
        FaServer *const fa_server,
        int worker_thread_count,
        int worker_uthread_count_per_thread,
        int worker_uthread_stack_size,
        NotifierPoolRouter *const notifier_pool_router,
        Dispatch_t dispatch,
        void *args, SessionRouter *session_router)
        : root_server_(root_server),
          fa_server_(fa_server),
#ifndef __APPLE__
          scheduler_(8 * 1024, 1000000, false),
#else
          scheduler_(32 * 1024, 1000000, false),
#endif
          worker_pool_(idx, &scheduler_, fa_server_->config_,
                       worker_thread_count, worker_uthread_count_per_thread,
                       worker_uthread_stack_size, root_server, this, notifier_pool_router,
                       &data_flow_, &fa_server_->hsha_server_stat_, dispatch, args),
          fa_server_io_(idx, &scheduler_, fa_server_->config_, &data_flow_,
                        &fa_server_->hsha_server_stat_, &fa_server_->hsha_server_qos_,
                        &worker_pool_, &session_mgr_, session_router),
          thread_(&FaServerUnit::RunFunc, this) {
}

FaServerUnit::~FaServerUnit() {
    thread_.join();
}

void FaServerUnit::RunFunc() {
    fa_server_io_.RunForever();
}

bool FaServerUnit::AddAcceptedFd(int accepted_fd) {
    return fa_server_io_.AddAcceptedFd(accepted_fd);
}

void FaServerUnit::PushResponse(void *args, BaseResponse *const resp) {
    data_flow_.PushResponse(args, resp);
    //fa_server_io_.hsha_server_stat_.outqueue_push_responses_++;
}


FaServerAcceptor::FaServerAcceptor(FaServer *fa_server)
        : fa_server_(fa_server) {
}

FaServerAcceptor::~FaServerAcceptor() {
}

void FaServerAcceptor::LoopAccept(const char *bind_ip, const int port) {
    int listen_fd{-1};
    if (!BlockTcpUtils::Listen(&listen_fd, bind_ip, port)) {
        printf("listen fail, ip %s mqtt_port %d\n", bind_ip, port);
        exit(-1);
    }

    printf("listen succ, ip %s mqtt_port %d\n", bind_ip, port);

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
            if (!fa_server_->hsha_server_qos_.CanAccept()) {
                fa_server_->hsha_server_stat_.rejected_fds_++;
                phxrpc::log(LOG_ERR, "%s too many connection, reject accept, fd %d", __func__, accepted_fd);
                close(accepted_fd);
                continue;
            }

            idx_ %= fa_server_->fa_server_unit_list_.size();
            if (!fa_server_->fa_server_unit_list_[idx_++]->AddAcceptedFd(accepted_fd)) {
                fa_server_->hsha_server_stat_.rejected_fds_++;
                phxrpc::log(LOG_ERR, "%s accept queue full, reject accept, fd %d", __func__, accepted_fd);
                close(accepted_fd);
                continue;
            }

            fa_server_->hsha_server_stat_.accepted_fds_++;
            fa_server_->hsha_server_stat_.hold_fds_++;
        } else {
            fa_server_->hsha_server_stat_.accept_fail_++;
        }
    }

    close(listen_fd);
}


FaServer::FaServer(const HshaServerConfig &config, const Dispatch_t &dispatch, void *args,
                   Server *const root_server)
        : config_(&config),
          hsha_server_monitor_(MonitorFactory::GetFactory()->
                               CreateServerMonitor(config.GetPackageName())),
          hsha_server_stat_(&config, hsha_server_monitor_),
          hsha_server_qos_(&config, &hsha_server_stat_),
          fa_server_acceptor_(this), root_server_(root_server) {
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
        auto fa_hsha_server_unit =
            new FaServerUnit(i, root_server, this, (int)worker_thread_count_per_io,
                    config.GetWorkerUThreadCount(), worker_uthread_stack_size,
                    &notifier_pool_router_, dispatch, args, &session_router_);
        assert(fa_hsha_server_unit != nullptr);
        fa_server_unit_list_.push_back(fa_hsha_server_unit);
    }
    printf("server already started, %zu io threads %zu workers\n", io_count, worker_thread_count);
    if (config.GetWorkerUThreadCount() > 0) {
        printf("server in uthread mode, %d uthread per worker\n", config.GetWorkerUThreadCount());
    }
}

FaServer::~FaServer() {
    for (auto &fa_server_unit : fa_server_unit_list_) {
        delete fa_server_unit;
    }

    fa_accept_thread_.join();
}

void FaServer::DoRunForever() {
    fa_accept_thread_ = thread(&FaServerAcceptor::LoopAccept, fa_server_acceptor_,
                               config_->GetBindIP(), config_->GetMqttPort());
}

void FaServer::PushResponse(const uint64_t session_id, BaseResponse *resp) {
    // push to server unit outqueue
    int server_unit_idx{session_router_.Get(session_id)};
    ServiceContext *context{new ServiceContext};
    context->session_id = session_id;
    // forward req and do not delete here
    fa_server_unit_list_[server_unit_idx]->PushResponse(context, (BaseResponse *)resp);
}


}  //namespace phxrpc

