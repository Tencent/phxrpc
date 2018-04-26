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

#include <cstring>
#include <string>

#include "service_code_render.h"

#include "name_render.h"
#include "syntax_tree.h"

#include "code_utils.h"


using namespace phxrpc;
using namespace std;


ServiceCodeRender::ServiceCodeRender(NameRender &name_render)
        : name_render_(name_render) {
}

ServiceCodeRender::~ServiceCodeRender() {
}

void ServiceCodeRender::GenerateServiceHpp(SyntaxTree *stree,
                                           const SyntaxFuncVector &mqtt_funcs,
                                           FILE *write) {
    char filename[128]{0};
    name_render_.GetServiceFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/msg/base_msg.h\"\n");

    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    fprintf(write, "\n");
    fprintf(write, "\n");

    char service_name[128]{0};
    name_render_.GetServiceClassName(stree->GetName(), service_name, sizeof(service_name));

    fprintf(write, "class %s {\n", service_name);
    fprintf(write, "  public:\n");
    fprintf(write, "    %s();\n", service_name);
    fprintf(write, "    virtual ~%s();\n", service_name);
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*mqtt_it), 1, 0, 1, &buffer);
        fprintf(write, "    virtual %s;\n", buffer.c_str());
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*fit), 1, 0, 1, &buffer);
        fprintf(write, "    virtual %s;\n", buffer.c_str());
    }

    fprintf(write, "};\n");

    fprintf(write, "\n");
}

void ServiceCodeRender::GenerateServiceCpp(SyntaxTree *stree,
                                           const SyntaxFuncVector &mqtt_funcs,
                                           FILE *write) {
    char filename[128]{0};
    name_render_.GetServiceFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "#include \"phxrpc/file.h\"\n");

    fprintf(write, "\n");
    fprintf(write, "\n");

    char service_name[128]{0};
    name_render_.GetServiceClassName(stree->GetName(), service_name, sizeof(service_name));

    fprintf(write, "%s::%s() {\n", service_name, service_name);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "%s::~%s() {\n", service_name, service_name);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*mqtt_it), 0, 0, 0, &buffer);
        fprintf(write, "%s {\n", buffer.c_str());
        fprintf(write, "    phxrpc::log(LOG_ERR, \"ERROR: %s unimplemented\");\n", mqtt_it->GetName());
        fprintf(write, "    return -1;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*fit), 0, 0, 0, &buffer);
        fprintf(write, "%s {\n", buffer.c_str());
        fprintf(write, "    phxrpc::log(LOG_ERR, \"ERROR: %s unimplemented\");\n", fit->GetName());
        fprintf(write, "    return -1;\n");
        fprintf(write, "}\n");
        fprintf(write, "\n");
    }
}

