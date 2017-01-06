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

#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include "uthread_runtime.h"
#include "uthread_context_system.h"

enum {
    UTHREAD_RUNNING,
    UTHREAD_SUSPEND,
    UTHREAD_DONE
};

namespace phxrpc {

UThreadRuntime :: UThreadRuntime(size_t stack_size, const bool need_stack_protect)
    :stack_size_(stack_size), first_done_item_(-1),
    current_uthread_(-1), unfinished_item_count_(0),
    need_stack_protect_(need_stack_protect) {
    if (UThreadContext::GetContextCreateFunc() == nullptr) {
        UThreadContext::SetContextCreateFunc(UThreadContextSystem::DoCreate);
    }
}

UThreadRuntime :: ~UThreadRuntime() {
    for (auto & context_slot : context_list_) {
        delete context_slot.context;
    }
}

int UThreadRuntime :: Create(UThreadFunc_t func, void * args) {
    if (func == nullptr) {
        return -2;
    }
    int index = -1;
    if (first_done_item_ >= 0) {
        index = first_done_item_;
        first_done_item_ = context_list_[index].next_done_item;
        context_list_[index].context->Make(func, args);
    } else {
        index = context_list_.size();
        auto new_context = UThreadContext::Create(stack_size_, func, args, 
                std::bind(&UThreadRuntime::UThreadDoneCallback, this),
                need_stack_protect_);
        assert(new_context != nullptr);
        ContextSlot context_slot;
        context_slot.context = new_context;
        context_list_.push_back(context_slot);
    }

    context_list_[index].next_done_item = -1;
    context_list_[index].status = UTHREAD_SUSPEND;
    unfinished_item_count_++;
    return index;
}

void UThreadRuntime :: UThreadDoneCallback() {
    if (current_uthread_ != -1) {
        ContextSlot & context_slot = context_list_[current_uthread_];
        context_slot.next_done_item = first_done_item_;
        context_slot.status = UTHREAD_DONE;
        first_done_item_ = current_uthread_;
        unfinished_item_count_--;
        current_uthread_ = -1;
    }
} 

bool UThreadRuntime :: Resume(size_t index) {
    if (index >= context_list_.size()) {
        return false;
    }

    auto context_slot = context_list_[index];
    if (context_slot.status == UTHREAD_SUSPEND) {
        current_uthread_ = index;
        context_slot.status = UTHREAD_RUNNING;
        context_slot.context->Resume();
        return true;
    }
    return false;
}

bool UThreadRuntime :: Yield() {
    if (current_uthread_ != -1) {
        auto context_slot = context_list_[current_uthread_];
        current_uthread_ = -1;
        context_slot.status = UTHREAD_SUSPEND;
        context_slot.context->Yield();
    }

    return true;
}

int UThreadRuntime::GetCurrUThread() {
    return current_uthread_;
}

bool UThreadRuntime::IsAllDone() {
    return unfinished_item_count_ == 0;
}

int UThreadRuntime :: GetUnfinishedItemCount() const {
    return unfinished_item_count_;
}

}

