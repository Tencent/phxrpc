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

#include "client_code_render.h"

#include "code_utils.h"

#include "client_template.h"

#include "syntax_tree.h"
#include "name_render.h"

#include <cstring>
#include <string>


using namespace phxrpc;
using namespace std;


ClientCodeRender::ClientCodeRender(NameRender &name_render)
        : name_render_(name_render) {
}

ClientCodeRender::~ClientCodeRender() {
}

void ClientCodeRender::GenerateStubHpp(SyntaxTree *stree, FILE *write) {
    char file_name[128]{'\0'};
    name_render_.GetStubFileName(stree->GetName(), file_name, sizeof(file_name));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->proto_file(), &buffer);

    fprintf(write, "/* %s.h\n", file_name);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->proto_file(), file_name, sizeof(file_name));
    fprintf(write, "#include \"%s.h\"\n", file_name);

    fprintf(write, "\n");
    fprintf(write, "\n");

    fprintf(write, "namespace phxrpc {\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "class BaseMessageHandlerFactory;\n");
    fprintf(write, "class BaseTcpStream;\n");
    fprintf(write, "class ClientMonitor;\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "}\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    char class_name[128]{'\0'};
    name_render_.GetStubClassName(stree->GetName(), class_name, sizeof(class_name));

    {
        fprintf(write, "class %s {\n", class_name);
        fprintf(write, "  public:\n");
        fprintf(write, "    %s(phxrpc::BaseTcpStream &socket, phxrpc::ClientMonitor &client_monitor,\n"
                "            phxrpc::BaseMessageHandlerFactory &msg_handler_factory);\n", class_name);
        fprintf(write, "    virtual ~%s();\n", class_name);
        fprintf(write, "\n");

        fprintf(write, "    void set_keep_alive(const bool keep_alive);\n\n");

        auto flist(stree->func_list());
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            string buffer;
            GetStubFuncDeclaration(stree, &(*fit), 1, &buffer);
            fprintf(write, "    %s;\n", buffer.c_str());
        }
        fprintf(write, "\n");

        fprintf(write, "  private:\n");
        fprintf(write, "    phxrpc::BaseTcpStream &socket_;\n");
        fprintf(write, "    phxrpc::ClientMonitor &client_monitor_;\n");
        fprintf(write, "    bool keep_alive_{false};\n");
        fprintf(write, "    phxrpc::BaseMessageHandlerFactory &msg_handler_factory_;\n");

        fprintf(write, "};\n");
        fprintf(write, "\n");

    }
}

void ClientCodeRender::GetStubFuncDeclaration(const SyntaxTree *const stree, const SyntaxFunc *const func,
                                              int is_header, string *result) {
    char class_name[128]{'\0'}, type_name[128]{'\0'};

    name_render_.GetStubClassName(stree->GetName(), class_name, sizeof(class_name));

    if (is_header) {
        phxrpc::StrAppendFormat(result, "int %s(", func->GetName());
    } else {
        phxrpc::StrAppendFormat(result, "int %s::%s(", class_name, func->GetName());
    }

    name_render_.GetMessageClassName(func->GetReq()->GetType(), type_name, sizeof(type_name));
    phxrpc::StrAppendFormat(result, "const %s &req, ", type_name);

    name_render_.GetMessageClassName(func->GetResp()->GetType(), type_name, sizeof(type_name));
    phxrpc::StrAppendFormat(result, "%s *resp", type_name);

    phxrpc::StrAppendFormat(result, ")");
}

void ClientCodeRender::GenerateStubCpp(SyntaxTree *stree, FILE *write) {
    char file_name[128]{'\0'};
    name_render_.GetStubFileName(stree->GetName(), file_name, sizeof(file_name));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->proto_file(), &buffer);

    fprintf(write, "/* %s.cpp\n", file_name);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", file_name);
    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/http.h\"\n");
    fprintf(write, "#include \"phxrpc/network.h\"\n");
    fprintf(write, "#include \"phxrpc/rpc.h\"\n");
    fprintf(write, "\n");
    fprintf(write, "\n");

    char class_name[128]{'\0'};
    name_render_.GetStubClassName(stree->GetName(), class_name, sizeof(class_name));

    {
        fprintf(write, "%s::%s(phxrpc::BaseTcpStream &socket, phxrpc::ClientMonitor &client_monitor,\n"
                "        phxrpc::BaseMessageHandlerFactory &msg_handler_factory)\n",
                class_name, class_name);

        fprintf(write, "        : socket_(socket), client_monitor_(client_monitor),\n"
                "          msg_handler_factory_(msg_handler_factory) {\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");

        fprintf(write, "%s::~%s() {\n", class_name, class_name);
        fprintf(write, "}\n");
        fprintf(write, "\n");

        fprintf(write, "void %s::set_keep_alive(const bool keep_alive) {\n", class_name);
        fprintf(write, "    keep_alive_ = keep_alive;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");

        auto flist(stree->func_list());
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            GenerateStubFunc(stree, &(*fit), write);
        }

    }
}

