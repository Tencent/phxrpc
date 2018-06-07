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

#include <cstdio>
#include <thread>
#include <unistd.h>
#include <vector>

#include "thread_queue.h"


using namespace std;
using namespace phxrpc;


class PluckThread {
  public:
    PluckThread(ThdQueue<int> * thd_queue, const size_t thread_count) :
    thd_queue_(thd_queue) {
        for (size_t i = 0; i < thread_count; i++) {
            thread * thd = new thread(&PluckThread::Func, this, i);
            thread_list_.push_back(thd);
        }
    }

    ~PluckThread() {
        thd_queue_->break_out();
        for (auto & thd : thread_list_) {
            thd->join();
            delete thd;
        }
    }

    void Func(size_t id) {
        int n = 0;
        do {
            bool succ = thd_queue_->pluck(n);
            if (succ) {
                printf("thread_id(%zu) value(%d)\n", id, n);
            } else {
                break;
            }
        } while (!thd_queue_->empty());
    }

  private:
    ThdQueue<int> * thd_queue_;
    vector<thread *> thread_list_;
};

int main(int argc, char ** argv) {
    ThdQueue<int> thd_queue;
    size_t n_size = 10;
    PluckThread threads(&thd_queue, n_size);

    for (size_t i = 0; i < n_size; i++) {
        thd_queue.push(i);
    }

    return 0;
}