void ServiceCodeRender::GenerateServiceImplHpp(SyntaxTree *stree,
                                               const SyntaxFuncVector &mqtt_funcs,
                                               FILE *write,
                                               const bool is_uthread_mode) {
    char filename[128]{0};
    name_render_.GetServiceImplFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    name_render_.GetServiceFileName(stree->GetName(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);
    if (is_uthread_mode) {
        fprintf(write, "#include \"phxrpc/network.h\"\n");
    }

    fprintf(write, "\n");
    fprintf(write, "\n");

    char service_impl_name[128]{0}, base_name[128]{0}, config_name[128]{0};
    name_render_.GetServiceClassName(stree->GetName(), base_name, sizeof(base_name));
    name_render_.GetServiceImplClassName(stree->GetName(), service_impl_name, sizeof(service_impl_name));
    name_render_.GetServerConfigClassName(stree->GetName(), config_name, sizeof(config_name));

    fprintf(write, "class %s;\n", config_name);
    fprintf(write, "\n");

    fprintf(write, "namespace phxrpc {\n");
    fprintf(write, "\n");
    fprintf(write, "class DataFlow;\n");
    fprintf(write, "class NotifierPoolRouter;\n");
    fprintf(write, "struct ServiceContext;\n");
    fprintf(write, "\n");
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "typedef struct tagServiceArgs {\n");
    fprintf(write, "    %s *config;\n", config_name);
    fprintf(write, "    // you can add other arguments here and initiate in main().\n");
    fprintf(write, "} ServiceArgs_t;\n");
    fprintf(write, "\n");

    fprintf(write, "class %s : public %s {\n", service_impl_name, base_name);
    fprintf(write, "  public:\n");
    if (!is_uthread_mode) {
        fprintf(write, "    %s(ServiceArgs_t &app_args,\n", service_impl_name);
        fprintf(write, "            const int pool_idx, const int worker_idx,\n");
        fprintf(write, "            phxrpc::UThreadNotifierPool *const notifier_pool,\n");
        fprintf(write, "            phxrpc::NotifierPoolRouter *const notifier_pool_router,\n");
        fprintf(write, "            phxrpc::DataFlow *const data_flow, "
                "phxrpc::DataFlow *const cross_unit_data_flow,\n");
        fprintf(write, "            phxrpc::ServiceContext *context);\n");
    } else {
        fprintf(write, "    %s(ServiceArgs_t &app_args,\n", service_impl_name);
        fprintf(write, "            phxrpc::UThreadEpollScheduler *worker_uthread_scheduler,\n");
        fprintf(write, "            const int pool_idx, const int worker_idx,\n");
        fprintf(write, "            phxrpc::UThreadNotifierPool *const notifier_pool,\n");
        fprintf(write, "            phxrpc::NotifierPoolRouter *const notifier_pool_router,\n");
        fprintf(write, "            phxrpc::DataFlow *const data_flow, "
                "phxrpc::DataFlow *const cross_unit_data_flow,\n");
        fprintf(write, "            phxrpc::ServiceContext *context);\n");
    }
    fprintf(write, "    virtual ~%s() override;\n", service_impl_name);
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*mqtt_it), 1, 1, 1, &buffer);
        fprintf(write, "    virtual %s override;\n", buffer.c_str());
    }
    fprintf(write, "\n");

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*fit), 1, 1, 1, &buffer);
        fprintf(write, "    virtual %s override;\n", buffer.c_str());
    }
    fprintf(write, "\n");

    fprintf(write, "  private:\n");
    fprintf(write, "    ServiceArgs_t &args_;\n");
    if (is_uthread_mode) {
        fprintf(write, "    phxrpc::UThreadEpollScheduler *worker_uthread_scheduler_{nullptr};\n");
    }
    fprintf(write, "    int pool_idx_{-1};\n");
    fprintf(write, "    int worker_idx_{-1};\n");
    fprintf(write, "    phxrpc::UThreadNotifierPool *notifier_pool_{nullptr};\n");
    fprintf(write, "    phxrpc::NotifierPoolRouter *notifier_pool_router_{nullptr};\n");
    fprintf(write, "    phxrpc::DataFlow *data_flow_{nullptr};\n");
    fprintf(write, "    phxrpc::DataFlow *cross_unit_data_flow_{nullptr};\n");
    fprintf(write, "    phxrpc::ServiceContext *context_{nullptr};\n");

    fprintf(write, "};\n");

    fprintf(write, "\n");
}

