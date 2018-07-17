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

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "code_utils.h"
#include "syntax_tree.h"


using namespace phxrpc;
using namespace std;


SyntaxNode::SyntaxNode() {
    memset(name_, 0, sizeof(name_));
}

SyntaxNode::~SyntaxNode() {
}

void SyntaxNode::SetName(const char *name) {
    strncpy(name_, name, sizeof(name_) - 1);
}

const char *SyntaxNode::GetName() const {
    return name_;
}

//====================================================================

SyntaxParam::SyntaxParam() {
    memset(type_, 0, sizeof(type_));
}

SyntaxParam::~SyntaxParam() {
}

void SyntaxParam::SetType(const char *type) {
    strncpy(type_, type, sizeof(type_) - 1);
}

const char *SyntaxParam::GetType() const {
    return type_;
}

//====================================================================

SyntaxFunc::SyntaxFunc() {
    cmdid_ = -1;
    memset(opt_string_, 0, sizeof(opt_string_));
    memset(usage_, 0, sizeof(usage_));
}

SyntaxFunc::~SyntaxFunc() {
}

const SyntaxParam *SyntaxFunc::GetReq() const {
    return &req_;
}

const SyntaxParam *SyntaxFunc::GetResp() const {
    return &resp_;
}

SyntaxParam *SyntaxFunc::GetReq() {
    return &req_;
}

SyntaxParam *SyntaxFunc::GetResp() {
    return &resp_;
}

void SyntaxFunc::SetCmdID(const int cmdid) {
    cmdid_ = cmdid;
}

int SyntaxFunc::GetCmdID() const {
    return cmdid_;
}

void SyntaxFunc::SetOptString(const char *opt_string) {
    strncpy(opt_string_, opt_string, sizeof(opt_string_));
}

const char *SyntaxFunc::GetOptString() const {
    return opt_string_;
}

void SyntaxFunc::SetUsage(const char *usage) {
    strncpy(usage_, usage, sizeof(usage_));
}

const char *SyntaxFunc::GetUsage() const {
    return usage_;
}

//====================================================================

char *SyntaxTree::ToLower(char *s) {
    char *ret = s;

    for (; *s != '\0'; ++s)
        *s = tolower(*s);

    return ret;
}

char *SyntaxTree::ToUpper(char *s) {
    char *ret = s;

    for (; *s != '\0'; s++)
        *s = toupper(*s);

    return ret;
}

string SyntaxTree::Pb2CppPackageName(const string &pb_package_name) {
    string cpp_package_name(pb_package_name);
    StrReplaceAll(&cpp_package_name, ".", "::");
    return cpp_package_name;
}

string SyntaxTree::Cpp2PbPackageName(const string &cpp_package_name) {
    string pb_package_name(cpp_package_name);
    StrReplaceAll(&pb_package_name, "::", ".");
    return pb_package_name;
}

string SyntaxTree::Pb2UriPackageName(const std::string &cpp_package_name) {
    string uri_package_name(cpp_package_name);
    StrReplaceAll(&uri_package_name, ".", "/");
    return uri_package_name;
}

string SyntaxTree::Uri2PbPackageName(const std::string &uri_package_name) {
    string cpp_package_name(uri_package_name);
    StrReplaceAll(&cpp_package_name, "/", ".");
    return cpp_package_name;
}

SyntaxTree::SyntaxTree() {
    memset(prefix_, 0, sizeof(prefix_));
    memset(proto_file_, 0, sizeof(proto_file_));
    memset(pb_package_name_, 0, sizeof(pb_package_name_));
}

SyntaxTree::~SyntaxTree() {
}

SyntaxFunc *SyntaxTree::FindFunc(const char *name) {
    SyntaxFunc *ret{nullptr};

    for (SyntaxFuncVector::iterator iter(func_list_.begin()); func_list_.end() != iter; ++iter) {
        if (0 == strcasecmp(name, iter->GetName())) {
            ret = &(*iter);
            break;
        }
    }

    return ret;
}

void SyntaxTree::Print() {
}

const char *SyntaxTree::proto_file() const {
    return proto_file_;
}

void SyntaxTree::set_proto_file(const char *proto_file) {
    strncpy(proto_file_, proto_file, sizeof(proto_file_) - 1);
}

const char *SyntaxTree::package_name() const {
    return pb_package_name_;
}

void SyntaxTree::set_package_name(const char *pb_package_name) {
    strncpy(pb_package_name_, pb_package_name, sizeof(pb_package_name_) - 1);
}

const char *SyntaxTree::prefix() const {
    return prefix_;
}

void SyntaxTree::set_prefix(const char *prefix) {
    strncpy(prefix_, prefix, sizeof(prefix_) - 1);
    ToUpper(prefix_);
}

const SyntaxFuncVector *SyntaxTree::func_list() const {
    return &func_list_;
}

SyntaxFuncVector *SyntaxTree::mutable_func_list() {
    return &func_list_;
}

