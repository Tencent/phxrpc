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

#include "uthread_epoll.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#ifdef __APPLE__
#include "epoll-darwin.h"
#else
#include <sys/epoll.h>
#endif

#include "phxrpc/comm.h"
#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"


namespace phxrpc {


using namespace std;


typedef struct tagUThreadSocket {
    UThreadEpollScheduler *scheduler;
    int uthread_id;

    int epoll_fd;

    int socket;
    int connect_timeout_ms;
    int socket_timeout_ms;

    int waited_events;
    size_t timer_id;
    struct epoll_event event;
    void * args;
} UThreadSocket_t;

EpollNotifier::EpollNotifier(UThreadEpollScheduler *scheduler)
        : scheduler_(scheduler) {
    pipe_fds_[0] = pipe_fds_[1] = -1;
}

EpollNotifier::~EpollNotifier() {
    if (pipe_fds_[0] != -1) {
        close(pipe_fds_[0]);
    }
    if (pipe_fds_[1] != -1) {
        close(pipe_fds_[1]);
    }
}

void EpollNotifier::Run() {
    PHXRPC_ASSERT(pipe(pipe_fds_) == 0);
    fcntl(pipe_fds_[1], F_SETFL, O_NONBLOCK);
    scheduler_->AddTask(std::bind(&EpollNotifier::Func, this), nullptr);
}

void EpollNotifier::Func() {
    UThreadSocket_t *socket{scheduler_->CreateSocket(pipe_fds_[0], -1, -1, false)};
    char tmp[2] = {0};
    while (true) {
        if (UThreadRead(*socket, tmp, 1, 0) < 0) {
            break;
        }
    }
    free(socket);
}

void EpollNotifier::Notify() {
    ssize_t write_len = write(pipe_fds_[1], (void *)"a", 1);
    if (0 > write_len) {
        //log(LOG_ERR, "%s write err", __func__);
    }
}


UThreadNotifier::UThreadNotifier() {
    pipe_fds_[0] = pipe_fds_[1] = -1;
}

UThreadNotifier::~UThreadNotifier() {
    free(socket_);
    if (pipe_fds_[0] != -1) {
        close(pipe_fds_[0]);
    }
    if (pipe_fds_[1] != -1) {
        close(pipe_fds_[1]);
    }
}

int UThreadNotifier::Init(UThreadEpollScheduler *const scheduler,
                          const int timeout_ms) {
    int ret{pipe(pipe_fds_)};
    if (0 != ret) return ret;
    fcntl(pipe_fds_[1], F_SETFL, O_NONBLOCK);
    socket_ = scheduler->CreateSocket(pipe_fds_[0], timeout_ms, -1, false);

    return 0;
}

int UThreadNotifier::SendNotify(void *const data) {
    data_ = data;
    ssize_t write_len{write(pipe_fds_[1], (void *)"a", 1)};
    if (0 > write_len) {
        log(LOG_ERR, "%s write errno %d", __func__, errno);

        return -1;
    }

    return 0;
}

int UThreadNotifier::WaitNotify(void *&data) {
    char buf;
    ssize_t read_len{UThreadRead(*socket_, (void *)&buf, 1, 0)};
    if (0 > read_len) {
        log(LOG_ERR, "%s UThreadRead errno %d", __func__, errno);

        return -1;
    }
    data = data_;
    data_ = nullptr;

    return 0;
}


UThreadNotifierPool::UThreadNotifierPool(UThreadEpollScheduler *const scheduler,
                                         const int timeout_ms)
        : scheduler_(scheduler), timeout_ms_(timeout_ms) {
}

UThreadNotifierPool::~UThreadNotifierPool() {
    for (auto &it : pool_map_) {
        delete it.second;
    }
}

int UThreadNotifierPool::GetNotifier(const string &id, UThreadNotifier *&notifier) {
    notifier = nullptr;

    auto it(pool_map_.find(id));
    if (pool_map_.end() != it && it->second) {
        notifier = it->second;

        return 0;
    }

    notifier = new UThreadNotifier();
    int ret{notifier->Init(scheduler_, timeout_ms_)};
    if (0 != ret) {
        return ret;
    }

    pool_map_[id] = notifier;

    return 0;
}

void UThreadNotifierPool::ReleaseNotifier(const string &id) {
    if (pool_map_[id]) {
        delete pool_map_[id];
        pool_map_[id] = nullptr;
    }
    pool_map_.erase(id);
}

////////////////////////////////////////////////

enum UThreadEpollREventStatus {
    UThreadEpollREvent_Timeout = 0,
    UThreadEpollREvent_Error = -1,
    UThreadEpollREvent_Close = -2,
};

UThreadEpollScheduler::UThreadEpollScheduler(size_t stack_size, int max_task, const bool need_stack_protect) :
    runtime_(stack_size, need_stack_protect), epoll_wake_up_(this) {
    //epoll notifier use one task.
    max_task_ = max_task + 1;

    epoll_fd_ = epoll_create(max_task_);

    if (epoll_fd_ < 0) {
        phxrpc::log(LOG_ERR, "ERR: epoll_create( %d ) %d, errno %d, %s", max_task_, epoll_fd_, errno, strerror(errno));
        assert(epoll_fd_ >= 0);
    }

    closed_ = false;
    run_forever_ = false;
    active_socket_func_ = nullptr;
    handler_accepted_fd_func_ = nullptr;
    handler_new_request_func_ = nullptr;

    epoll_wait_events_ = 0;
    epoll_wait_events_per_second_ = 0;
    epoll_wait_events_last_cal_time_ = Timer::GetSteadyClockMS();
}

UThreadEpollScheduler::~UThreadEpollScheduler() {
    close(epoll_fd_);
}

UThreadEpollScheduler *UThreadEpollScheduler::Instance() {
    static UThreadEpollScheduler obj(64 * 1024, 300);
    return &obj;
}

bool UThreadEpollScheduler::IsTaskFull() {
    return (runtime_.GetUnfinishedItemCount() + (int)todo_list_.size()) >= max_task_;
}

void UThreadEpollScheduler::AddTask(UThreadFunc_t func, void *args) {
    todo_list_.push(std::make_pair(func, args));
}

void UThreadEpollScheduler::SetActiveSocketFunc(UThreadActiveSocket_t active_socket_func) {
    active_socket_func_ = active_socket_func;
}

void UThreadEpollScheduler::SetHandlerNewRequestFunc(UThreadHandlerNewRequest_t handler_new_request_func) {
    handler_new_request_func_ = handler_new_request_func;
}

void UThreadEpollScheduler::SetHandlerAcceptedFdFunc(UThreadHandlerAcceptedFdFunc_t handler_accepted_fd_func) {
    handler_accepted_fd_func_ = handler_accepted_fd_func;
}

bool UThreadEpollScheduler::YieldTask() {
    return runtime_.Yield();
}

int UThreadEpollScheduler::GetCurrUThread() {
    return runtime_.GetCurrUThread();
}

UThreadSocket_t *UThreadEpollScheduler::CreateSocket(const int fd,
        const int socket_timeout_ms, const int connect_timeout_ms,
        const bool no_delay) {
    UThreadSocket_t *socket = (UThreadSocket_t *)calloc(1, sizeof(UThreadSocket_t));

    BaseTcpUtils::SetNonBlock(fd, true);
    if (no_delay) {
        BaseTcpUtils::SetNoDelay(fd, true);
    }

    socket->scheduler = this;
    socket->epoll_fd = epoll_fd_;
    socket->event.data.ptr = socket;

    socket->socket = fd;
    socket->connect_timeout_ms = connect_timeout_ms;
    socket->socket_timeout_ms = socket_timeout_ms;

    socket->waited_events = 0;
    socket->args = nullptr;

    return socket;
}

void UThreadEpollScheduler::ConsumeTodoList() {
    while (!todo_list_.empty()) {
        auto & it = todo_list_.front();
        int id = runtime_.Create(it.first, it.second);
        runtime_.Resume(id);

        todo_list_.pop();
    }
}

void UThreadEpollScheduler::Close() {
    closed_ = true;
}

void UThreadEpollScheduler::NotifyEpoll() {
    if (epoll_wait_events_per_second_ < 2000) {
        //phxrpc::log(LOG_ERR, "%s now epoll_wait per second %d", __func__, epoll_wait_events_per_second_);
        epoll_wake_up_.Notify();
    }
}

void UThreadEpollScheduler::ResumeAll(int flag) {
    std::vector<UThreadSocket_t*> exist_socket_list = timer_.GetSocketList();
    for (auto & socket : exist_socket_list) {
        socket->waited_events = flag;
        runtime_.Resume(socket->uthread_id);
    }
}

void UThreadEpollScheduler::RunForever() {
    run_forever_ = true;
    epoll_wake_up_.Run();
    Run();
}

void UThreadEpollScheduler::StatEpollwaitEvents(const int event_count) {
    epoll_wait_events_ += event_count;
    auto now_time = Timer::GetSteadyClockMS();
    if (now_time > epoll_wait_events_last_cal_time_ + 1000) {
        epoll_wait_events_per_second_ = epoll_wait_events_;
        epoll_wait_events_ = 0;
        epoll_wait_events_last_cal_time_ = now_time;
        //phxrpc::log(LOG_ERR, "%s now epoll_wait per second %d", __func__, epoll_wait_events_per_second_);
    }
}

bool UThreadEpollScheduler::Run() {
    ConsumeTodoList();

    struct epoll_event *events = (struct epoll_event*) calloc(max_task_, sizeof(struct epoll_event));

    int next_timeout = timer_.GetNextTimeout();

    for (; (run_forever_) || (!runtime_.IsAllDone());) {
        int nfds = epoll_wait(epoll_fd_, events, max_task_, 4);
        if (nfds != -1) {
            for (int i = 0; i < nfds; i++) {
                UThreadSocket_t * socket = (UThreadSocket_t*) events[i].data.ptr;
                socket->waited_events = events[i].events;

                runtime_.Resume(socket->uthread_id);
            }

            // for server mode
            if (active_socket_func_ != nullptr) {
                UThreadSocket_t * socket = nullptr;
                while ((socket = active_socket_func_()) != nullptr) {
                    runtime_.Resume(socket->uthread_id);
                }
            }

            // for server uthread worker
            if (handler_new_request_func_ != nullptr) {
                handler_new_request_func_();
            }

            if (handler_accepted_fd_func_ != nullptr) {
                handler_accepted_fd_func_();
            }

            if (closed_) {
                ResumeAll(UThreadEpollREvent_Close);
                break;
            }

            ConsumeTodoList();
            DealwithTimeout(next_timeout);
        } else if (errno != EINTR) {
            ResumeAll(UThreadEpollREvent_Error);
            break;
        }

        StatEpollwaitEvents(nfds);
    }

    free(events);

    return true;
}

void UThreadEpollScheduler::AddTimer(UThreadSocket_t *socket, const int timeout_ms) {
    RemoveTimer(socket->timer_id);

    if (timeout_ms == -1) {
        timer_.AddTimer((std::numeric_limits<uint64_t>::max)(), socket);
    } else {
        timer_.AddTimer(Timer::GetSteadyClockMS() + timeout_ms, socket);
    }
}

void UThreadEpollScheduler::RemoveTimer(const size_t timer_id) {
    if (timer_id > 0) {
        timer_.RemoveTimer(timer_id);
    }
}

void UThreadEpollScheduler::DealwithTimeout(int &next_timeout) {
    while (true) {
        next_timeout = timer_.GetNextTimeout();
        if (0 != next_timeout) {
            break;
        }

        UThreadSocket_t * socket = timer_.PopTimeout();
        socket->waited_events = UThreadEpollREvent_Timeout;
        runtime_.Resume(socket->uthread_id);
    }
}

//////////////////////////////////////////////////////////////////////

int UThreadPoll(UThreadSocket_t &socket, int events, int *revents, const int timeout_ms) {
    int ret{-1};

    socket.uthread_id = socket.scheduler->GetCurrUThread();

    socket.event.events = events;

    socket.scheduler->AddTimer(&socket, timeout_ms);
    epoll_ctl(socket.epoll_fd, EPOLL_CTL_ADD, socket.socket, &socket.event);

    socket.scheduler->YieldTask();

    epoll_ctl(socket.epoll_fd, EPOLL_CTL_DEL, socket.socket, &socket.event);
    socket.scheduler->RemoveTimer(socket.timer_id);

    *revents = socket.waited_events;

    if ((*revents) > 0) {
        if ((*revents) & events) {
            ret = 1;
        } else {
            errno = EINVAL;
            ret = 0;
        }
    } else if ((*revents) == UThreadEpollREvent_Timeout) {
        // timeout
        errno = ETIMEDOUT;
        ret = 0;
    } else if ((*revents) == UThreadEpollREvent_Error){
        // error
        errno = ECONNREFUSED;
        ret = -1;
    } else {
        // active close
        errno = 0;
        ret = -1;
    }

    return ret;
}

int UThreadPoll(UThreadSocket_t *list[], int count, const int timeout_ms) {
    int nfds = -1;

    UThreadSocket_t *socket = list[0];

    int epollfd = epoll_create(count);

    for (int i = 0; i < count; i++) {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, list[i]->socket, &(list[i]->event));
    }

