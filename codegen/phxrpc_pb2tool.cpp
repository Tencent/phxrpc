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

#include <errno.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "syntax_tree.h"

#include "tool_code_render.h"
#include "proto_utils.h"
#include "name_render.h"


using namespace phxrpc;
using namespace std;


void PrintHelp(const char *program) {
    printf("\n");
    printf("PhxRPC ProtoBuf tool\n");
    printf("\n");
    printf("%s <-f profo file> <-d destination file dir> [-p] [-v]\n", program);
    printf(" Usage: -f <proto file>             # proto file\n");
    printf("        -d <dir>                    # destination file dir\n");
    printf("        -I <dir>                    # include path dir\n");
    printf("        -p <protocol>               # http or mqtt\n");
    printf("        -v                          # print this screen\n");
    printf("\n");

    return;
}

void Proto2Tool(const char *program, const char *pb_file, const char *dir_path,
                const vector<string> &include_list, const bool mqtt) {
    SyntaxTree syntax_tree;

    int ret = ProtoUtils::Parse(pb_file, &syntax_tree, include_list);

    if (0 != ret) {
        printf("parse proto file fail, please check error log\n");
        return;
    }

    // printf( "parse(%s) = %d\n", pb_file, ret );

    // mqtt
    SyntaxFuncVector mqtt_funcs;

    if (mqtt) {
        SyntaxFunc connect_func;
        connect_func.SetCmdID(-201);
        connect_func.SetName("PhxMqttConnect");
        connect_func.GetReq()->SetType("phxrpc::MqttConnectPb");
        connect_func.GetResp()->SetType("phxrpc::MqttConnackPb");
        mqtt_funcs.push_back(connect_func);

        SyntaxFunc publish_func;
        publish_func.SetCmdID(-202);
        publish_func.SetName("PhxMqttPublish");
        publish_func.SetOptString("s:");
        publish_func.SetUsage("-s <string>");
        publish_func.GetReq()->SetType("phxrpc::MqttPublishPb");
        publish_func.GetResp()->SetType("phxrpc::MqttPubackPb");
        mqtt_funcs.push_back(publish_func);

        SyntaxFunc disconnect_func;
        disconnect_func.SetCmdID(-207);
        disconnect_func.SetName("PhxMqttDisconnect");
        disconnect_func.GetReq()->SetType("phxrpc::MqttDisconnectPb");
        disconnect_func.GetResp()->SetType("");
        mqtt_funcs.push_back(disconnect_func);
    }

    NameRender name_render(syntax_tree.GetPrefix());
    ToolCodeRender code_render(name_render);

    char filename[256]{0}, tmp[256]{0};

    // [xx]tool.h
    {
        name_render.GetToolFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);
        FILE *fp{fopen(filename, "w")};
        assert(nullptr != fp);
        code_render.GenerateToolHpp(&syntax_tree, mqtt_funcs, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]tool.cpp
    {
        name_render.GetToolFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        FILE *fp{fopen(filename, "w")};
        assert(nullptr != fp);
        code_render.GenerateToolCpp(&syntax_tree, mqtt_funcs, fp);
        fclose(fp);

        printf("\n%s: Build %s file ... done\n", program, filename);
    }

    // [xx]toolimpl.h
    {
        name_render.GetToolImplFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.h", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            assert(nullptr != fp);
            code_render.GenerateToolImplHpp(&syntax_tree, mqtt_funcs, fp);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]toolimpl.cpp
    {
        name_render.GetToolImplFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            assert(nullptr != fp);
            code_render.GenerateToolImplCpp(&syntax_tree, mqtt_funcs, fp);
            fclose(fp);

            printf("\n%s: Build %s file ... done\n", program, filename);
        } else {
            printf("\n%s: %s is exist, skip\n", program, filename);
        }
    }

    // [xx]toolmain.cpp
    {
        name_render.GetToolMainFileName(syntax_tree.GetName(), tmp, sizeof(tmp));
        snprintf(filename, sizeof(filename), "%s/%s.cpp", dir_path, tmp);

        if (0 != access(filename, F_OK)) {
            FILE *fp{fopen(filename, "w")};
            assert(nullptr != fp);
            code_render.GenerateToolMainCpp(&syntax_tree, fp);
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
    bool mqtt{false};

    while (EOF != (c = getopt(argc, argv, "f:d:I:p:v"))) {
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
            case 'p':
                if (0 == strcasecmp(optarg, "mqtt"))
                mqtt = true;
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

    Proto2Tool(argv[0], pb_file, path, include_list, mqtt);

    printf("\n");

    return 0;
}

