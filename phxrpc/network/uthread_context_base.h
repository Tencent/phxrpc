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

namespace phxrpc {

class UThreadContext;

typedef std::function< void(void *) > UThreadFunc_t;
typedef std::function< void() > UThreadDoneCallback_t;
typedef std::function< UThreadContext* 
    (size_t, UThreadFunc_t, void *, UThreadDoneCallback_t, const bool) > ContextCreateFunc_t;

class UThreadContext {
public:
    UThreadContext() { }
    virtual ~UThreadContext() { }

    static UThreadContext * Create(size_t stack_size, 
            UThreadFunc_t func, void * args, 
            UThreadDoneCallback_t callback, const bool need_stack_protect);
    static void SetContextCreateFunc(ContextCreateFunc_t context_create_func);
    static ContextCreateFunc_t GetContextCreateFunc();

    virtual void Make(UThreadFunc_t func, void * args) = 0;
    virtual bool Resume() = 0;
    virtual bool Yield() = 0;

private:
    static ContextCreateFunc_t context_create_func_;
};

} //namespace phxrpc
