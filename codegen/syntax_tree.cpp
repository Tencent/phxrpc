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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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
    memset(opt_string_, 0, sizeof(opt_string_));
    memset(usage_, 0, sizeof(usage_));
    cmdid_ = -1;
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

void SyntaxFunc::SetCmdID(const int cmdid) {
    cmdid_ = cmdid;
}

int SyntaxFunc::GetCmdID() const {
    return cmdid_;
}

//====================================================================

SyntaxTree::SyntaxTree() {
    memset(prefix_, 0, sizeof(prefix_));
    memset(proto_file_, 0, sizeof(proto_file_));
    memset(package_name_, 0, sizeof(package_name_));
}

SyntaxTree::~SyntaxTree() {
}

void SyntaxTree::SetProtoFile(const char * proto_file) {
    strncpy(proto_file_, proto_file, sizeof(proto_file_) - 1);
}

const char *SyntaxTree::GetProtoFile() const {
    return proto_file_;
}

const char *SyntaxTree::GetPackageName() const {
    return package_name_;
}

void SyntaxTree::SetPackageName(const char *package_name) {
    strncpy(package_name_, package_name, sizeof(package_name_) - 1);
}

void SyntaxTree::SetPrefix(const char *prefix) {
    strncpy(prefix_, prefix, sizeof(prefix_) - 1);
    ToUpper(prefix_);
}

const char *SyntaxTree::GetPrefix() const {
    return prefix_;
}

const SyntaxFuncVector *SyntaxTree::GetFuncList() const {
    return &func_list_;
}

SyntaxFuncVector *SyntaxTree::GetFuncList() {
    return &func_list_;
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

