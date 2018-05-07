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


struct ResidentMessage final {
    std::string topic_name;
    std::string content;
    uint32_t qos{0u};
};


class MqttSession final {
  public:
    //enum class Status {
    //    None = 0,
    //    Connected,
    //    Closed,
    //};

    //void Heartbeat();
    //bool IsExpired();

    uint64_t session_id{0uLL};
    // not use unique_ptr because socket is own by stream
    UThreadSocket_t *socket{nullptr};
    std::unique_ptr<UThreadTcpStream> stream;
    //Status status{Status::None};
    //SessionAttribute session_attribute;
    std::vector<ResidentMessage> resident_messages;

  private:
    uint64_t expire_time_ms_{0uLL};
};


class SessionManager final {
  public:
    MqttSession *Create(const int fd, UThreadEpollScheduler *const scheduler,
                        const int socket_timeout_ms);

    //MqttSession *GetByClientId(const std::string &client_id);

    MqttSession *GetBySessionId(const uint64_t session_id);

    MqttSession *GetByFd(const int fd);

    void DeleteBySessionId(const uint64_t session_id);

  private:
    static std::atomic_uint32_t s_session_num;

    std::list<MqttSession> sessions_;
};


class SessionRouter final {
  public:
    void Add(const uint64_t session_id, const int idx);
    int Get(const uint64_t session_id) const;
    void Delete(const uint64_t session_id);

  private:
    mutable std::mutex mutex_;
    std::map<uint64_t, int> session_id2thread_index_map_;
};


class FaServerIO final {
  public:
    FaServerIO(const int idx, UThreadEpollScheduler *const scheduler,
               const HshaServerConfig *config, DataFlow *data_flow,
               HshaServerStat *hsha_server_stat, HshaServerQos *hsha_server_qos,
               WorkerPool *worker_pool, SessionManager *session_mgr,
               SessionRouter *session_router);
    ~FaServerIO();

    void RunForever();

    bool AddAcceptedFd(int accepted_fd);

    void HandlerAcceptedFd();

    void UThreadIFunc(int accept_fd);

    //UThreadSocket_t *ActiveSocketFunc();

    void HandlerNewResponseFunc();

    void UThreadOFunc(void *args, BaseResponse *resp, int queue_wait_time_ms);

  private:
    int idx_{-1};
    UThreadEpollScheduler *scheduler_{nullptr};
    const HshaServerConfig *config_{nullptr};
    DataFlow *data_flow_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    HshaServerQos *hsha_server_qos_{nullptr};
    WorkerPool *worker_pool_{nullptr};
    SessionManager *session_mgr_{nullptr};
    SessionRouter *session_router_{nullptr};

    std::queue<int> accepted_fd_list_;
    std::mutex queue_mutex_;
};


class FaServer;

class FaServerUnit : public BaseServerUnit {
  public:
    FaServerUnit(const int idx,
            Server *const root_server,
            FaServer *const fa_server,
            int worker_thread_count,
            int worker_uthread_count_per_thread,
            int worker_uthread_stack_size,
            NotifierPoolRouter *const notifier_pool_router,
            Dispatch_t dispatch,
            void *args, SessionRouter *session_router);
    virtual ~FaServerUnit() override;

    void RunFunc();
    bool AddAcceptedFd(int accepted_fd);
    void PushResponse(void *args, BaseResponse *const resp);

  private:
    Server *root_server_{nullptr};
    FaServer *fa_server_{nullptr};
    UThreadEpollScheduler scheduler_;
    WorkerPool worker_pool_;
    FaServerIO fa_server_io_;
    SessionManager session_mgr_;
    std::thread thread_;
};


class FaServerAcceptor final {
  public:
    FaServerAcceptor(FaServer *fa_server);
    ~FaServerAcceptor();

    void LoopAccept(const char *bind_ip, const int port);

  private:
    FaServer *fa_server_{nullptr};
    size_t idx_{0};
};


class FaServer : public BaseServer {
  public:
    FaServer(const HshaServerConfig &config, const Dispatch_t &dispatch, void *args,
             Server *const root_server);
    virtual ~FaServer() override;

    virtual void DoRunForever() override;

    void PushResponse(const uint64_t session_id, BaseResponse *const resp);

    Server *root_server() const { return root_server_; }

    const HshaServerConfig *config_{nullptr};
    ServerMonitorPtr hsha_server_monitor_;
    HshaServerStat hsha_server_stat_;
    HshaServerQos hsha_server_qos_;
    FaServerAcceptor fa_server_acceptor_;

    std::vector<FaServerUnit *> fa_server_unit_list_;

    std::thread fa_accept_thread_;

    SessionRouter session_router_;

  private:
    Server *root_server_{nullptr};
    NotifierPoolRouter notifier_pool_router_;
};


}  //namespace phxrpc

