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

#include "timer.h"
#include <map>
#include <math.h>
#include <algorithm>
#include "uthread_epoll.h"

using namespace std;
using namespace phxrpc;

bool pass = true;

int PopTimeout(Timer & timer, std::map<UThreadSocket_t *, uint64_t> & exist_timer,
        std::map<UThreadSocket_t *, uint64_t> & removed_timer) {
    int next_timeout = 0;
    while(true) {
        next_timeout = timer.GetNextTimeout();
        if (next_timeout != 0) {
            return next_timeout;
        }

        UThreadSocket_t * socket = timer.PopTimeout();
        uint64_t now_time = Timer::GetSteadyClockMS();
        if (removed_timer.find(socket) != end(removed_timer)) {
            pass = false;
            printf("found removed timer, nowtime %lu", now_time);
        }

        printf("pop nowtime %lu expabstime %lu\n", now_time, exist_timer[socket]);

        uint64_t cut = exist_timer[socket] < now_time ? 
            now_time - exist_timer[socket]
            : exist_timer[socket] - now_time;
        
        if (cut > 10) {
            pass = false;
        }

        free(socket);
    }
    return next_timeout;
}

int main(int argc, char ** argv)
{
    Timer timer;

    std::map<UThreadSocket_t *, uint64_t> exist_timer;
    std::map<UThreadSocket_t *, uint64_t> removed_timer;
    std::vector<pair<UThreadSocket_t *, uint64_t> > need_remove;

    int timer_obj_count = 100;
    for (int i = 0; i < timer_obj_count; i++) {
        uint64_t abs_time = Timer::GetSteadyClockMS() + (rand() % 500);

        UThreadSocket_t * socket = NewUThreadSocket();
        timer.AddTimer(abs_time, socket);
        printf("add abstime %lu\n", abs_time);

        exist_timer[socket] = abs_time;
        if (need_remove.size() < (size_t)timer_obj_count / 2) {
            need_remove.push_back(make_pair(socket, abs_time));
        }
    }

    int next_timeout = 0;
    while(next_timeout != -1) {
        if (!need_remove.empty()) {
            pair<UThreadSocket_t *, uint64_t> & p = need_remove[need_remove.size() - 1];
            timer.RemoveTimer(UThreadSocketTimerID(*p.first));
            removed_timer[p.first] = p.second;
            need_remove.pop_back();
        }

        next_timeout = PopTimeout(timer, exist_timer, removed_timer);
        Timer::MsSleep(next_timeout);
    }

    printf("%s\n", pass ? "Pass..." : "NotPass...");
    return 0;
}