    UThreadSocket_t fake_socket;
    fake_socket.scheduler = socket->scheduler;
    fake_socket.socket = epollfd;
    fake_socket.uthread_id = socket->scheduler->GetCurrUThread();
    fake_socket.event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    fake_socket.event.data.ptr = &fake_socket;
    fake_socket.waited_events = 0;

    epoll_ctl(socket->epoll_fd, EPOLL_CTL_ADD, epollfd, &(fake_socket.event));

    socket->scheduler->YieldTask();

    if (0 != (EPOLLIN & fake_socket.waited_events)) {
        struct epoll_event * events = (struct epoll_event*)calloc(count, sizeof(struct epoll_event));

        nfds = epoll_wait(epollfd, events, count, 0);

        for (int i = 0; i < nfds; i++) {
            UThreadSocket_t * socket = (UThreadSocket_t*) events[i].data.ptr;
            socket->waited_events = events[i].events;
        }
    } else if (0 == fake_socket.waited_events) {
        nfds = 0;
    }

    close(epollfd);

    return nfds;
}

int UThreadConnect(UThreadSocket_t &socket, const struct sockaddr *addr, socklen_t addrlen) {
    int ret = connect(socket.socket, addr, addrlen);

    if (0 != ret) {
        if (EAGAIN != errno && EINPROGRESS != errno)
            return -1;

        int revents = 0;
        if (UThreadPoll(socket, EPOLLOUT, &revents, socket.connect_timeout_ms) > 0) {
            ret = 0;
        } else {
            ret = -1;
        }
    }

    return ret;
}

