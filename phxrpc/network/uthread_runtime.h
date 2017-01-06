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

#include <stdlib.h>
#include <vector>
#include <functional>
#include "uthread_context_base.h"

namespace phxrpc {

class UThreadRuntime {
public:
    UThreadRuntime(size_t stack_size, const bool need_stack_protect);
    ~UThreadRuntime();

    int Create(UThreadFunc_t func, void * args);
    int GetCurrUThread();
    bool Yield();
    bool Resume(size_t index);
    bool IsAllDone();
    int GetUnfinishedItemCount() const;

    void UThreadDoneCallback();

private:
    struct ContextSlot {
        ContextSlot() {
            context = nullptr;
            next_done_item = -1;
        }
        UThreadContext * context;
        int next_done_item;
        int status;
    };

    size_t stack_size_;
    std::vector<ContextSlot> context_list_;
    int first_done_item_;
    int current_uthread_;
    int unfinished_item_count_;
    bool need_stack_protect_;
};

} //namespace phxrpc


