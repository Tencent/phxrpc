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

const char * PHXRPC_TOOL_MAIN_TEMPLATE =
        R"(

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>

#include "$ToolFile$.h"
#include "$ToolImplFile$.h"

#include "$ClientFile$.h"

#include "phxrpc/file.h"


using namespace phxrpc;


void ShowUsage(const char *program) {
    printf("\nUsage: %s [-c <config>] [-f <func>] [-v]\n", program);

    $ToolClass$::Name2Func_t *name2func = $ToolClass$::GetName2Func();

    for (int i{0}; ; ++i) {
        $ToolClass$::Name2Func_t *iter = &(name2func[i]);

        if (nullptr == iter->name) break;

        printf("    -f %s %s\n", iter->name, iter->usage);
    }
    printf("\n");
    exit(0);
}

int main(int argc, char **argv) {
    const char *func{nullptr};
    const char *config{nullptr};

    for (int i{1}; argc - 1 > i; ++i) {
        if (0 == strcmp(argv[i], "-c")) {
            config = argv[++i];
        }
        if (0 == strcmp(argv[i], "-f")) {
            func = argv[++i];
        }
        if (0 == strcmp(argv[i], "-v")) {
            ShowUsage(argv[0]);
        }
    }

    if (nullptr == func) ShowUsage(argv[0]);

    if (nullptr != config) $ClientClass$::Init(config);

    $ToolClass$::Name2Func_t *target{nullptr};

    $ToolClass$::Name2Func_t *name2func{$ToolClass$::GetName2Func()};

    for (int i{0}; 100 > i; ++i) {
        $ToolClass$::Name2Func_t *iter = &(name2func[i]);

        if (nullptr == iter->name) break;

        if (0 == strcasecmp(func, iter->name)) {
            target = iter;
            break;
        }
    }

    if (nullptr == target) ShowUsage(argv[0]);

    OptMap opt_map(target->opt_string);

    if (!opt_map.Parse(argc, argv)) ShowUsage(argv[0]);

    $ToolClass$::ToolFunc_t targefunc = target->func;

    $ToolImplClass$ tool;

    if (0 != (tool.*targefunc)(opt_map)) ShowUsage(argv[0]);

    return 0;
}

)";

