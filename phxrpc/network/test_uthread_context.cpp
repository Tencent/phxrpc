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

#include "uthread_context_system.h"

using namespace phxrpc;

void f1(void *);
void f2(void *);

UThreadContextSystem c1(64 * 1024, &f1, nullptr, nullptr, true);
UThreadContextSystem c2(64 * 1024, &f2, nullptr, nullptr, true);

int test_count = 0;

void f1(void *) {
    for (int i = 0; i < test_count; i++) {
        //printf("f1 resume\n");
        c1.Yield();
    }
    //printf("f1 end\n");
}

void f2(void *) {
    for (int i = 0; i < test_count; i++) {
        //printf("f2 resume\n");
        c2.Yield();
    }
    //printf("f2 end\n");
}


int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("%s <test times>\n", argv[0]);
        return -2;
    }

    test_count = atoi(argv[1]);
    int test_count_2 = (test_count + 1) * 2;
    int test_count_3 = (test_count + 1);
    for (int i = 0; i < test_count_2; i++) {
        if (i < test_count_3) {
            //printf("start resume f1\n");
            c1.Resume();
        } else {
            //printf("start resume f2\n");
            c2.Resume();
        }
    }

    return 0;
}