void ServiceCodeRender::GenerateServiceImplCpp(SyntaxTree *stree,
                                               const SyntaxFuncVector &mqtt_funcs,
                                               FILE *write,
                                               const bool is_uthread_mode) {
    char filename[128]{0}, config_file[128]{0};
    name_render_.GetServiceImplFileName(stree->GetName(), filename, sizeof(filename));
    name_render_.GetServerConfigFileName(stree->GetName(), config_file, sizeof(config_file));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer, false);

    fprintf(write, "/* %s.cpp\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", config_file);

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "#include \"phxrpc/file.h\"\n");

    fprintf(write, "\n\n");
    fprintf(write, "using namespace std;\n");
    fprintf(write, "\n\n");

    char service_impl_name[128]{0}, config_name[128]{0};
    name_render_.GetServiceImplClassName(stree->GetName(), service_impl_name, sizeof(service_impl_name));
    name_render_.GetServerConfigClassName(stree->GetName(), config_name, sizeof(config_name));

    if (!is_uthread_mode) {
        fprintf(write, "%s::%s(ServiceArgs_t &app_args,\n", service_impl_name, service_impl_name);
        fprintf(write, "        const int pool_idx, const int worker_idx,\n");
        fprintf(write, "        phxrpc::UThreadNotifierPool *const notifier_pool,\n");
        fprintf(write, "        phxrpc::NotifierPoolRouter *const notifier_pool_router,\n");
        fprintf(write, "        phxrpc::DataFlow *const data_flow, "
                "phxrpc::DataFlow *const cross_unit_data_flow,\n");
        fprintf(write, "        phxrpc::ServiceContext *context)\n");
        fprintf(write, "        : args_(app_args), pool_idx_(pool_idx), worker_idx_(worker_idx),\n");
        fprintf(write, "          notifier_pool_(notifier_pool), "
                "notifier_pool_router_(notifier_pool_router),\n");
        fprintf(write, "          data_flow_(data_flow), "
                "cross_unit_data_flow_(cross_unit_data_flow), context_(context) {\n");
    } else {
        fprintf(write, "%s::%s(ServiceArgs_t &app_args,\n", service_impl_name, service_impl_name);
        fprintf(write, "        phxrpc::UThreadEpollScheduler *const worker_uthread_scheduler,\n");
        fprintf(write, "        const int pool_idx, const int worker_idx,\n");
        fprintf(write, "        phxrpc::UThreadNotifierPool *const notifier_pool,\n");
        fprintf(write, "        phxrpc::NotifierPoolRouter *const notifier_pool_router,\n");
        fprintf(write, "        phxrpc::DataFlow *const data_flow, "
                "phxrpc::DataFlow *const cross_unit_data_flow,\n");
        fprintf(write, "        phxrpc::ServiceContext *context)\n");
        fprintf(write, "        : args_(app_args),\n");
        fprintf(write, "          worker_uthread_scheduler_(worker_uthread_scheduler),\n");
        fprintf(write, "          pool_idx_(pool_idx), worker_idx_(worker_idx),\n");
        fprintf(write, "          notifier_pool_(notifier_pool), "
                "notifier_pool_router_(notifier_pool_router),\n");
        fprintf(write, "          data_flow_(data_flow), "
                "cross_unit_data_flow_(cross_unit_data_flow), context_(context) {\n");
    }
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "%s::~%s() {\n", service_impl_name, service_impl_name);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*mqtt_it), 0, 1, 1, &buffer);
        fprintf(write, "%s {\n", buffer.c_str());

        if (0 == strcmp("PhxMqttConnect", mqtt_it->GetName())) {
            fprintf(write, "    context_->client_identifier = req.client_identifier();\n");
            fprintf(write, "\n");
            fprintf(write, "    return 0;\n");
        } else if (0 == strcmp("PhxMqttPublish", mqtt_it->GetName())) {
            fprintf(write, "    // TODO: req.packet_identifier(), req.content()\n");
            fprintf(write, "\n");
            fprintf(write, "    return -1;\n");
        } else if (0 == strcmp("PhxMqttPuback", mqtt_it->GetName())) {
            fprintf(write, "    // TODO: req.packet_identifier()\n");
            fprintf(write, "\n");
            fprintf(write, "    return -1;\n");
        } else {
            fprintf(write, "    return 0;\n");
        }

        fprintf(write, "}\n");
        fprintf(write, "\n");
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        string buffer;
        GetServiceFuncDeclaration(stree, &(*fit), 0, 1, 1, &buffer);
        fprintf(write, "%s {\n", buffer.c_str());

        if (0 == strcmp("PhxEcho", fit->GetName())) {
            fprintf(write, "    resp->set_value(req.value());\n");
            fprintf(write, "\n");
            fprintf(write, "    return 0;\n");
        } else {
            fprintf(write, "    return -1;\n");
        }

        fprintf(write, "}\n");
        fprintf(write, "\n");
    }
}

void ServiceCodeRender::GetServiceFuncDeclaration(SyntaxTree *stree,
        const SyntaxFunc *const func, int is_header, int is_impl,
        int need_param_name, string *result) {
    char service_impl_name[128]{0}, type_name[128]{0};

    if (is_impl) {
        name_render_.GetServiceImplClassName(stree->GetName(), service_impl_name, sizeof(service_impl_name));
    } else {
        name_render_.GetServiceClassName(stree->GetName(), service_impl_name, sizeof(service_impl_name));
    }

    if (is_header) {
        StrAppendFormat(result, "int %s(", func->GetName());
    } else {
        StrAppendFormat(result, "int %s::%s(", service_impl_name, func->GetName());
    }

    name_render_.GetMessageClassName(func->GetReq()->GetType(), type_name, sizeof(type_name));
    StrAppendFormat(result, "const %s &%s", type_name, need_param_name ? "req" : "/* req */");

    const char *const resp_type{func->GetResp()->GetType()};
    if (resp_type && 0 < strlen(resp_type)) {
        name_render_.GetMessageClassName(func->GetResp()->GetType(), type_name, sizeof(type_name));
        StrAppendFormat(result, ", %s *%s", type_name, need_param_name ? "resp" : "/* resp */");
    }

    result->append(")");
}

