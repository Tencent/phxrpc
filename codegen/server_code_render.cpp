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

#include "server_code_render.h"
#include "server_template.h"

#include "name_render.h"
#include "syntax_tree.h"
#include "code_utils.h"

#include <cstring>

using namespace phxrpc;
using namespace std;


ServerCodeRender::ServerCodeRender(NameRender &name_render)
        : name_render_(name_render) {
}

ServerCodeRender::~ServerCodeRender() {
}

void ServerCodeRender::GenerateServerConfigHpp(SyntaxTree *stree, FILE *write) {
    char file_name[128]{'\0'};
    name_render_.GetServerConfigFileName(stree->GetName(), file_name, sizeof(file_name));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false);

    fprintf(write, "/* %s.h\n", file_name);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    char class_name[128]{'\0'};
    name_render_.GetServerConfigClassName(stree->GetName(), class_name, sizeof(class_name));

    string content = PHXRPC_EPOLL_SERVER_CONFIG_HPP_TEMPLATE;

    StrTrim(&content);
    StrReplaceAll(&content, "$ServerConfigClass$", class_name);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerConfigCpp(SyntaxTree *stree, FILE *write) {
    char file_name[128]{'\0'};
    name_render_.GetServerConfigFileName(stree->GetName(), file_name, sizeof(file_name));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", file_name);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    char class_name[128]{'\0'};
    char message_file[128]{'\0'};
    name_render_.GetServerConfigClassName(stree->GetName(), class_name, sizeof(class_name));
    name_render_.GetMessageFileName(stree->proto_file(), message_file, sizeof(message_file));

    string content = PHXRPC_EPOLL_SERVER_CONFIG_CPP_TEMPLATE;

    string package_name_expression = "\"" + string(stree->package_name()) + "\"";

    {
        string message_name;
        for (auto itr : *(stree->func_list())) {
            if (string(itr.GetReq()->GetType()).find(stree->package_name()) != string::npos) {
                message_name = itr.GetReq()->GetType();
                break;
            } else if (string(itr.GetResp()->GetType()).find(stree->package_name()) != string::npos) {
                message_name = itr.GetResp()->GetType();
                break;
            }
        }
        if (message_name != "") {
            int package_name_len = strlen(stree->package_name());
            message_name = message_name.substr(package_name_len + 1,
                                               message_name.size() - package_name_len - 1);
            package_name_expression = "\n                " + SyntaxTree::Pb2CppPackageName(stree->package_name()) + "::" + message_name
                           + "::default_instance().GetDescriptor()->file()->package().c_str()";
        }
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$MessageFile$", message_file);
    StrReplaceAll(&content, "$PackageNameExpression$", package_name_expression);
    StrReplaceAll(&content, "$ServerConfigClass$", class_name);
    StrReplaceAll(&content, "$ServerConfigFile$", file_name);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerMainCpp(SyntaxTree *stree, FILE *write, const bool is_uthread_mode) {
    char svrfile[128]{'\0'};
    name_render_.GetServerMainFileName(stree->GetName(), svrfile, sizeof(svrfile));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", svrfile);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    char dispatcher_class[128]{'\0'}, dispatcher_file[128]{'\0'};
    char service_impl_class[128]{'\0'}, service_impl_file[128]{'\0'};
    char server_config_class[128]{'\0'}, server_config_file[128]{'\0'};

    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_class, sizeof(dispatcher_class));
    name_render_.GetDispatcherFileName(stree->GetName(), dispatcher_file, sizeof(dispatcher_file));
    name_render_.GetServiceImplClassName(stree->GetName(), service_impl_class, sizeof(service_impl_class));
    name_render_.GetServiceImplFileName(stree->GetName(), service_impl_file, sizeof(service_impl_file));
    name_render_.GetServerConfigClassName(stree->GetName(), server_config_class, sizeof(server_config_class));
    name_render_.GetServerConfigFileName(stree->GetName(), server_config_file, sizeof(server_config_file));

    string content;
    if (!is_uthread_mode) {
        content = PHXRPC_EPOLL_SERVER_MAIN_TEMPLATE;
    } else {
        content = PHXRPC_EPOLL_UTHREAD_SERVER_MAIN_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$DispatcherFile$", dispatcher_file);
    StrReplaceAll(&content, "$DispatcherClass$", dispatcher_class);
    StrReplaceAll(&content, "$ServiceImplFile$", service_impl_file);
    StrReplaceAll(&content, "$ServiceImplClass$", service_impl_class);
    StrReplaceAll(&content, "$ServerConfigFile$", server_config_file);
    StrReplaceAll(&content, "$ServerConfigClass$", server_config_class);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerEtc(SyntaxTree *stree, FILE *write, const bool is_uthread_mode) {
    char etc_file[128]{'\0'};
    name_render_.GetServerEtcFileName(stree->GetName(), etc_file, sizeof(etc_file));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false, "#");

    fprintf(write, "# %s\n", etc_file);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    string content;
    if (!is_uthread_mode) {
        content = PHXRPC_EPOLL_SERVER_ETC_TEMPLATE;
    } else {
        content = PHXRPC_EPOLL_UTHREAD_SERVER_ETC_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$PbPackageName$", stree->package_name());

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

void ServerCodeRender::GenerateMakefile(SyntaxTree *stree,
        const string &mk_dir_path, FILE *write, const bool is_uthread_mode) {
    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false, "#");

    fprintf(write, "# Makefile\n");
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    char dispatcher_file[128]{'\0'}, service_file[128]{'\0'}, service_impl_file[128]{'\0'};
    char server_config_file[128]{'\0'}, server_main_file[128]{'\0'};
    char message_file[128]{'\0'}, stub_file[128]{'\0'}, client_file[128]{'\0'};
    char tool_file[128]{'\0'}, tool_impl_file[128]{'\0'}, tool_main_file[128]{'\0'};

    name_render_.GetDispatcherFileName(stree->GetName(), dispatcher_file, sizeof(dispatcher_file));
    name_render_.GetServiceImplFileName(stree->GetName(), service_impl_file, sizeof(service_impl_file));
    name_render_.GetServiceFileName(stree->GetName(), service_file, sizeof(service_file));

    name_render_.GetServerConfigFileName(stree->GetName(), server_config_file, sizeof(server_config_file));
    name_render_.GetServerMainFileName(stree->GetName(), server_main_file, sizeof(server_main_file));

    name_render_.GetToolImplFileName(stree->GetName(), tool_impl_file, sizeof(tool_impl_file));
    name_render_.GetToolFileName(stree->GetName(), tool_file, sizeof(tool_file));
    name_render_.GetToolMainFileName(stree->GetName(), tool_main_file, sizeof(tool_main_file));

    name_render_.GetClientFileName(stree->GetName(), client_file, sizeof(client_file));
    name_render_.GetStubFileName(stree->GetName(), stub_file, sizeof(stub_file));
    name_render_.GetMessageFileName(stree->proto_file(), message_file, sizeof(message_file));

    string content;
    if (!is_uthread_mode) {
        content = PHXRPC_SERVER_MAKEFILE_TEMPLATE;
    } else {
        content = PHXRPC_UTHREAD_SERVER_MAKEFILE_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$PhxRPCMKDir$", mk_dir_path);
    StrReplaceAll(&content, "$DispatcherFile$", dispatcher_file);
    StrReplaceAll(&content, "$ServiceFile$", service_file);
    StrReplaceAll(&content, "$ServiceImplFile$", service_impl_file);

    StrReplaceAll(&content, "$ServerConfigFile$", server_config_file);
    StrReplaceAll(&content, "$ServerMainFile$", server_main_file);

    StrReplaceAll(&content, "$MessageFile$", message_file);
    StrReplaceAll(&content, "$StubFile$", stub_file);
    StrReplaceAll(&content, "$ClientFile$", client_file);

    StrReplaceAll(&content, "$ToolFile$", tool_file);
    StrReplaceAll(&content, "$ToolImplFile$", tool_impl_file);
    StrReplaceAll(&content, "$ToolMainFile$", tool_main_file);

    StrReplaceAll(&content, "$ProtoFile$", stree->proto_file());

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n\n");
}

