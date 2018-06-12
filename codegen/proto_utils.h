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

#pragma once

#include <map>
#include <string>
#include <vector>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/compiler/command_line_interface.h>

#include "syntax_tree.h"


namespace phxrpc {


class ProtoUtils {
  public:
    static int Parse(const char *file, phxrpc::SyntaxTree *stree,
                     const std::vector<std::string> &include_list);
    static int Parse(const char *file, phxrpc::SyntaxTree *stree,
                     std::map<std::string, bool> *parsed_file_map,
                     const std::vector<std::string> & include_list);

  private:
    static int LoadNormal(const char *file_name, phxrpc::SyntaxTree *stree,
                          std::map<std::string, bool> *parsed_file_map,
                          google::protobuf::compiler::DiskSourceTree & tree);

    static int LoadExtension(const char *file_name, phxrpc::SyntaxTree *stree,
                             google::protobuf::compiler::DiskSourceTree & tree);

    static int AddEcho(phxrpc::SyntaxTree *stree);

    ProtoUtils();
    virtual ~ProtoUtils();
};


}

