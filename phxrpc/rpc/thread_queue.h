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

#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>

namespace phxrpc {

template <class T>
class ThdQueue {
public:
    ThdQueue() : break_out_(false), size_(0){ }
    ~ThdQueue() { break_out(); }

    size_t size() {
        return static_cast<size_t>(size_);
    }

    bool empty() {
        return static_cast<size_t>(size_) == 0;
    }

    void push(const T & value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        size_++;
        cv_.notify_one();
    }

    bool pluck(T & value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (break_out_) {
            return false;
        }
        while (queue_.empty()) {
            cv_.wait(lock);
            if (break_out_) {
                return false;
            }
        }
        size_--;
        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool pick(T & value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }

        size_--;
        value = queue_.front();
        queue_.pop();
        return true;
    }

    void break_out() {
        std::lock_guard<std::mutex> lock(mutex_);
        break_out_ = true;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    bool break_out_;
    std::atomic_int size_;
};

} //namespace phxrpc