int UThreadAccept(UThreadSocket_t &socket, struct sockaddr *addr, socklen_t *addrlen) {
    int ret = accept(socket.socket, addr, addrlen);
    if (ret < 0) {
        if (EAGAIN != errno && EWOULDBLOCK != errno) {
            return -1;
        }

        int revents = 0;
        if (UThreadPoll(socket, EPOLLIN, &revents, -1) > 0) {
            ret = accept(socket.socket, addr, addrlen);
        } else {
            ret = -1;
        }
    }

    return ret;
}

ssize_t UThreadRead(UThreadSocket_t &socket, void *buf, size_t len, const int flags) {
    int ret = read(socket.socket, buf, len);

    if (ret < 0 && EAGAIN == errno) {
        int revents = 0;
        if (UThreadPoll(socket, EPOLLIN, &revents, socket.socket_timeout_ms) > 0) {
            ret = read(socket.socket, buf, len);
        } else {
            ret = -1;
        }
    }

    return ret;
}

ssize_t UThreadRecv(UThreadSocket_t &socket, void *buf, size_t len, const int flags) {
    int ret = recv(socket.socket, buf, len, flags);

    if (ret < 0 && EAGAIN == errno) {
        int revents = 0;
        if (UThreadPoll(socket, EPOLLIN, &revents, socket.socket_timeout_ms) > 0) {
            ret = recv(socket.socket, buf, len, flags);
        } else {
            ret = -1;
        }
    }

    return ret;
}

