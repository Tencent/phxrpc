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

#include "uthread_runtime.h"
#include <stdio.h>
#include <time.h>

using namespace phxrpc;

typedef struct tagTestArgs {
    UThreadRuntime * scheduler;
    int id;

    int seq;
} TestArgs_t;

void test(void * args) {
    TestArgs_t * tt = (TestArgs_t*) args;

    printf("%d-%d START\n", tt->id, tt->seq);

    for (int i = 0; i < 8; i++) {
        printf("%d-%d-%d\n", tt->id, tt->seq, i);

        tt->scheduler->Yield();
    }

    printf("%d-%d END\n", tt->id, tt->seq);
}

void execute(UThreadRuntime & runtime, size_t count) {
    srandom (time(NULL) );

    TestArgs_t * args = (TestArgs_t*)calloc( count, sizeof( TestArgs_t ) );

    for( size_t i = 0; i < count; i++ ) {
        args[i].seq = i;

        args[i].scheduler = &runtime;
        args[i].id = runtime.Create( test, &(args[i]) );
    }

    for(; ! runtime.IsAllDone(); ) {
        int seq = random() % count;
        runtime.Resume( args[ seq ].id );
    }

    free( args );
}

void run(size_t count) {
    UThreadRuntime runtime(64 * 1024, false);

    execute(runtime, count);

    printf("\n");

    execute(runtime, count);
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: %s <count>\n", argv[0]);
        return -1;
    }

    run(atoi(argv[1]));

    return 0;
}

