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

    void Heartbeat();
    bool IsExpired();

    uint64_t session_id{0uLL};
    // not use unique_ptr because socket is own by stream
    UThreadSocket_t *socket{nullptr};
    std::unique_ptr<UThreadTcpStream> stream;
    //Status status{Status::None};
    SessionAttribute session_attribute;
    std::vector<ResidentMessage> resident_messages;

  private:
    uint64_t expire_time_ms_{0uLL};
};


class SessionManager final {
  public:
    MqttSession *Create(const int fd, UThreadEpollScheduler *const scheduler,
                        const int socket_timeout_ms);

    MqttSession *GetByClientId(const std::string &client_id);

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


//////////////////////////////////

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

    mutable std::mutex mutex_;
    ThdQueue<std::pair<QueueExtData, BaseRequest *>> in_queue_;
    ThdQueue<std::pair<QueueExtData, BaseResponse *>> out_queue_;
};


//class CrossUnitDataFlow final {
//  public:
//    CrossUnitDataFlow();
//    ~CrossUnitDataFlow();
//
//    void PushRequest(void *args, google::protobuf::Message *req);
//    int PluckRequest(void *&args, google::protobuf::Message *&req);
//    int PickRequest(void *&args, google::protobuf::Message *&req);
//    void PushResponse(void *args, google::protobuf::Message *resp);
//    int PluckResponse(void *&args, google::protobuf::Message *&resp);
//    int PickResponse(void *&args, google::protobuf::Message *&resp);
//    bool CanPushRequest(const int max_queue_length);
//    bool CanPluckRequest();
//    bool CanPluckResponse();
//
//    void BreakOut();
//
//  private:
//    struct QueueExtData {
//        QueueExtData() {
//            enqueue_time_ms = 0;
//            args = nullptr;
//        }
//        QueueExtData(void *t_args) {
//            enqueue_time_ms = Timer::GetSteadyClockMS();
//            args = t_args;
//        }
//        uint64_t enqueue_time_ms;
//        void *args;
//    };
//    ThdQueue<std::pair<QueueExtData, google::protobuf::Message *>> in_queue_;
//    ThdQueue<std::pair<QueueExtData, google::protobuf::Message *>> out_queue_;
//};


//////////////////////////////////

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


//////////////////////////////////

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

//////////////////////////////////

class Worker final {
  public:
    Worker(const int idx, WorkerPool *const pool,
           const int uthread_count, const int utherad_stack_size);
    ~Worker();

    void Func();
    void Shutdown();

    void ThreadMode();
    void UThreadMode();
    void HandlerNewRequestFunc();
    void UThreadFunc(void *args, BaseRequest *req, int queue_wait_time_ms);
    void WorkerLogic(void *args, BaseRequest *req, int queue_wait_time_ms);
    void NotifyEpoll();
    int NotifyTarget(const NotifierPoolRouter::NotifierId &notifier_id, BaseResponse *resp);

  private:
    int idx_{-1};
    WorkerPool *pool_{nullptr};
    int uthread_count_;
    int utherad_stack_size_;
    bool shut_down_{false};
    UThreadEpollScheduler *worker_scheduler_{nullptr};
    // TODO: support thread mode
    UThreadNotifierPool *notifier_pool_{nullptr};
    std::thread thread_;
};

/////////////////////////////////

typedef std::function<void(const BaseRequest *, BaseResponse *, DispatcherArgs_t *)> Dispatch_t;

class WorkerPool final {
  public:
    WorkerPool(const int idx, UThreadEpollScheduler *scheduler,
               const HshaServerConfig *config,
               const int thread_count,
               const int uthread_count_per_thread,
               const int utherad_stack_size,
               NotifierPoolRouter *const notifier_pool_router,
               DataFlow *const data_flow,
               DataFlow *const cross_unit_data_flow,
               HshaServerStat *const hsha_server_stat,
               Dispatch_t dispatch,
               void *args);
    ~WorkerPool();

    void NotifyEpoll();
    int NotifyTarget(const int idx, const NotifierPoolRouter::NotifierId &notifier_id, BaseResponse *resp);

