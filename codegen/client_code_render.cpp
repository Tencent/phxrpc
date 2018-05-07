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

#include "code_utils.h"

#include "client_code_render.h"
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

void ClientCodeRender::GenerateStubHpp(SyntaxTree *stree,
                                       const map<string, SyntaxTree> &protocol2syntax_tree_map,
                                       FILE *write) {
    char filename[128]{0};
    name_render_.GetStubFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    fprintf(write, "\n");
    fprintf(write, "\n");

    fprintf(write, "namespace phxrpc {\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
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
        fprintf(write, "    %s(phxrpc::BaseTcpStream &socket, phxrpc::ClientMonitor &client_monitor);\n", class_name);
        fprintf(write, "    virtual ~%s();\n", class_name);
        fprintf(write, "\n");

        fprintf(write, "    void SetKeepAlive(const bool keep_alive);\n\n");

        for (const auto &kv : protocol2syntax_tree_map) {
            fprintf(write, "    // %s protocol\n", kv.first.c_str());
            const auto &funcs{kv.second.GetFuncList()};
            for (const auto &func : *funcs) {
                string buffer;
                GetStubFuncDeclaration(stree, &func, 1, &buffer);
                fprintf(write, "    %s;\n", buffer.c_str());
            }
            fprintf(write, "\n");
        }

        fprintf(write, "    // user custom\n");
        SyntaxFuncVector *flist{stree->GetFuncList()};
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
        fprintf(write, "    bool keep_alive_;\n");

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
    phxrpc::StrAppendFormat(result, "const %s &req", type_name);

    const char *const resp_type{func->GetResp()->GetType()};
    if (resp_type && 0 < strlen(resp_type)) {
        name_render_.GetMessageClassName(resp_type, type_name, sizeof(type_name));
        phxrpc::StrAppendFormat(result, ", %s *resp", type_name);
    }

    phxrpc::StrAppendFormat(result, ")");
}

void ClientCodeRender::GenerateStubCpp(SyntaxTree *stree,
                                       const map<string, SyntaxTree> &protocol2syntax_tree_map,
                                       FILE *write) {
    char filename[128]{0};
    name_render_.GetStubFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", filename);

    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/rpc.h\"\n");
    fprintf(write, "#include \"phxrpc/network.h\"\n");
    fprintf(write, "\n");
    fprintf(write, "\n");

    char class_name[128]{'\0'};
    name_render_.GetStubClassName(stree->GetName(), class_name, sizeof(class_name));

    {
        fprintf(write, "%s::%s(phxrpc::BaseTcpStream &socket, phxrpc::ClientMonitor &client_monitor)\n",
                class_name, class_name);

        fprintf(write, "        : socket_(socket), client_monitor_(client_monitor), keep_alive_(false) {\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");

        fprintf(write, "%s::~%s() {\n", class_name, class_name);
        fprintf(write, "}\n");
        fprintf(write, "\n");

        fprintf(write, "void %s::SetKeepAlive(const bool keep_alive) {\n", class_name );
        fprintf(write, "    keep_alive_ = keep_alive;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");

        for (const auto &kv : protocol2syntax_tree_map) {
            fprintf(write, "// %s protocol\n", kv.first.c_str());
            fprintf(write, "\n");
            const auto &funcs{kv.second.GetFuncList()};
            for (const auto &func : *funcs) {
                if ("http" == kv.first) {
                    GenerateStubFunc(stree, &(kv.second), &func, write, true);
                } else {
                    GenerateStubFunc(stree, &(kv.second), &func, write, false);
                }
            }
        }

        fprintf(write, "// user custom\n");
        fprintf(write, "\n");
        SyntaxFuncVector *flist{stree->GetFuncList()};
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            GenerateStubFunc(stree, stree, &(*fit), write, true);
        }

    }
}

void ClientCodeRender::GenerateStubFunc(const SyntaxTree *const stree,
                                        const SyntaxTree *const stree2,
                                        const SyntaxFunc *const func,
                                        FILE *write, const bool use_default_caller) {
    string buffer;

    GetStubFuncDeclaration(stree, func, 0, &buffer);

    char caller_name[128]{"HttpCaller"};
    if (!use_default_caller) {
        // protocol
        name_render_.GetCallerClassName(stree2->GetName(), caller_name, sizeof(caller_name));
    }

    fprintf(write, "%s {\n", buffer.c_str());

    fprintf(write, "    phxrpc::%s caller(socket_, client_monitor_);\n", caller_name);
    fprintf(write, "    caller.SetURI(\"/%s/%s\", %d);\n",
            SyntaxTree::Cpp2UriPackageName(stree2->GetCppPackageName()).c_str(),
            func->GetName(), func->GetCmdID());
    fprintf(write, "    caller.SetKeepAlive(keep_alive_);\n");
    if (!use_default_caller) {
        // protocol
        fprintf(write, "    return caller.%sCall(req, resp);\n", func->GetName());
    } else {
        // user custom
        fprintf(write, "    return caller.Call(req, resp);\n");
    }

    fprintf(write, "}\n");
    fprintf(write, "\n");
}

