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

#include <string.h>

#include "tool_code_render.h"
#include "tool_template.h"

#include "syntax_tree.h"
#include "name_render.h"
#include "code_utils.h"


using namespace phxrpc;
using namespace std;


ToolCodeRender::ToolCodeRender(NameRender &name_render)
        : name_render_(name_render) {
}

ToolCodeRender::~ToolCodeRender() {
}

void ToolCodeRender::GenerateToolHpp(SyntaxTree *stree,
                                     const SyntaxFuncVector &mqtt_funcs,
                                     FILE *write) {
    char filename[128]{0};
    name_render_.GetToolFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2tool", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");
    fprintf(write, "\n");

    fprintf(write, "#include <cstdio>\n");
    fprintf(write, "\n");
    fprintf(write, "\n");

    fprintf(write, "namespace phxrpc {\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "class OptMap;\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "}\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    char clasname[128]{0};
    name_render_.GetToolClasname(stree->GetName(), clasname, sizeof(clasname));

    fprintf(write, "class %s {\n", clasname);
    fprintf(write, "  public:\n");
    fprintf(write, "    %s();\n", clasname);
    fprintf(write, "    virtual ~%s();\n", clasname);
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        fprintf(write, "    virtual int %s(phxrpc::OptMap &bigmap);\n", mqtt_it->GetName());
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        fprintf(write, "    virtual int %s(phxrpc::OptMap &bigmap);\n", fit->GetName());
    }
    fprintf(write, "\n");

    fprintf(write, "    typedef int (%s::*ToolFunc_t)(phxrpc::OptMap &);\n", clasname);
    fprintf(write, "\n");
    fprintf(write, "    typedef struct tagName2Func {\n");
    fprintf(write, "        const char *name;\n");
    fprintf(write, "        %s::ToolFunc_t func;\n", clasname);
    fprintf(write, "        const char *opt_string;\n");
    fprintf(write, "        const char *usage;\n");
    fprintf(write, "    } Name2Func_t;\n");
    fprintf(write, "\n");
    fprintf(write, "    static Name2Func_t *GetName2Func() {\n");
    fprintf(write, "        static Name2Func_t name2func[]{\n");
    {
        for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
            ++mqtt_it) {
            fprintf(write, "            {\"%s\", &%s::%s, \"c:f:v%s\",\n                    \"%s\"},\n",
                    mqtt_it->GetName(), clasname,
                    mqtt_it->GetName(), mqtt_it->GetOptString(), mqtt_it->GetUsage());
        }

        SyntaxFuncVector *flist{stree->GetFuncList()};
        auto fit(flist->cbegin());
        for (; flist->cend() != fit; ++fit) {
            if (0 < strlen(fit->GetOptString())) {
                fprintf(write, "            {\"%s\", &%s::%s, \"c:f:v%s\",\n                    \"%s\"},\n",
                        fit->GetName(), clasname,
                        fit->GetName(), fit->GetOptString(), fit->GetUsage());
            }
        }

        fprintf(write, "            {nullptr, nullptr}\n");
    }
    fprintf(write, "        };\n");
    fprintf(write, "\n");

    fprintf(write, "        return name2func;\n");
    fprintf(write, "    };\n");
    fprintf(write, "};\n");

    fprintf(write, "\n");
}

