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

#include <cinttypes>
#include <cstdlib>
#include <vector>


namespace phxrpc {


typedef struct tagUThreadSocket UThreadSocket_t;

class Timer final {
 public:
    Timer();
    ~Timer();

    void AddTimer(uint64_t abs_time, UThreadSocket_t *socket);
    void RemoveTimer(const size_t timer_id);
    UThreadSocket_t *PopTimeout();
    const int GetNextTimeout() const;
    const bool empty();
    static const uint64_t GetTimestampMS();
    static const uint64_t GetSteadyClockMS();
    static void MsSleep(const int time_ms);
    std::vector<UThreadSocket_t *> GetSocketList();

 private:
    void heap_up(const size_t end_idx);
    void heap_down(const size_t begin_idx);

    struct TimerObj {
        TimerObj(uint64_t abs_time, UThreadSocket_t *socket)
                : abs_time_(abs_time), socket_(socket){
        }

        uint64_t abs_time_;
        UThreadSocket_t *socket_;

        bool operator <(const TimerObj &obj) const {
            return abs_time_ < obj.abs_time_;
        }

        bool operator ==(const TimerObj &obj) const {
            return abs_time_ == obj.abs_time_;
        }
    };

    std::vector<TimerObj> timer_heap_;
};


}

