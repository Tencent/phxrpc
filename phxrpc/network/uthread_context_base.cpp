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

#include "uthread_context_base.h"

namespace phxrpc {

ContextCreateFunc_t UThreadContext::context_create_func_ = nullptr;

UThreadContext * UThreadContext :: Create(size_t stack_size, 
        UThreadFunc_t func, void * args, 
        UThreadDoneCallback_t callback, const bool need_stack_protect) {
    if (context_create_func_ != nullptr) {
        return context_create_func_(stack_size, func, args, callback, need_stack_protect);
    }
    return nullptr;
}

void UThreadContext :: SetContextCreateFunc(ContextCreateFunc_t context_create_func) {
    context_create_func_ = context_create_func;
}

ContextCreateFunc_t UThreadContext :: GetContextCreateFunc() {
    return context_create_func_;
}

} //namespace phxrpc
