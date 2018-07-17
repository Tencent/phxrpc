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
#include <errno.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "proto_utils.h"


using namespace google::protobuf::compiler;
using namespace google::protobuf;

using namespace phxrpc;
using namespace std;


class MyErrorPrinter : public MultiFileErrorCollector {
  public:
    MyErrorPrinter() {
    }
    virtual ~MyErrorPrinter() override {
    }

    void AddError(const std::string& file_name, int line, int column, const std::string& message) {

        fprintf(stderr, "%s", file_name.c_str());

        if (line != -1)
            fprintf(stderr, ":%d:%d", line + 1, column + 1);

        fprintf(stderr, ": %s\n", message.c_str());
    }
};

int ProtoUtils::Parse(const char *file, SyntaxTree *stree, const vector<string> &include_list) {
    return Parse(file, stree, nullptr, include_list);
}

int ProtoUtils::Parse(const char *file, SyntaxTree *stree, map<string, bool> *parsed_file_map,
                      const vector<string> &include_list) {

    DiskSourceTree tree;
    tree.MapPath("", "./");
    for (auto & include_path : include_list) {
        tree.MapPath("", include_path);
    }

    int ret{LoadNormal(file, stree, parsed_file_map, tree)};

    if (0 == ret)
        ret = LoadExtension(file, stree, tree);

    if (0 == ret)
        AddEcho(stree);

    return ret;
}

int ProtoUtils::LoadNormal(const char *file_name, SyntaxTree *stree, map<string, bool> *parsed_file_map,
                           DiskSourceTree &tree) {
    MyErrorPrinter error;

    Importer importer(&tree, &error);

    const FileDescriptor *fd{importer.Import(file_name)};

    stree->set_package_name(fd->package().c_str());

    stree->set_proto_file(file_name);

    for (int i{0}; 1 > i && fd->service_count() > i; ++i) {
        const ServiceDescriptor * iter = fd->service(i);
        stree->SetName(iter->name().c_str());

        for (int j{0}; iter->method_count() > j; ++j) {
            const MethodDescriptor *method{iter->method(j)};

            SyntaxFunc func;
            func.SetName(method->name().c_str());

            const Descriptor * input_type = method->input_type();
            const Descriptor * output_type = method->output_type();

            func.GetReq()->SetName("Req");
            func.GetReq()->SetType(input_type->full_name().c_str());

            func.GetResp()->SetName("Resp");
            func.GetResp()->SetType(output_type->full_name().c_str());

            stree->mutable_func_list()->push_back(func);
        }
    }

    return 0;
}

int ProtoUtils::LoadExtension(const char *file_name, SyntaxTree *stree, DiskSourceTree &tree) {
    MyErrorPrinter error;

    SourceTreeDescriptorDatabase db(&tree);
    db.RecordErrorsTo(&error);

    FileDescriptorProto fd_proto;

    db.FindFileByName(file_name, &fd_proto);

    for (int i{0}; 1 > i && fd_proto.service_size() > i; ++i) {
        const ServiceDescriptorProto &svc(fd_proto.service(i));

        for (int j{0}; svc.method_size() > j; ++j) {
            const MethodDescriptorProto &method(svc.method(j));

            SyntaxFunc *func{stree->FindFunc(method.name().c_str())};

            assert(nullptr != func);

            const MethodOptions &options(method.options());

            for (int k{0}; options.uninterpreted_option_size() > k; ++k) {
                const UninterpretedOption &opt(options.uninterpreted_option(k));

                if (opt.name_size() > 0) {
                    if (nullptr != strstr(opt.name(0).name_part().c_str(), "CmdID")) {
                        func->SetCmdID(opt.positive_int_value());
                    }

                    if (nullptr != strstr(opt.name(0).name_part().c_str(), "OptString")) {
                        func->SetOptString(opt.string_value().c_str());
                    }

                    if (nullptr != strstr(opt.name(0).name_part().c_str(), "Usage")) {
                        func->SetUsage(opt.string_value().c_str());
                    }
                }
            }
        }
    }

    return 0;
}

int ProtoUtils::AddEcho(SyntaxTree *stree) {
    char name[256]{0};

    snprintf(name, sizeof(name), "google.protobuf.StringValue");

    // always add a echo function
    {
        SyntaxFunc echo_func;
        echo_func.SetName("PHXEcho");
        echo_func.GetReq()->SetName("req");
        echo_func.GetReq()->SetType(name);
        echo_func.GetResp()->SetName("resp");
        echo_func.GetResp()->SetType(name);
        echo_func.SetOptString("s:");
        echo_func.SetUsage("-s <string>");

        stree->mutable_func_list()->insert(stree->mutable_func_list()->begin(), echo_func);
    }

    return 0;
}

