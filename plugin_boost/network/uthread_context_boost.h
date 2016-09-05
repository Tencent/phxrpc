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

#include <unistd.h>
#include <functional>
#include <assert.h>
#include <boost/context/all.hpp>
#include "phxrpc/network.h"

namespace phxrpc {

class UThreadContextBoostInit {
public:
    UThreadContextBoostInit();
};

class UThreadContextBoost : public UThreadContext {
public:
    UThreadContextBoost(size_t stack_size, UThreadFunc_t func, void * args, 
            UThreadDoneCallback_t callback, const bool need_stack_protect);
    ~UThreadContextBoost();

    static UThreadContext * DoCreate(size_t stack_size, 
            UThreadFunc_t func, void * args, UThreadDoneCallback_t callback,
            const bool need_stack_protect);

    void Make(UThreadFunc_t func, void * args) override;
    bool Resume() override;
    bool Yield() override;

    boost::context::fcontext_t & GetMainContext();

private:
    static void UThreadFuncWrapper(intptr_t ptr);

    boost::context::fcontext_t context_;
    UThreadFunc_t func_;
    void * args_;
    UThreadStackMemory stack_;
    UThreadDoneCallback_t callback_;
};

} //namespace phxrpc