void ClientCodeRender::GenerateClientHpp(SyntaxTree *stree,
                                         const map<string, SyntaxTree> &protocol2syntax_tree_map,
                                         FILE *write, const bool is_uthread_mode) {
    char filename[128]{0};
    name_render_.GetClientFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    string declarations;

    {
        for (const auto &kv : protocol2syntax_tree_map) {
            declarations.append("    // ").append(kv.first).append(" protocol\n");
            const auto &funcs{kv.second.GetFuncList()};
            for (const auto &func : *funcs) {
                string buffer;
                GetClienfuncDeclaration(stree, &func, 1, &buffer, is_uthread_mode);
                declarations.append("    ").append(buffer).append(";\n");
            }
            declarations.append("\n");
        }
    }

    {
        declarations.append("    // user custom\n");
        SyntaxFuncVector *flist{stree->GetFuncList()};
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            string buffer;
            GetClienfuncDeclaration(stree, &(*fit), 1, &buffer, is_uthread_mode);
            declarations.append("    ").append(buffer).append(";\n");

            if (0 == strcmp(fit->GetName(), "PhxEcho")) {
                SyntaxFunc echo_func = *fit;
                echo_func.SetName("PhxBatchEcho");
                string buffer;
                GetClienfuncDeclaration(stree, &echo_func, 1, &buffer, is_uthread_mode);
                declarations.append("    ").append(buffer).append(";\n");
            }
        }
    }

    char client_class[128]{0}, message_file[128]{0};
    char client_class_lower[128]{0};
    name_render_.GetClientClassName(stree->GetName(), client_class, sizeof(client_class));
    name_render_.GetClientClassNameLower(stree->GetName(), client_class_lower, sizeof(client_class_lower));
    name_render_.GetMessageFileName(stree->GetProtoFile(), message_file, sizeof(message_file));

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
                                         const map<string, SyntaxTree> &protocol2syntax_tree_map,
                                         FILE *write, const bool is_uthread_mode) {
    char client_class[128]{0}, client_file[128]{0};
    char client_class_lower[128]{0};
    char stub_class[128]{0}, stub_file[128]{0};
    name_render_.GetClientClassName(stree->GetName(), client_class, sizeof(client_class));
    name_render_.GetClientClassNameLower(stree->GetName(), client_class_lower, sizeof(client_class_lower));
    name_render_.GetClientFileName(stree->GetName(), client_file, sizeof(client_file));
    name_render_.GetStubClassName(stree->GetName(), stub_class, sizeof(stub_class));
    name_render_.GetStubFileName(stree->GetName(), stub_file, sizeof(stub_file));

    string client_class_str = string(client_class);
    string client_class_lower_str = string(client_class_lower);
    if (is_uthread_mode) {
        client_class_str += "UThread";
        client_class_lower_str += "uthread";
    }

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2client", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", client_file);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");

    fprintf(write, "\n");

    string functions;

    {
        for (const auto &kv : protocol2syntax_tree_map) {
            functions.append("// ").append(kv.first).append(" protocol\n\n");
            const auto &funcs{kv.second.GetFuncList()};
            for (const auto &func : *funcs) {
                string buffer;
                GetClienfuncDeclaration(stree, &(func), 0, &buffer, is_uthread_mode);

                functions.append(buffer).append(" ");

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
                string func_string(func.GetName());
                func_string += "(req, resp)";
                StrReplaceAll(&content, "$Func$", func_string);

                functions.append(content).append("\n\n");
            }
            fprintf(write, "\n");
        }
    }

    {
        functions.append("// user custom\n\n");
        SyntaxFuncVector *flist{stree->GetFuncList()};
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            string buffer;
            GetClienfuncDeclaration(stree, &(*fit), 0, &buffer, is_uthread_mode);

            functions.append(buffer).append(" ");

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

            if (strcmp(fit->GetName(), "PhxEcho") == 0) {
                SyntaxFunc echo_func = *fit;
                echo_func.SetName("PhxBatchEcho");

                string buffer;
                GetClienfuncDeclaration(stree, &echo_func, 0, &buffer, is_uthread_mode);

                functions.append(buffer).append(" ");

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
    StrReplaceAll(&content, "$PbPackageName$", SyntaxTree::Cpp2PbPackageName(stree->GetCppPackageName()));
    StrReplaceAll(&content, "$ClientFile$", client_file);
    StrReplaceAll(&content, "$StubFile$", stub_file);
    StrReplaceAll(&content, "$ClientClass$", client_class_str);
    StrReplaceAll(&content, "$ClientClassLower$", client_class_lower_str.c_str());
    StrReplaceAll(&content, "$ClientClassFuncs$", functions);

    fprintf(write, "%s", content.c_str());
}

void ClientCodeRender::GetClienfuncDeclaration(SyntaxTree *stree,
                                               const SyntaxFunc *const func,
                                               int is_header, string *result,
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

    const char *const resp_type{func->GetResp()->GetType()};
    if (resp_type && 0 < strlen(resp_type)) {
        name_render_.GetMessageClassName(resp_type, type_name, sizeof(type_name));
        phxrpc::StrAppendFormat(result, ", %s *resp", type_name);
    }

    phxrpc::StrAppendFormat(result, ")");
}

void ClientCodeRender::GenerateClientEtc(SyntaxTree *stree, FILE *write) {
    char etcfile[128]{0};
    name_render_.GetClientEtcFileName(stree->GetName(), etcfile, sizeof(etcfile));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2server", stree->GetProtoFile(), &buffer, false, "#");

    fprintf(write, "# %s\n", etcfile);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "#\n");
    fprintf(write, "\n");

    string content(PHXRPC_CLIENT_ETC_TEMPLATE);
    StrTrim(&content);
    StrReplaceAll(&content, "$PbPackageName$", SyntaxTree::Cpp2PbPackageName(stree->GetCppPackageName()));
    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

