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

#include <cstdio>

#include "proto_utils.h"


using namespace phxrpc;


void PrintTree(SyntaxTree *stree) {
    printf("proto file %s\n", stree->proto_file());
    printf("prefix %s, name %s\n", stree->prefix(), stree->GetName());

    printf("\n");

    auto flist(stree->func_list());
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        printf("request %s, type %s\n", fit->GetReq()->GetName(), fit->GetReq()->GetType());
        printf("response %s, type %s\n", fit->GetResp()->GetName(), fit->GetResp()->GetType());
        printf("optstring %s, usage %s\n", fit->GetOptString(), fit->GetUsage());

        printf("\n");
    }
}

void test(const char *file) {
    SyntaxTree stree;

    std::vector<std::string> include_list;
    include_list.push_back("./");
    int ret = ProtoUtils::Parse(file, &stree, include_list);

    printf("Parse %d\n", ret);

    PrintTree(&stree);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <proto file>\n", argv[0]);
        return -1;
    }

    const char *file = argv[1];

    test(file);

    return 0;
}