ssize_t UThreadSend(UThreadSocket_t &socket, const void *buf, size_t len, const int flags) {
    int ret = send(socket.socket, buf, len, flags);

    if (ret < 0 && EAGAIN == errno) {
        int revents = 0;
        if (UThreadPoll(socket, EPOLLOUT, &revents, socket.socket_timeout_ms) > 0) {
            ret = send(socket.socket, buf, len, flags);
        } else {
            ret = -1;
        }
    }

    return ret;
}

int UThreadClose(UThreadSocket_t &socket) {
    if (socket.socket >= 0) {
        return close(socket.socket);
    }
    return -1;
}

void UThreadSetConnectTimeout(UThreadSocket_t &socket, const int connect_timeout_ms) {
    socket.connect_timeout_ms = connect_timeout_ms;
}

void UThreadSetSocketTimeout(UThreadSocket_t &socket, const int socket_timeout_ms) {
    socket.socket_timeout_ms = socket_timeout_ms;
}

int UThreadSocketFd(UThreadSocket_t &socket) {
    return socket.socket;
}

size_t UThreadSocketTimerID(UThreadSocket_t &socket) {
    return socket.timer_id;
}

void UThreadSocketSetTimerID(UThreadSocket_t &socket, size_t timer_id) {
    socket.timer_id = timer_id;
}

UThreadSocket_t *NewUThreadSocket() {
    UThreadSocket_t *socket = (UThreadSocket_t *)calloc(1, sizeof(UThreadSocket_t));
    return socket;
}

void UThreadSetArgs(UThreadSocket_t &socket, void *args) {
    socket.args = args;
}

void *UThreadGetArgs(UThreadSocket_t &socket) {
    return socket.args;
}

void UThreadWait(UThreadSocket_t &socket, const int timeout_ms) {
    socket.uthread_id = socket.scheduler->GetCurrUThread();
    socket.scheduler->AddTimer(&socket, timeout_ms);
    socket.scheduler->YieldTask();
    socket.scheduler->RemoveTimer(socket.timer_id);
}

void UThreadLazyDestory(UThreadSocket_t &socket) {
    socket.uthread_id = -1;
}

bool IsUThreadDestory(UThreadSocket_t &socket) {
    return socket.uthread_id == -1;
}


};

