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

#include <unistd.h>
#include <ucontext.h>
#include <functional>
#include <assert.h>
#include "uthread_context_system.h"
#include "uthread_context_util.h"

namespace phxrpc {

UThreadContextSystem :: UThreadContextSystem(size_t stack_size, UThreadFunc_t func, void * args, UThreadDoneCallback_t callback)
    : func_(func), args_(args), stack_(nullptr), stack_size_(stack_size), 
    protect_page_(0), callback_(callback) {

    stack_ = (char *)calloc(1, stack_size_);
    assert(stack_ != nullptr);

    Make(func, args);
}

UThreadContextSystem :: ~UThreadContextSystem() {
    free(stack_);
}

UThreadContext * UThreadContextSystem :: DoCreate(size_t stack_size, 
        UThreadFunc_t func, void * args, UThreadDoneCallback_t callback) {
    return new UThreadContextSystem(stack_size, func, args, callback);
}

void UThreadContextSystem :: Make(UThreadFunc_t func, void * args) {
    func_ = func;
    args_ = args;
    getcontext(&context_);
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_stack.ss_flags = 0;
    context_.uc_link = GetMainContext();
    uintptr_t ptr = (uintptr_t)this;
    makecontext(&context_, (void (*)(void))UThreadContextSystem::UThreadFuncWrapper, 
            2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
}

bool UThreadContextSystem :: Resume() {
    swapcontext(GetMainContext(), &context_);
    return true;
}

bool UThreadContextSystem :: Yield() {
    swapcontext(&context_, GetMainContext());
    return true;
}

ucontext_t * UThreadContextSystem :: GetMainContext() {
    static thread_local ucontext_t main_context;
    return &main_context;
}

void UThreadContextSystem :: UThreadFuncWrapper(uint32_t low32, uint32_t high32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t) high32 << 32);
    UThreadContextSystem * uc = (UThreadContextSystem *)ptr;
    uc->func_(uc->args_);
    if (uc->callback_ != nullptr) {
        uc->callback_();
    }
}

} //namespace phxrpc
