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

namespace phxrpc {

int UThreadProtectStack(void * stack_top, size_t stack_size) {
    int page = STACK_PROTECT_PAGE;
    int page_size = getpagesize();
    assert(stack_size >= (size_t)page_size * (page + 1));
    void * protect_addr = stack_top;
    if ((size_t)protect_addr & (page_size - 1)) {
        protect_addr = (void *)(((size_t)stack_top & (~(page_size - 1))) + page_size);
    }
    return mprotect(protect_addr, page_size * page, PROT_NONE);
}

int UThreadUnProtectStack(void * stack_top, int page) {
    void * protect_addr = stack_top;
    int page_size = getpagesize();
    if ((size_t)protect_addr & (page_size - 1)) {
        protect_addr = (void *)(((size_t)stack_top & (~(page_size - 1))) + page_size);
    }
    return mprotect(protect_addr, page_size * page, PROT_READ | PROT_WRITE);
}

} //namespace phxrpc