void ClientCodeRender::GenerateStubFunc(const SyntaxTree *const stree,
                                        const SyntaxFunc *const func,
                                        FILE *write) {
    string buffer;

    GetStubFuncDeclaration(stree, func, 0, &buffer);

    fprintf(write, "%s {\n", buffer.c_str());

    fprintf(write, "    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);\n");
    fprintf(write, "    caller.set_uri(\"/%s/%s\", %d);\n",
            SyntaxTree::Pb2UriPackageName(stree->package_name()).c_str(),
            func->GetName(), func->GetCmdID());
    fprintf(write, "    caller.set_keep_alive(keep_alive_);\n");
    fprintf(write, "    return caller.Call(req, resp);\n");

    fprintf(write, "}\n");
    fprintf(write, "\n");
}

void ClientCodeRender::GenerateClientHpp(SyntaxTree *stree,
                                         FILE *write, const bool is_uthread_mode) {
    char file_name[128]{'\0'};
    name_render_.GetClientFileName(stree->GetName(), file_name, sizeof(file_name));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->proto_file(), &buffer, false);

    if (is_uthread_mode) {
        fprintf(write, "/* %s_uthread.h\n", file_name);
    } else {
        fprintf(write, "/* %s.h\n", file_name);
    }
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    string declarations;
    {
        auto flist(stree->func_list());
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            string buffer;
            GetClienfuncDeclaration(stree, &(*fit), 1, &buffer, is_uthread_mode);
            declarations.append("    ").append(buffer).append(";\n");

            if (0 == strcmp(fit->GetName(), "PHXEcho")) {
                SyntaxFunc echo_func = *fit;
                echo_func.SetName("PHXBatchEcho");
                string buffer;
                GetClienfuncDeclaration(stree, &echo_func, 1, &buffer, is_uthread_mode);
                declarations.append("    ").append(buffer).append(";\n");
            }
        }
    }

    char client_class[128]{'\0'}, message_file[128]{'\0'};
    char client_class_lower[128]{'\0'};
    name_render_.GetClientClassName(stree->GetName(), client_class, sizeof(client_class));
    name_render_.GetClientClassNameLower(stree->GetName(), client_class_lower, sizeof(client_class_lower));
    name_render_.GetMessageFileName(stree->proto_file(), message_file, sizeof(message_file));

    string client_class_str(client_class);
    string client_class_lower_str(client_class_lower);
    if (is_uthread_mode) {
        client_class_str += "UThread";
        client_class_lower_str += "uthread";
    }

    string content;
    if (!is_uthread_mode) {
        content = PHXRPC_CLIENT_HPP_TEMPLATE;
    } else {
        content = PHXRPC_UTHREAD_CLIENT_HPP_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$MessageFile$", message_file);
    StrReplaceAll(&content, "$ClientClass$", client_class_str.c_str());
    StrReplaceAll(&content, "$ClientClassLower$", client_class_lower_str.c_str());
    StrReplaceAll(&content, "$ClientClassFuncDeclarations$", declarations);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

void ClientCodeRender::GenerateClientCpp(SyntaxTree *stree,
                                         FILE *write, const bool is_uthread_mode) {
    char client_class[128]{'\0'}, client_file[128]{'\0'};
    char client_class_lower[128]{'\0'};
    char stub_class[128]{'\0'}, stub_file[128]{'\0'};
    name_render_.GetClientClassName(stree->GetName(), client_class, sizeof(client_class));
    name_render_.GetClientClassNameLower(stree->GetName(), client_class_lower, sizeof(client_class_lower));
    name_render_.GetClientFileName(stree->GetName(), client_file, sizeof(client_file));
    name_render_.GetStubClassName(stree->GetName(), stub_class, sizeof(stub_class));
    name_render_.GetStubFileName(stree->GetName(), stub_file, sizeof(stub_file));

    string client_class_str(client_class);
    string client_class_lower_str(client_class_lower);
    if (is_uthread_mode) {
        client_class_str += "UThread";
        client_class_lower_str += "uthread";
    }

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->proto_file(), &buffer, false);

    if (is_uthread_mode) {
        fprintf(write, "/* %s_uthread.cpp\n", client_file);
    } else {
        fprintf(write, "/* %s.cpp\n", client_file);
    }
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");

    fprintf(write, "\n");

    string functions;

    {
        auto flist(stree->func_list());
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            string buffer;
            GetClienfuncDeclaration(stree, &(*fit), 0, &buffer, is_uthread_mode);

            functions.append(buffer).append("\n");

            string content;
            if (!is_uthread_mode) {
                content = PHXRPC_CLIENT_FUNC_TEMPLATE;
            } else {
                content = PHXRPC_UTHREAD_CLIENT_FUNC_TEMPLATE;
            }

            StrTrim(&content);
            StrReplaceAll(&content, "$ClientClass$", client_class_str.c_str());
            StrReplaceAll(&content, "$ClientClassLower$", client_class_lower_str.c_str());
            StrReplaceAll(&content, "$StubClass$", stub_class);
            string func_string(fit->GetName());
            func_string += "(req, resp)";
            StrReplaceAll(&content, "$Func$", func_string);

            functions.append(content).append("\n\n");

            if (0 == strcmp(fit->GetName(), "PHXEcho")) {
                SyntaxFunc echo_func = *fit;
                echo_func.SetName("PHXBatchEcho");

                string buffer;
                GetClienfuncDeclaration(stree, &echo_func, 0, &buffer, is_uthread_mode);

                functions.append(buffer).append("\n");

                string content = PHXRPC_BATCH_CLIENT_FUNC_TEMPLATE;

                StrTrim(&content);
                StrReplaceAll(&content, "$ClientClass$", client_class_str.c_str());
                StrReplaceAll(&content, "$ClientClassLower$", client_class_lower_str.c_str());
                StrReplaceAll(&content, "$StubClass$", stub_class);
                string echo_func_string(echo_func.GetName());
                echo_func_string += "(req, resp)";
                StrReplaceAll(&content, "$Func$", echo_func_string);

                functions.append(content).append("\n\n");
            }
        }
    }

    string content;
    if (!is_uthread_mode) {
        content = PHXRPC_CLIENT_CPP_TEMPLATE;
    } else {
        content = PHXRPC_UTHREAD_CLIENT_CPP_TEMPLATE;
    }

    StrTrim(&content);
    StrReplaceAll(&content, "$PbPackageName$", stree->package_name());
    StrReplaceAll(&content, "$ClientFile$", client_file);
    StrReplaceAll(&content, "$StubFile$", stub_file);
    StrReplaceAll(&content, "$ClientClass$", client_class_str);
    StrReplaceAll(&content, "$ClientClassLower$", client_class_lower_str.c_str());
    StrReplaceAll(&content, "$ClientClassFuncs$", functions);

    fprintf(write, "%s", content.c_str());
}

