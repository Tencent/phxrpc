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

namespace phxrpc {

void __bad_assertion(const char *mess);

}

#undef __PHXRPC_STR
#undef __PHXRPC_XSTR
#undef PHXRPC_ASSERT

#define __PHXRPC_STR(x) # x
#define __PHXRPC_XSTR(x) __PHXRPC_STR(x)
#define PHXRPC_ASSERT(expr)                                             \
    ((expr) ? (void)0 :                                                 \
     phxrpc::__bad_assertion("Assertion \"" #expr                       \
                             "\" failed, file " __PHXRPC_XSTR(__FILE__) \
                             ", line " __PHXRPC_XSTR(__LINE__) "\n"))