  private:
    friend class Worker;
    int idx_{-1};
    UThreadEpollScheduler *scheduler_{nullptr};
    const HshaServerConfig *config_{nullptr};
    NotifierPoolRouter *notifier_pool_router_{nullptr};
    DataFlow *data_flow_{nullptr};
    DataFlow *cross_unit_data_flow_{nullptr};
    HshaServerStat *hsha_server_stat_{nullptr};
    Dispatch_t dispatch_;
    void *args_{nullptr};
    std::vector<Worker *> worker_list_;
    size_t last_notify_idx_;
    std::mutex mutex_;
};

/////////////////////////////////

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

/////////////////////////////////

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

/////////////////////////////////

class HshaServer;
class HshaServerUnit final {
  public:
    HshaServerUnit(HshaServer *hsha_server,
            const int idx,
            int worker_thread_count,
            int worker_uthread_count_per_thread,
            int worker_utherad_stack_size,
            NotifierPoolRouter *const notifier_pool_router,
            Dispatch_t dispatch,
            void *args);
    ~HshaServerUnit();

    void RunFunc();
    bool AddAcceptedFd(int accepted_fd);
    int NotifyTargetWorker(const int idx, const NotifierPoolRouter::NotifierId &notifier_id,
                           BaseResponse *resp);

  private:
    HshaServer *hsha_server_;
    UThreadEpollScheduler scheduler_;
    DataFlow data_flow_;
    WorkerPool worker_pool_;
    HshaServerIO hsha_server_io_;
    std::thread thread_;
};

/////////////////////////////////

class FaServerUnit final {
  public:
    FaServerUnit(HshaServer *hsha_server,
            const int idx,
            int worker_thread_count,
            int worker_uthread_count_per_thread,
            int worker_utherad_stack_size,
            NotifierPoolRouter *const notifier_pool_router,
            Dispatch_t dispatch,
            void *args, SessionRouter *session_router);
    ~FaServerUnit();

    void RunFunc();
    bool AddAcceptedFd(int accepted_fd);
    void PushResponse(void *args, BaseResponse *const resp);

  private:
    HshaServer *hsha_server_;
    UThreadEpollScheduler scheduler_;
    DataFlow data_flow_;
    WorkerPool worker_pool_;
    SessionManager session_mgr_;
    FaServerIO fa_server_io_;
    std::thread thread_;
};

/////////////////////////////////

class HshaServerAcceptor final {
  public:
    HshaServerAcceptor(HshaServer *hsha_server);
    ~HshaServerAcceptor();

    void LoopAccept(const char *bind_ip, const int port);

  private:
    HshaServer *hsha_server_{nullptr};
    size_t idx_{};
};

/////////////////////////////////

class FaServerAcceptor final {
  public:
    FaServerAcceptor(HshaServer *hsha_server);
    ~FaServerAcceptor();

    void LoopAccept(const char *bind_ip, const int port);

  private:
    HshaServer *hsha_server_{nullptr};
    size_t idx_{0};
};

/////////////////////////////////

class HshaServer final {
  public:
    HshaServer(const HshaServerConfig &config, Dispatch_t dispatch, void *args);
    ~HshaServer();

    void RunForever();

    const HshaServerConfig *config_{nullptr};
    ServerMonitorPtr hsha_server_monitor_;
    HshaServerStat hsha_server_stat_;
    HshaServerQos hsha_server_qos_;
    HshaServerAcceptor hsha_server_acceptor_;
    FaServerAcceptor fa_server_acceptor_;

    std::vector<HshaServerUnit *> server_unit_list_;
    std::vector<FaServerUnit *> fa_server_unit_list_;

    std::thread hsha_accept_thread_;
    std::thread fa_accept_thread_;
    std::thread cross_unit_req_thread_;
    std::thread cross_unit_resp_thread_;

    SessionRouter session_router_;
    DataFlow cross_unit_data_flow_;
    NotifierPoolRouter notifier_pool_router_;

  private:
    void LoopReadCrossUnitRequest();
    void LoopReadCrossUnitResponse();
};


}  //namespace phxrpc