void ServiceCodeRender::GenerateDispatcherHpp(SyntaxTree *stree,
                                              const SyntaxFuncVector &mqtt_funcs,
                                              FILE *write) {
    char filename[128]{0};
    name_render_.GetDispatcherFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#pragma once\n");

    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/rpc.h\"\n");
    fprintf(write, "\n");
    fprintf(write, "\n");

    char dispatcher_name[128]{0}, service_name[128]{0};
    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_name, sizeof(dispatcher_name));
    name_render_.GetServiceClassName(stree->GetName(), service_name, sizeof(service_name));

    fprintf(write, "class %s;\n", service_name);
    fprintf(write, "\n");

    fprintf(write, "class %s {\n", dispatcher_name);

    fprintf(write, "  public:\n");
    fprintf(write, "    static const phxrpc::BaseDispatcher<%s>::URIFuncMap &GetURIFuncMap();\n", dispatcher_name);
    fprintf(write, "\n");

    fprintf(write, "    %s(%s &service, phxrpc::DispatcherArgs_t *dispatcher_args);\n", dispatcher_name, service_name);
    fprintf(write, "\n");

    fprintf(write, "    virtual ~%s();\n", dispatcher_name);
    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        fprintf(write, "    int %s(const phxrpc::BaseRequest *const req, "
                "phxrpc::BaseResponse *const resp);\n", mqtt_it->GetName());
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        fprintf(write, "    int %s(const phxrpc::BaseRequest *const req, "
                "phxrpc::BaseResponse *const resp);\n",
                fit->GetName());
    }
    fprintf(write, "\n");

    fprintf(write, "  private:\n");
    fprintf(write, "    %s &service_;\n", service_name);
    fprintf(write, "    phxrpc::DispatcherArgs_t *dispatcher_args_;\n" );

    fprintf(write, "};\n");

    fprintf(write, "\n");
}