void ClientCodeRender::GetClienfuncDeclaration(const SyntaxTree *const stree,
                                               const SyntaxFunc *const func,
                                               const int is_header, string *result,
                                               const bool is_uthread_mode) {
    char class_name[128]{'\0'}, type_name[128]{'\0'};

    name_render_.GetClientClassName(stree->GetName(), class_name, sizeof(class_name));
    string class_name_str(class_name);
    if (is_uthread_mode) {
        class_name_str += "UThread";
    }

    if (is_header) {
        phxrpc::StrAppendFormat(result, "int %s(", func->GetName());
    } else {
        phxrpc::StrAppendFormat(result, "int %s::%s(", class_name_str.c_str(), func->GetName());
    }

    name_render_.GetMessageClassName(func->GetReq()->GetType(), type_name, sizeof(type_name));
    phxrpc::StrAppendFormat(result, "const %s &req", type_name);

    name_render_.GetMessageClassName(func->GetResp()->GetType(), type_name, sizeof(type_name));
    phxrpc::StrAppendFormat(result, ", %s *resp", type_name);

    phxrpc::StrAppendFormat(result, ")");
}

void ClientCodeRender::GenerateClientEtc(SyntaxTree *stree, FILE *write) {
    char etc_file[128]{'\0'};
    name_render_.GetClientEtcFileName(stree->GetName(), etc_file, sizeof(etc_file));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->proto_file(), &buffer, false, "#");

    fprintf(write, "# %s\n", etc_file);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    string content(PHXRPC_CLIENT_ETC_TEMPLATE);
    StrTrim(&content);
    StrReplaceAll(&content, "$PbPackageName$", stree->package_name());
    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

