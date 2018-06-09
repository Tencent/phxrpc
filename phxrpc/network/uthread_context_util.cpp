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

#include "uthread_context_util.h"

#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

#include "phxrpc/comm.h"

namespace phxrpc {


#ifdef __APPLE__
#define MAP_ANONYMOUS MAP_ANON
#endif


UThreadStackMemory :: UThreadStackMemory(const size_t stack_size, const bool need_protect) :
    raw_stack_(nullptr), stack_(nullptr), need_protect_(need_protect) {
    int page_size = getpagesize();
    if ((stack_size % page_size) != 0) {
        stack_size_ = (stack_size / page_size + 1) * page_size;
    } else {
        stack_size_ = stack_size;
    }

    if (need_protect) {
        raw_stack_ = mmap(NULL, stack_size_ + page_size * 2, 
                PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert(raw_stack_ != nullptr);

        PHXRPC_ASSERT(mprotect(raw_stack_, page_size, PROT_NONE) == 0);
        PHXRPC_ASSERT(mprotect((void *)((char *)raw_stack_ + stack_size_ + page_size), page_size, PROT_NONE) == 0);

        stack_ = (void *)((char *)raw_stack_ + page_size);
    } else {
        raw_stack_ = mmap(NULL, stack_size_, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert(raw_stack_ != nullptr);
        stack_ = raw_stack_;
    }
}

UThreadStackMemory :: ~UThreadStackMemory() {
    int page_size = getpagesize();
    if (need_protect_) {
        PHXRPC_ASSERT(mprotect(raw_stack_, page_size, PROT_READ | PROT_WRITE) == 0);
        PHXRPC_ASSERT(mprotect((void *)((char *)raw_stack_ + stack_size_ + page_size), page_size, PROT_READ | PROT_WRITE) == 0);
        PHXRPC_ASSERT(munmap(raw_stack_, stack_size_ + page_size * 2) == 0);
    } else {
        PHXRPC_ASSERT(munmap(raw_stack_, stack_size_) == 0);
    }
}

void * UThreadStackMemory :: top() {
    return stack_;
}

size_t UThreadStackMemory :: size() {
    return stack_size_;
}


}  // namespace phxrpc

