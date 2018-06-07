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

#include <vector>
#include <string>


namespace phxrpc {


enum {
    _SYNTAX_NAME_LEN = 512,
    _SYNTAX_DESC_LEN = 512,
    _SYNTAX_TYPE_LEN = 512
};


class SyntaxNode {
  public:
    SyntaxNode();
    virtual ~SyntaxNode();

    void SetName(const char *name);
    const char *GetName() const;

  private:
    char name_[_SYNTAX_NAME_LEN];
};

class SyntaxParam : public SyntaxNode {
  public:
    SyntaxParam();
    virtual ~SyntaxParam() override;

    void SetType(const char *type);
    const char *GetType() const;

  private:
    char type_[_SYNTAX_TYPE_LEN];
};

class SyntaxFunc : public SyntaxNode {
  public:
    SyntaxFunc();
    virtual ~SyntaxFunc() override;

    const SyntaxParam *GetReq() const;

    const SyntaxParam *GetResp() const;

    SyntaxParam *GetReq();

    SyntaxParam *GetResp();

    void SetOptString(const char *opt_string);
    const char *GetOptString() const;

    void SetUsage(const char *usage);
    const char *GetUsage() const;

    void SetCmdID(const int cmdid);
    int GetCmdID() const;

  private:
    SyntaxParam req_;
    SyntaxParam resp_;
    char opt_string_[_SYNTAX_DESC_LEN];
    char usage_[_SYNTAX_DESC_LEN];
    int cmdid_;
};

//------------------------------------------------------------

typedef std::vector<SyntaxFunc> SyntaxFuncVector;

class SyntaxTree;

typedef std::vector<SyntaxTree *> SyntaxTreeVector;

class SyntaxTree : public SyntaxNode {
  public:
    SyntaxTree();
    virtual ~SyntaxTree() override;

    void SetProtoFile(const char *proto_file);
    const char *GetProtoFile() const;

    void SetPrefix(const char *prefix);
    const char *GetPrefix() const;

    void SetPackageName(const char *package_name);
    const char *GetPackageName() const;

    const SyntaxFuncVector *GetFuncList() const;
    SyntaxFuncVector *GetFuncList();

    SyntaxFunc *FindFunc(const char *name);

    void Print();

    static char *ToLower(register char *s);
    static char *ToUpper(register char *s);

  private:
    char proto_file_[128];
    char prefix_[32];
    char package_name_[128];

    SyntaxFuncVector func_list_;
};


}

