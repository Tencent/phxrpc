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
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <unistd.h>

#include "syntax_tree.h"
#include "name_render.h"

#include "client_code_render.h"
#include "proto_utils.h"


using namespace phxrpc;
using namespace std;


void PrintHelp(const char *program) {
    printf("\n");
    printf("PhxRPC ProtoBuf tool\n");
    printf("\n");
    printf("%s <-f profo file> <-d destination file dir> [-v]\n", program);
    printf(" Usage: -f <proto file>             # proto file\n");
    printf("        -d <dir>                    # destination file dir\n");
    printf("        -I <dir>                    # include path dir\n");
    printf("        -u <uthread mode>           # uthread mode\n");
    printf("        -v                          # print this screen\n");
    printf("\n");

    return;
}

void Proto2Client(const char *program, const char *pb_file,
                  const char *dir_path, const vector<string> &include_list,
                  const bool is_uthread_mode) {
    SyntaxTree syntax_tree;
    int ret{ProtoUtils::Parse(pb_file, &syntax_tree, include_list)};
    if (0 != ret) {
        printf("parse proto file fail, please check error log\n");
        return;
    }

    NameRender name_render(syntax_tree.prefix());
    ClientCodeRender code_render(name_render);

    char filename[256]{0}, tmp[256]{0};

    // [xx]stub.h
    {
        name_render.GetStubFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);

        FILE *fp{fopen(filename, "w")};
        code_render.GenerateStubHpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]stub.cpp
    {
        name_render.GetStubFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        FILE *fp{fopen(filename, "w")};
        code_render.GenerateStubCpp(&syntax_tree, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }


    // [xx]client.h
    {
        name_render.GetClientFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);

        char name4hpp[256]{0};
        snprintf(name4hpp, sizeof(name4hpp), "%s/%s.hpp", dir_path, tmp);

        if (0 != access(name4hpp, F_OK)) {
            if (0 != access(filename, F_OK)) {
                FILE *fp{fopen(filename, "w")};
                code_render.GenerateClientHpp(&syntax_tree, fp, false);
                fclose(fp);

                printf("\n%s: Build %s file ... done\n", program, filename);
            } else {
                printf("\n%s: %s is exist, skip\n", program, filename);
            }
        } else {
            printf("\n%s: %s is exist, skip\n", program, name4hpp);
        }
    }

    // [xx]client_uthread.h
    if (is_uthread_mode)
    {
        name_render.GetClientFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s_uthread.h", dir_path, tmp);

        char name4hpp[256]{0};
        snprintf(name4hpp, sizeof(name4hpp), "%s/%s_uthread.hpp", dir_path, tmp);

        if (0 != access(name4hpp, F_OK)) {
            if (0 != access(filename, F_OK)) {
                FILE *fp{fopen(filename, "w")};
                code_render.GenerateClientHpp(&syntax_tree, fp, is_uthread_mode);
                fclose(fp);

                printf("\n%s: Build %s file ... done\n", program, filename);
            } else {
                printf("\n%s: %s is exist, skip\n", program, filename);
            }
        } else {
            printf("\n%s: %s is exist, skip\n", program, name4hpp);
        }
    }

    // [xx]client.cpp
    {
        name_render.GetClientFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            code_render.GenerateClientCpp(&syntax_tree, fp, false);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]client_uthread.cpp
    if (is_uthread_mode)
    {
        name_render.GetClientFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s_uthread.cpp", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            code_render.GenerateClientCpp(&syntax_tree, fp, is_uthread_mode);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]_client.conf
    {
        name_render.GetClientEtcFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            code_render.GenerateClientEtc(&syntax_tree, fp);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }
}

int main(int argc, char **argv) {
    const char *pb_file{nullptr};
    const char *dir_path{nullptr};

    extern char *optarg;
    int c;
    vector<string> include_list;
    char real_path[1024]{0};
    char *rp{nullptr};
    bool is_uthread_mode{false};

    while (EOF != (c = getopt(argc, argv, "f:d:I:uv"))) {
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

    if (nullptr == pb_file || nullptr == dir_path) {
        printf("Invalid arguments\n");

        PrintHelp(argv[0]);
        exit(0);
    }

    if (0 != access(dir_path, R_OK | W_OK | X_OK)) {
        printf("Invalid dir: %s, errno %d, %s\n\n", dir_path, errno, strerror(errno));
        PrintHelp(argv[0]);
        exit(0);
    }

    char path[128]{0};
    strncpy(path, dir_path, sizeof(path));
    if ('/' == path[strlen(path) - 1]) {
        path[strlen(path) - 1] = '\0';
    }

    Proto2Client(argv[0], pb_file, path, include_list, is_uthread_mode);

    printf("\n");

    return 0;
}

