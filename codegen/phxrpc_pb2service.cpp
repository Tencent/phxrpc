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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <map>
#include <string>

#include "syntax_tree.h"

#include "name_render.h"
#include "service_code_render.h"
#include "proto_utils.h"

using namespace phxrpc;
using namespace std;

void PrintHelp(const char * program) {
    printf("\n");
    printf("PHXRPC ProtoBuf tool\n");
    printf("\n");
    printf("%s <-f Profo file> <-d destination file dir> [-v]\n", program);
    printf(" Usage: -f <Proto file>            # Proto File\n");
    printf("        -d <dir>                   # destination file dir\n");
    printf("        -I <dir>                   # include path dir\n");
    printf("        -v                         # print this screen\n");
    printf("\n");
    return;
}

void Proto2Service(const char * program, const char * pb_file, const char * dir_path,
        const std::vector<std::string> & include_list, const bool is_uthread_mode) {
    std::map<std::string, bool> parsed_file_map;
    SyntaxTree syntax_tree;

    int ret = ProtoUtils::Parse(pb_file, &syntax_tree, &parsed_file_map, include_list);

    if (0 != ret) {
        printf("parse proto file fail, please check error log\n");
        return;
    }

    // printf( "parse(%s) = %d\n", pb_file, ret );

    NameRender name_render(syntax_tree.GetPrefix());
    ServiceCodeRender codeRender(name_render);

    char filename[256] = { 0 }, tmp[256] = { 0 };

    // [xx]service.h
    {
        name_render.GetServiceFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);
        FILE * fp = fopen(filename, "w");
        assert(NULL != fp);
        codeRender.GenerateServiceHpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]service.cpp
    {
        name_render.GetServiceFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);
        FILE * fp = fopen(filename, "w");
        assert(NULL != fp);
        codeRender.GenerateServiceCpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]serviceimpl.h
    {
        name_render.GetServiceImplFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE * fp = fopen(filename, "w");
            assert(NULL != fp);
            codeRender.GenerateServiceImplHpp(&syntax_tree, fp, is_uthread_mode);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]serviceimpl.cpp
    {
        name_render.GetServiceImplFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE * fp = fopen(filename, "w");
            assert(NULL != fp);
            codeRender.GenerateServiceImplCpp(&syntax_tree, fp, is_uthread_mode);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]dispatcher.h
    {
        name_render.GetDispatcherFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);

        FILE * fp = fopen(filename, "w");
        assert(NULL != fp);
        codeRender.GenerateDispatcherHpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]dispatcher.cpp
    {
        name_render.GetDispatcherFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        FILE * fp = fopen(filename, "w");
        assert(NULL != fp);
        codeRender.GenerateDispatcherCpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }
}

int main(int argc, char * argv[]) {
    const char * pb_file = NULL;
    const char * dir_path = NULL;

    extern char *optarg;
    int c;
    std::vector<std::string> include_list;
    char real_path[1024] = {0};
    char * rp = nullptr;
    bool is_uthread_mode = false;

    while ((c = getopt(argc, argv, "f:d:I:uv")) != EOF) {
        switch (c) {
            case 'f':
                pb_file = optarg;
                break;
            case 'd':
                dir_path = optarg;
                break;
            case 'I':
                rp = realpath(optarg, real_path);
                if (rp != nullptr) {
                    include_list.push_back(rp);
                }
                break;
            case 'u':
                is_uthread_mode = true;
                break;
            default:
                PrintHelp(argv[0]);
                exit(-1);
                break;
        }
    }

    if (NULL == pb_file || NULL == dir_path) {
        printf("Invalid arguments\n");

        PrintHelp(argv[0]);
        exit(0);
    }

    if (0 != access(dir_path, R_OK | W_OK | X_OK)) {
        printf("Invalid dir: %s, errno %d, %s\n\n", dir_path, errno,
                     strerror(errno));
        PrintHelp(argv[0]);
        exit(0);
    }

    char path[128] = { 0 };
    strncpy(path, dir_path, sizeof(path));
    if ('/' == path[strlen(path) - 1]) {
        path[strlen(path) - 1] = '\0';
    }

    Proto2Service(argv[0], pb_file, path, include_list, is_uthread_mode);

    printf("\n");

    return 0;
}