void ToolCodeRender::GenerateToolCpp(SyntaxTree *stree,
                                     const SyntaxFuncVector &mqtt_funcs,
                                     FILE *write) {
    char filename[128]{0};
    name_render_.GetToolFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2tool", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/file.h\"\n\n");
    fprintf(write, "#include \"%s.h\"\n", filename);

    name_render_.GetClientFileName(stree->GetName(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "using namespace phxrpc;\n");
    fprintf(write, "\n");
    fprintf(write, "\n");

    char clasname[128]{0};
    name_render_.GetToolClasname(stree->GetName(), clasname, sizeof(clasname));

    fprintf(write, "%s::%s() {\n", clasname, clasname);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "%s::~%s() {\n", clasname, clasname);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    char client_class[128]{0};
    name_render_.GetClientClasname(stree->GetName(), client_class, sizeof(client_class));

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        fprintf(write, "int %s::%s(phxrpc::OptMap &/* opt_map */) {\n", clasname, mqtt_it->GetName());
        fprintf(write, "    printf(\"\\n    *** %s unimplement ***\\n\");\n\n", mqtt_it->GetName());
        fprintf(write, "    return -1;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        fprintf(write, "int %s::%s(phxrpc::OptMap &/* opt_map */) {\n", clasname, fit->GetName());
        fprintf(write, "    printf(\"\\n    *** %s unimplement ***\\n\");\n\n", fit->GetName());
        fprintf(write, "    return -1;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }
}

void ToolCodeRender::GenerateToolImplHpp(SyntaxTree *stree,
                                         const SyntaxFuncVector &mqtt_funcs,
                                         FILE *write) {
    char filename[128]{0};
    name_render_.GetToolImplFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2tool", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");
    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    char toolfile[128]{0};
    name_render_.GetToolFileName(stree->GetName(), toolfile, sizeof(toolfile));

    fprintf(write, "#include \"%s.h\"\n", toolfile);

    fprintf(write, "\n");

    fprintf(write, "#include <cstdio>\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    fprintf(write, "namespace phxrpc {\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "class OptMap;\n");
    fprintf(write, "\n");
    fprintf(write, "\n");
    fprintf(write, "}\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    char clasname[128]{0};
    name_render_.GetToolClasname(stree->GetName(), clasname, sizeof(clasname));

    fprintf(write, "class %sImpl : public %s {\n", clasname, clasname);
    fprintf(write, "  public:\n");
    fprintf(write, "    %sImpl();\n", clasname);
    fprintf(write, "    virtual ~%sImpl();\n", clasname);
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        fprintf(write, "    virtual int %s(phxrpc::OptMap &opt_map) override;\n", mqtt_it->GetName());
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        fprintf(write, "    virtual int %s(phxrpc::OptMap &opt_map) override;\n", fit->GetName());
    }

    fprintf(write, "};\n");

    fprintf(write, "\n");
}

void ToolCodeRender::GenerateToolImplCpp(SyntaxTree *stree,
                                         const SyntaxFuncVector &mqtt_funcs,
                                         FILE *write) {
    char filename[128]{0};
    name_render_.GetToolImplFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2tool", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "\n");

    name_render_.GetClientFileName(stree->GetName(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "\n");
    fprintf(write, "#include \"phxrpc/file.h\"\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    fprintf(write, "using namespace phxrpc;\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    char clasname[128]{0};
    name_render_.GetToolImplClasname(stree->GetName(), clasname, sizeof(clasname));

    fprintf(write, "%s::%s() {\n", clasname, clasname);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "%s::~%s() {\n", clasname, clasname);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    char client_class[128]{0}, req_class[128]{0}, resp_class[128]{0};
    name_render_.GetClientClasname(stree->GetName(), client_class, sizeof(client_class));

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        name_render_.GetMessageClasname(mqtt_it->GetReq()->GetType(), req_class, sizeof(req_class));
        const char *const resp_type{mqtt_it->GetResp()->GetType()};
        if (resp_type && 0 < strlen(resp_type)) {
            name_render_.GetMessageClasname(mqtt_it->GetResp()->GetType(), resp_class, sizeof(resp_class));
        }

        fprintf(write, "int %s::%s(phxrpc::OptMap &opt_map) {\n", clasname, mqtt_it->GetName());
        fprintf(write, "    %s req;\n", req_class);
        if (resp_type && 0 < strlen(resp_type)) {
            fprintf(write, "    %s resp;\n", resp_class);
        }
        fprintf(write, "\n");

        if (0 == strcmp("PhxMqttPublish", mqtt_it->GetName())) {
            fprintf(write, "    if (nullptr == opt_map.Get('s')) return -1;\n\n");
            fprintf(write, "    req.set_content(opt_map.Get('s'));\n");
        }

        fprintf(write, "\n");
        fprintf(write, "    %s client;\n", client_class);
        if (resp_type && 0 < strlen(resp_type)) {
            fprintf(write, "    int ret{client.%s(req, &resp)};\n", mqtt_it->GetName());
        } else {
            fprintf(write, "    int ret{client.%s(req)};\n", mqtt_it->GetName());
        }
        fprintf(write, "    printf(\"%%s return %%d\\n\", __func__, ret);\n");
        if (resp_type && 0 < strlen(resp_type)) {
            fprintf(write, "    printf(\"resp: {\\n%%s}\\n\", resp.DebugString().c_str());\n");
        }
        fprintf(write, "\n");
        fprintf(write, "    return ret;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }
    fprintf(write, "\n");

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        name_render_.GetMessageClasname(fit->GetReq()->GetType(), req_class, sizeof(req_class));
        name_render_.GetMessageClasname(fit->GetResp()->GetType(), resp_class, sizeof(resp_class));

        fprintf(write, "int %s::%s(phxrpc::OptMap &opt_map) {\n", clasname, fit->GetName());
        fprintf(write, "    %s req;\n", req_class);
        fprintf(write, "    %s resp;\n", resp_class);
        fprintf(write, "\n");

        if (0 == strcmp("PhxEcho", fit->GetName())) {
            fprintf(write, "    if (nullptr == opt_map.Get('s')) return -1;\n\n");
            fprintf(write, "    req.set_value(opt_map.Get('s'));\n");
        } else {
            fprintf(write, "    // TODO: fill req from opt_map\n");
        }

        fprintf(write, "\n");
        fprintf(write, "    %s client;\n", client_class);
        fprintf(write, "    int ret{client.%s(req, &resp)};\n", fit->GetName());
        fprintf(write, "    printf(\"%%s return %%d\\n\", __func__, ret);\n");
        fprintf(write, "    printf(\"resp: {\\n%%s}\\n\", resp.DebugString().c_str());\n");
        fprintf(write, "\n");
        fprintf(write, "    return ret;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }
}

void ToolCodeRender::GenerateToolMainCpp(SyntaxTree *stree, FILE *write) {
    char filename[128]{0};
    name_render_.GetToolMainFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2tool", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    char tool_class[128]{0}, tool_file[128]{0};
    char tool_impl_class[128]{0}, tool_impl_file[128]{0};
    char client_class[128]{0}, client_file[128]{0};

    name_render_.GetToolClasname(stree->GetName(), tool_class, sizeof(tool_class));
    name_render_.GetToolFileName(stree->GetName(), tool_file, sizeof(tool_file));
    name_render_.GetToolImplClasname(stree->GetName(), tool_impl_class, sizeof(tool_impl_class));
    name_render_.GetToolImplFileName(stree->GetName(), tool_impl_file, sizeof(tool_impl_file));
    name_render_.GetClientClasname(stree->GetName(), client_class, sizeof(client_class));
    name_render_.GetClientFileName(stree->GetName(), client_file, sizeof(client_file));

    string content(PHXRPC_TOOL_MAIN_TEMPLATE);
    StrTrim(&content);
    StrReplaceAll(&content, "$ClientFile$", client_file);
    StrReplaceAll(&content, "$ClientClass$", client_class);
    StrReplaceAll(&content, "$ToolFile$", tool_file);
    StrReplaceAll(&content, "$ToolClass$", tool_class);
    StrReplaceAll(&content, "$ToolImplFile$", tool_impl_file);
    StrReplaceAll(&content, "$ToolImplClass$", tool_impl_class);

    fprintf(write, "%s", content.c_str());

    fprintf(write, "\n");
    fprintf(write, "\n");
}

