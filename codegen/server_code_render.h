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

#include <stdio.h>
#include <string>

namespace phxrpc {

class NameRender;
class SyntaxTree;

class ServerCodeRender {
  public:
    ServerCodeRender(NameRender &name_render);
    virtual ~ServerCodeRender();

    void GenerateServerConfigHpp(SyntaxTree *stree, FILE *write);

    void GenerateServerConfigCpp(SyntaxTree *stree, FILE *write);

    void GenerateServerMainCpp(SyntaxTree *stree, FILE *write, const bool is_uthread_mode);

    void GenerateServerEtc(SyntaxTree *stree, FILE *write, const bool is_uthread_mode);

    void GenerateMakefile(SyntaxTree *stree, const std::string &mk_dir_path,
                          FILE *write, const bool is_uthread_mode);

  private:
    NameRender & name_render_;
};

}