void ServiceCodeRender::GenerateDispatcherCpp(SyntaxTree *stree,
                                              const SyntaxFuncVector &mqtt_funcs,
                                              FILE *write) {
    char filename[128]{0};
    name_render_.GetDispatcherFileName(stree->GetName(), filename, sizeof(filename));

    string buffer;
    name_render_.GetCopyright("phxrpc_pb2service", stree->GetProtoFile(), &buffer);

    fprintf(write, "/* %s.h\n", filename);
    fprintf(write, "%s", buffer.c_str());
    fprintf(write, "*/\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"%s.h\"\n", filename);
    fprintf(write, "\n");

    fprintf(write, "#include <errno.h>\n");
    fprintf(write, "\n");

    fprintf(write, "#include \"phxrpc/mqtt.h\"\n");
    fprintf(write, "#include \"phxrpc/http.h\"\n");
    fprintf(write, "#include \"phxrpc/file.h\"\n");
    fprintf(write, "\n");

    name_render_.GetMessageFileName(stree->GetProtoFile(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    name_render_.GetServiceFileName(stree->GetName(), filename, sizeof(filename));
    fprintf(write, "#include \"%s.h\"\n", filename);

    fprintf(write, "\n");
    fprintf(write, "\n");

    char dispatcher_name[128]{0}, service_name[128]{0};
    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_name, sizeof(dispatcher_name));
    name_render_.GetServiceClassName(stree->GetName(), service_name, sizeof(service_name));

    fprintf(write, "%s::%s(%s &service, phxrpc::DispatcherArgs_t *dispatcher_args)\n",
            dispatcher_name, dispatcher_name, service_name);
    fprintf(write, "        : service_(service), dispatcher_args_(dispatcher_args) {\n");
    fprintf(write, "}\n");
    fprintf(write, "\n");

    fprintf(write, "%s::~%s() {\n", dispatcher_name, dispatcher_name);
    fprintf(write, "}\n");
    fprintf(write, "\n");

    vector<FunctionItem> functions;

    if (0 < mqtt_funcs.size()) {
        FunctionItem connect_item;
        connect_item.uri = "/phxrpc/mqtt/connect";
        connect_item.function_name = "PhxMqttConnect";
        functions.push_back(connect_item);

        FunctionItem publish_item;
        publish_item.uri = "/phxrpc/mqtt/publish";
        publish_item.function_name = "PhxMqttPublish";
        functions.push_back(publish_item);

        FunctionItem puback_item;
        puback_item.uri = "/phxrpc/mqtt/puback";
        puback_item.function_name = "PhxMqttPuback";
        functions.push_back(puback_item);

        FunctionItem subscribe_item;
        subscribe_item.uri = "/phxrpc/mqtt/subscribe";
        subscribe_item.function_name = "PhxMqttSubscribe";
        functions.push_back(subscribe_item);

        FunctionItem unsubscribe_item;
        unsubscribe_item.uri = "/phxrpc/mqtt/unsubscribe";
        unsubscribe_item.function_name = "PhxMqttUnsubscribe";
        functions.push_back(unsubscribe_item);

        FunctionItem ping_item;
        ping_item.uri = "/phxrpc/mqtt/ping";
        ping_item.function_name = "PhxMqttPing";
        functions.push_back(ping_item);

        FunctionItem disconnect_item;
        disconnect_item.uri = "/phxrpc/mqtt/disconnect";
        disconnect_item.function_name = "PhxMqttDisconnect";
        functions.push_back(disconnect_item);
    }

    GenerateURIFuncMap(stree, functions, write);

    fprintf(write, "\n");

    for (auto mqtt_it(mqtt_funcs.cbegin()); mqtt_funcs.cend() != mqtt_it;
         ++mqtt_it) {
        GenerateDispatcherMqttFunc(stree, &(*mqtt_it), write);
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        GenerateDispatcherHttpFunc(stree, &(*fit), write);
    }
}

void ServiceCodeRender::GenerateURIFuncMap(SyntaxTree *stree, const vector<FunctionItem> functions,
                                           FILE *write) {
    char dispatcher_name[128]{0};
    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_name, sizeof(dispatcher_name));

    fprintf(write, "const phxrpc::BaseDispatcher<%s>::URIFuncMap &%s::GetURIFuncMap() {\n",
            dispatcher_name, dispatcher_name);

    fprintf(write, "    static phxrpc::BaseDispatcher<%s>::URIFuncMap uri_func_map = {\n",
            dispatcher_name);

    for (auto it(functions.cbegin()); functions.cend() != it; ++it) {
        fprintf(write, "        {\"%s\", &%s::%s},\n",
                it->uri.c_str(), dispatcher_name, it->function_name.c_str());
    }

    SyntaxFuncVector *flist{stree->GetFuncList()};
    auto fit(flist->cbegin());
    for (; flist->cend() != fit; ++fit) {
        if (fit != flist->cbegin()) {
            fprintf(write, ",\n");
        }
        fprintf(write, "        {\"/%s/%s\", &%s::%s}",
                SyntaxTree::Cpp2PbPackageName(stree->GetCppPackageName()).c_str(),
                fit->GetName(), dispatcher_name, fit->GetName());
    }
    fprintf(write, "};\n");

    fprintf(write, "    return uri_func_map;\n");

    fprintf(write, "}\n");
}

void ServiceCodeRender::GenerateDispatcherMqttFunc(SyntaxTree *stree,
                                                   const SyntaxFunc *const func,
                                                   FILE *write) {
    char dispatcher_name[128]{0}, type_name[128]{0};

    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_name, sizeof(dispatcher_name));

    fprintf(write, "int %s::%s(const phxrpc::BaseRequest *const req, "
            "phxrpc::BaseResponse *const resp) {\n",
            dispatcher_name, func->GetName());
    fprintf(write, "    dispatcher_args_->server_monitor->SvrCall(%d, \"%s\", 1);\n",
            func->GetCmdID(), func->GetName());
    fprintf(write, "\n");
    fprintf(write, "    int ret{0};\n");
    fprintf(write, "\n");
    name_render_.GetMessageClassName(func->GetReq()->GetType(), type_name, sizeof(type_name));
    fprintf(write, "    %s req_pb;\n", type_name);
    bool has_resp{func->GetResp()->GetType() && 0 < strlen(func->GetResp()->GetType())};
    if (has_resp) {
        name_render_.GetMessageClassName(func->GetResp()->GetType(), type_name, sizeof(type_name));
        fprintf(write, "    %s resp_pb;\n", type_name);
    }
    fprintf(write, "\n");
    fprintf(write, "    // unpack request\n");
    fprintf(write, "    {\n");
    fprintf(write, "        phxrpc::ReturnCode ret_code{req->ToPb(&req_pb)};\n");
    fprintf(write, "        if (phxrpc::ReturnCode::OK != ret_code) {\n");
    fprintf(write, "            phxrpc::log(LOG_ERR, \"ToPb ip %%s err %%d\", req->GetClientIP(), static_cast<int>(ret_code));\n");
    fprintf(write, "\n");
    fprintf(write, "            return -EINVAL;\n");
    fprintf(write, "        }\n");
    fprintf(write, "    }\n");
    fprintf(write, "\n");
    fprintf(write, "    // logic process\n");
    fprintf(write, "    {\n");
    fprintf(write, "        if (0 == ret) {\n");
    if (has_resp) {
        fprintf(write, "            ret = service_.%s(req_pb, &resp_pb);\n", func->GetName());
    } else {
        fprintf(write, "            ret = service_.%s(req_pb);\n", func->GetName());
    }
    fprintf(write, "        }\n");
    fprintf(write, "    }\n");
    fprintf(write, "\n");
    if (has_resp) {
        fprintf(write, "    // pack response\n");
        fprintf(write, "    {\n");
        fprintf(write, "        phxrpc::ReturnCode ret_code{resp->FromPb(resp_pb)};\n");
        fprintf(write, "        if (phxrpc::ReturnCode::OK != ret_code) {\n");
        fprintf(write, "            phxrpc::log(LOG_ERR, \"FromPb ip %%s err %%d\", req->GetClientIP(), static_cast<int>(ret_code));\n");
        fprintf(write, "\n");
        fprintf(write, "            return -ENOMEM;\n");
        fprintf(write, "        }\n");
        fprintf(write, "    }\n");
        fprintf(write, "\n");
    }
    fprintf(write, "    phxrpc::log(LOG_DEBUG, \"ret %%d\", ret);\n");
    fprintf(write, "\n");
    fprintf(write, "    return ret;\n");
    fprintf(write, "}\n");

    fprintf(write, "\n");
}

