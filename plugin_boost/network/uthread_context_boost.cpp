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
#include <functional>
#include <assert.h>
#include <boost/context/all.hpp>

#include "uthread_context_boost.h"
#include "phxrpc/network.h"

namespace phxrpc {

UThreadContextBoostInit g_uthread_context_boost_init_;

UThreadContextBoostInit :: UThreadContextBoostInit() {
    UThreadContext::SetContextCreateFunc(UThreadContextBoost::DoCreate);
}

UThreadContextBoost :: UThreadContextBoost(size_t stack_size, UThreadFunc_t func, 
        void * args, UThreadDoneCallback_t callback)
    : func_(func), args_(args), stack_(nullptr), stack_size_(stack_size), 
    protect_page_(0), callback_(callback) {
    stack_ = (char *)calloc(1, stack_size_);
    assert(stack_ != nullptr);
    Make(func, args);
}

UThreadContextBoost :: ~UThreadContextBoost() {
    free(stack_);
}

UThreadContext * UThreadContextBoost:: DoCreate(size_t stack_size, 
        UThreadFunc_t func, void * args, UThreadDoneCallback_t callback) {
    return new UThreadContextBoost(stack_size, func, args, callback);
}

void UThreadContextBoost :: Make(UThreadFunc_t func, void * args) {
    func_ = func;
    args_ = args;
    void * stack_p = (void *)(stack_ + stack_size_);
    context_ = boost::context::make_fcontext(stack_p, stack_size_, &UThreadFuncWrapper);
}

bool UThreadContextBoost :: Resume() {
    boost::context::jump_fcontext(&GetMainContext(), context_, reinterpret_cast<intptr_t>(this));
    return true;
}

bool UThreadContextBoost :: Yield() {
    boost::context::jump_fcontext(&context_, GetMainContext(), 0);
    return true;
}

boost::context::fcontext_t & UThreadContextBoost :: GetMainContext() {
    static thread_local boost::context::fcontext_t main_context;
    return main_context;
}

void UThreadContextBoost :: UThreadFuncWrapper(intptr_t ptr) {
    UThreadContextBoost * uc = reinterpret_cast<UThreadContextBoost *>(ptr);
    uc->func_(uc->args_);
    if (uc->callback_ != nullptr) {
        uc->callback_();
    }
    uc->Yield();
}

} //namespace phxrpc
