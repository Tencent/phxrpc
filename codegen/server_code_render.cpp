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

ServerCodeRender::ServerCodeRender(NameRender & name_render)
        : name_render_(name_render) {
}

ServerCodeRender::~ServerCodeRender() {
}

void ServerCodeRender::GenerateServerConfigHpp(SyntaxTree * stree, FILE * write) {
    char filename[128] = { 0 };
    name_render_.GetServerConfigFileName(stree->GetName(), filename, sizeof(filename));

    std::string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    char classname[128] = { 0 };
    name_render_.GetServerConfigClasname(stree->GetName(), classname, sizeof(classname));

    std::string content = PHXRPC_EPOLL_SERVER_CONFIG_HPP_TEMPLATE;

    StrTrim(&content);
    StrReplaceAll(&content, "$ServerConfigClass$", classname);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerConfigCpp(SyntaxTree * stree, FILE * write) {
    char filename[128] = { 0 };
    name_render_.GetServerConfigFileName(stree->GetName(), filename, sizeof(filename));

    std::string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    char classname[128] = { 0 };
    char message_file[128] = {0};
    name_render_.GetServerConfigClasname(stree->GetName(), classname, sizeof(classname));
    name_render_.GetMessageFileName(stree->GetProtoFile(), message_file, sizeof(message_file));

    std::string content = PHXRPC_EPOLL_SERVER_CONFIG_CPP_TEMPLATE;

    std::string package_name = "\"" + std::string(stree->GetPackageName()) + "\"";

    {
        std::string message_name = "";
        for( auto itr : *(stree->GetFuncList()) ) {
            if ( std::string(itr.GetReq()->GetType()).find( stree->GetPackageName() ) != std::string::npos ) {
                message_name = itr.GetReq()->GetType();
                break;
            } else if ( std::string(itr.GetResp()->GetType()).find( stree->GetPackageName() ) != std::string::npos ) {
                message_name = itr.GetResp()->GetType();
                break;
            }
        }
        if( message_name != "" ) {
            int package_name_len = strlen(stree->GetPackageName());
            message_name = message_name.substr( package_name_len + 1, message_name.size() - package_name_len - 1 );
            package_name = "\n" + std::string(stree->GetPackageName()) + "::" + message_name 
                           + "::default_instance().GetDescriptor()->file()->package().c_str()";
        }
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$MessageFile$", message_file);
    StrReplaceAll(&content, "$PackageName$", package_name);
    StrReplaceAll(&content, "$ServerConfigClass$", classname);
    StrReplaceAll(&content, "$ServerConfigFile$", filename);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerMainCpp(SyntaxTree * stree, FILE * write, const bool is_uthread_mode) {
    char svrfile[128] = { 0 };
    name_render_.GetServerMainFileName(stree->GetName(), svrfile, sizeof(svrfile));

    std::string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", svrfile);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    char dispatcher_calss[128] = { 0 }, dispatcher_file[128] = { 0 };
    char service_impl_class[128] = { 0 }, service_impl_file[128] = { 0 };
    char server_config_class[128] = { 0 }, server_config_file[128] = { 0 };

    name_render_.GetDispatcherClasname(stree->GetName(), dispatcher_calss, sizeof(dispatcher_calss));
    name_render_.GetDispatcherFileName(stree->GetName(), dispatcher_file, sizeof(dispatcher_file));
    name_render_.GetServiceImplClasname(stree->GetName(), service_impl_class, sizeof(service_impl_class));
    name_render_.GetServiceImplFileName(stree->GetName(), service_impl_file, sizeof(service_impl_file));
    name_render_.GetServerConfigClasname(stree->GetName(), server_config_class, sizeof(server_config_class));
    name_render_.GetServerConfigFileName(stree->GetName(), server_config_file, sizeof(server_config_file));

    std::string content;
    if (!is_uthread_mode) {
        content = PHXRPC_EPOLL_SERVER_MAIN_TEMPLATE;
    } else {
        content = PHXRPC_EPOLL_UTHREAD_SERVER_MAIN_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$DispatcherFile$", dispatcher_file);
    StrReplaceAll(&content, "$DispatcherClass$", dispatcher_calss);
    StrReplaceAll(&content, "$ServiceImplFile$", service_impl_file);
    StrReplaceAll(&content, "$ServiceImplClass$", service_impl_class);
    StrReplaceAll(&content, "$ServerConfigFile$", server_config_file);
    StrReplaceAll(&content, "$ServerConfigClass$", server_config_class);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
}

void ServerCodeRender::GenerateServerEtc(SyntaxTree * stree, FILE * write, const bool is_uthread_mode) {
    char etcfile[128] = { 0 };
    name_render_.GetServerEtcFileName(stree->GetName(), etcfile, sizeof(etcfile));

    std::string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false, "#");

    fprintf(write, "# %s\n", etcfile);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    std::string content;
    if (!is_uthread_mode) {
        content = PHXRPC_EPOLL_SERVER_ETC_TEMPLATE;
    } else {
        content = PHXRPC_EPOLL_UTHREAD_SERVER_ETC_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$PackageName$", stree->GetPackageName() );

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
}

void ServerCodeRender::GenerateMakefile(SyntaxTree * stree, const std::string & mk_dir_path, FILE * write, const bool is_uthread_mode) {
    std::string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false, "#");

    fprintf(write, "# Makefile\n");
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    char dispatcher_file[128] = { 0 }, service_file[128] = { 0 }, service_impl_file[128] = { 0 };
    char server_config_file[128] = { 0 }, server_main_file[128] = { 0 };
    char message_file[128] = { 0 }, stub_file[128] = { 0 }, client_file[128] = { 0 };
    char tool_file[128] = { 0 }, tool_impl_file[128] = { 0 }, tool_main_file[128] = { 0 };

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
    name_render_.GetMessageFileName(stree->GetProtoFile(), message_file, sizeof(message_file));

    std::string content;
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

    StrReplaceAll(&content, "$ProtoFile$", stree->GetProtoFile());

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n\n");
}