void ServiceCodeRender::GenerateDispatcherHttpFunc(SyntaxTree *stree,
                                                   const SyntaxFunc *const func,
                                                   FILE *write) {
    char dispatcher_name[128]{0}, type_name[128]{0};

    name_render_.GetDispatcherClassName(stree->GetName(), dispatcher_name, sizeof(dispatcher_name));

    fprintf(write, "int %s::%s(const phxrpc::BaseRequest *const req, "
            "phxrpc::BaseResponse *const resp) {\n",
            dispatcher_name, func->GetName());

    fprintf(write, "    dispatcher_args_->server_monitor->SvrCall(%d, \"%s\", 1);\n",
            func->GetCmdID(), func->GetName());
    fprintf(write, "\n");

    fprintf(write, "    int ret{0};\n");
    fprintf(write, "\n");

    name_render_.GetMessageClassName(func->GetReq()->GetType(), type_name, sizeof(type_name));
    fprintf(write, "    %s req_pb;\n", type_name);

    name_render_.GetMessageClassName(func->GetResp()->GetType(), type_name, sizeof(type_name));
    fprintf(write, "    %s resp_pb;\n", type_name);

    fprintf(write, "\n");

    fprintf(write, "    // unpack request\n");
    fprintf(write, "    {\n");

    fprintf(write, "        if (!req_pb.ParseFromString(req->GetContent())) {\n");

    fprintf(write, "            phxrpc::log(LOG_ERR, \"ERROR: FromBuffer fail size %%zu ip %%s\",\n"
            "                req->GetContent().size(), req->GetClientIP());\n");

    fprintf(write, "            return -EINVAL;\n");
    fprintf(write, "        }\n");
    fprintf(write, "    }\n");

    fprintf(write, "\n");

    fprintf(write, "    // logic process\n");
    fprintf(write, "    {\n");

    fprintf(write, "        if (0 == ret) ret = service_.%s(req_pb, &resp_pb);\n", func->GetName());

    fprintf(write, "    }\n\n");

    fprintf(write, "    // pack response\n");
    fprintf(write, "    {\n");
    fprintf(write, "        if (!resp_pb.SerializeToString(&(resp->GetContent()))) {\n");

    fprintf(write, "            phxrpc::log(LOG_ERR, \"ERROR: ToBuffer fail ip %%s\", req->GetClientIP());\n");

    fprintf(write, "            return -ENOMEM;\n");
    fprintf(write, "        }\n");
    fprintf(write, "    }\n");

    fprintf(write, "\n");

    fprintf(write, "    phxrpc::log(LOG_DEBUG, \"RETN: %s = %%d\", ret);\n", func->GetName());

    fprintf(write, "\n");
    fprintf(write, "    return ret;\n");
    fprintf(write, "}\n");
    fprintf(write, "\n");
}

