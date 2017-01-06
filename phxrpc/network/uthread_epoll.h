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

#include <vector>
#include <map>
#include <queue>
#include <arpa/inet.h>

#include "uthread_runtime.h"
#include "timer.h"

namespace phxrpc {

class UThreadEpollScheduler;

typedef struct tagUThreadSocket UThreadSocket_t;

typedef std::pair<UThreadEpollScheduler *, int> UThreadEpollArgs_t;

typedef std::function< UThreadSocket_t *() > UThreadActiveSocket_t;
typedef std::function< void() > UThreadHandlerAcceptedFdFunc_t;
typedef std::function< void() > UThreadHandlerNewRequest_t;

class EpollNotifier {
public:
    EpollNotifier(UThreadEpollScheduler * scheduler);
    ~EpollNotifier();

    void Run();
    void Func();
    void Notify();

private:
    UThreadEpollScheduler * scheduler_;
    int pipe_fds_[2];
};

class UThreadEpollScheduler {
public:
    UThreadEpollScheduler(size_t stack_size, int max_task, const bool need_stack_protect = true);
    ~UThreadEpollScheduler();

    static UThreadEpollScheduler * Instance();

    bool IsTaskFull();

    void AddTask(UThreadFunc_t func, void * args);

    UThreadSocket_t * CreateSocket(int fd, int socket_timeout_ms = 5000, 
            int connect_timeout_ms = 200, bool no_delay = true);

    void SetActiveSocketFunc(UThreadActiveSocket_t active_socket_func);

    void SetHandlerAcceptedFdFunc(UThreadHandlerAcceptedFdFunc_t handler_accepted_fd_func);

    void SetHandlerNewRequestFunc(UThreadHandlerNewRequest_t handler_new_request_func);

    bool YieldTask();

    bool Run();

    void RunForever();

    void Close();
    
    void NotifyEpoll();

    int GetCurrUThread();

    void AddTimer(UThreadSocket_t * socket, int timeout_ms);
    void RemoveTimer(const size_t timer_id);
    void DealwithTimeout(int & next_timeout);

private:
    typedef std::queue<std::pair<UThreadFunc_t, void *> > TaskQueue;
    void ConsumeTodoList();
    void ResumeAll(int flag);
    void StatEpollwaitEvents(const int event_count);

private:
    UThreadRuntime runtime_;
    int max_task_;
    TaskQueue todo_list_;
    int epoll_fd_;

    Timer timer_;
    bool closed_;
    bool run_forever_;

    UThreadActiveSocket_t active_socket_func_;
    UThreadHandlerAcceptedFdFunc_t handler_accepted_fd_func_;
    UThreadHandlerNewRequest_t handler_new_request_func_;

    int epoll_wait_events_;
    int epoll_wait_events_per_second_;
    uint64_t epoll_wait_events_last_cal_time_;

    EpollNotifier epoll_wake_up_;
};

class __uthread {
public:
    __uthread(UThreadEpollScheduler & scheduler) : scheduler_(scheduler) { }
    template <typename Func>
    void operator-(Func const & func) {
        scheduler_.AddTask(func, nullptr);
    }

private:
    UThreadEpollScheduler & scheduler_;
};

#define uthread_begin phxrpc::UThreadEpollScheduler _uthread_scheduler(64 * 1024, 300);
#define uthread_begin_withargs(stack_size, max_task) phxrpc::UThreadEpollScheduler _uthread_scheduler(stack_size, max_task);
#define uthread_s _uthread_scheduler
#define uthread_t phxrpc::__uthread(_uthread_scheduler)-
#define uthread_end _uthread_scheduler.Run();

//////////////////////////////////////////////////////////////////////

int UThreadPoll(UThreadSocket_t & socket, int events, int * revents, int timeout_ms);

int UThreadPoll(UThreadSocket_t * list[], int count, int timeout_ms);

int UThreadConnect(UThreadSocket_t & socket, const struct sockaddr *addr, socklen_t addrlen);

int UThreadAccept(UThreadSocket_t & socket, struct sockaddr *addr, socklen_t *addrlen);

ssize_t UThreadRecv(UThreadSocket_t & socket, void * buf, size_t len, int flags);

ssize_t UThreadRead(UThreadSocket_t & socket, void * buf, size_t len, int flags);

ssize_t UThreadSend(UThreadSocket_t & socket, const void *buf, size_t len, int flags);

int UThreadClose(UThreadSocket_t & socket);

void UThreadSetConnectTimeout(UThreadSocket_t & socket, int connect_timeout_ms);

void UThreadSetSocketTimeout(UThreadSocket_t & socket, int socket_timeout_ms);

int UThreadSocketFd(UThreadSocket_t & socket);

size_t UThreadSocketTimerID(UThreadSocket_t & socket);

void UThreadSocketSetTimerID(UThreadSocket_t & socket, size_t timer_id);

UThreadSocket_t * NewUThreadSocket();

void UThreadSetArgs(UThreadSocket_t & socket, void * args);

void * UThreadGetArgs(UThreadSocket_t & socket);

void UThreadWait(UThreadSocket_t & socket, int timeout_ms);

void UThreadLazyDestory(UThreadSocket_t & socket);

bool IsUThreadDestory(UThreadSocket_t & socket);

};

