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

#include <cstdio>
#include <string>
#include <vector>


namespace phxrpc {


class NameRender;
class SyntaxTree;
class SyntaxFunc;
typedef std::vector<SyntaxFunc> SyntaxFuncVector;

class ClientCodeRender {
  public:
    ClientCodeRender(NameRender &name_render);
    virtual ~ClientCodeRender();

    void GenerateStubHpp(SyntaxTree *stree, const SyntaxFuncVector &mqtt_funcs,
                         FILE *write);

    void GenerateStubCpp(SyntaxTree *stree, const SyntaxFuncVector &mqtt_funcs,
                         FILE *write);

    void GenerateClientHpp(SyntaxTree *stree,
                           const SyntaxFuncVector &mqtt_funcs,
                           FILE *write, const bool is_uthread_mode);

    void GenerateClientCpp(SyntaxTree *stree,
                           const SyntaxFuncVector &mqtt_funcs,
                           FILE *write, const bool is_uthread_mode);

    void GenerateClientEtc(SyntaxTree *stree, FILE *write);

  private:
    void GetStubFuncDeclaration(SyntaxTree *stree, const SyntaxFunc *const func,
                                int is_header, std::string *result);

    void GenerateMqttStubFunc(SyntaxTree *stree, const SyntaxFunc *const func,
                              FILE *write);

    void GenerateStubFunc(SyntaxTree *stree, const SyntaxFunc *const func,
                          FILE *write);

    void GetClienfuncDeclaration(SyntaxTree *stree,
                                 const SyntaxFunc *const func,
                                 int is_header, std::string *result,
                                 const bool is_uthread_mode);

    NameRender &name_render_;
};


}

