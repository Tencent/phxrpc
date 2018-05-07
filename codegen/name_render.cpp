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
#include <cstring>
#include <sstream>

#include "name_render.h"
#include "syntax_tree.h"

#include "code_utils.h"


using namespace phxrpc;
using namespace std;


NameRender::NameRender(const char *prefix) {
    memset(prefix_, 0, sizeof(prefix_));
    strncpy(prefix_, prefix, sizeof(prefix_) - 1);
}

NameRender::~NameRender() {
}

const char *NameRender::GetPrefix(char *dest, int size) {
    snprintf(dest, size, "%s", prefix_);

    ToUpper(dest);

    return dest;
}

const char *NameRender::GetMessageClassName(const char *type, char *name, int size) {
    string tmp = type;

    phxrpc::StrReplaceAll(&tmp, ".", "::");

    snprintf(name, size, "%s", tmp.c_str());

    return name;
}

const char *NameRender::GetMessageFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%s", prefix_, name);

    char *pos{strrchr(dest, '.')};
    if (nullptr != pos)
        *pos = '\0';

    strncat(dest, ".pb", size);

    ToLower(dest);

    return dest;
}

char *NameRender::ToLower(char *s) {
    char *ret = s;

    for (; *s != '\0'; ++s)
        *s = tolower(*s);

    return ret;
}

char *NameRender::ToUpper(char *s) {
    char * ret = s;

    for (; *s != '\0'; s++)
        *s = toupper(*s);

    return ret;
}

const char *NameRender::GetStubClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sStub", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetCallerClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sCaller", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetStubFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "phxrpc_%s%c%s_stub", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetClientClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sClient", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetClientClassNameLower(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sClient", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetClientFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%s_client", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetClientEtcFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%s_client.conf", prefix_, name);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetServerConfigClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sServerConfig", prefix_, toupper(*name), name + 1);
    return dest;
}

const char *NameRender::GetServerConfigFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%s_server_config", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetServerEtcFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%s_server.conf", prefix_, name);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetServerMainFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%s_main", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetToolClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sTool", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetToolFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "phxrpc_%s%s_tool", prefix_, name);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetToolImplClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sToolImpl", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetToolImplFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%s_tool_impl", prefix_, name);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetToolMainFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%s_tool_main", prefix_, name);

    ToLower(dest);

    return dest;
}

void NameRender::GetCopyright(const char *tool_name, const char *proto_file,
                              string *result, bool dont_edit,
                              const char *comment_prefix) {
    ostringstream tmp;

    tmp << comment_prefix << endl;
    tmp << comment_prefix << " Generated by " << tool_name << " from " << proto_file << endl;

    if (dont_edit) {
        tmp << comment_prefix << endl;
        tmp << comment_prefix << " Please DO NOT edit unless you know exactly what you are doing.\n";
    }
    tmp << comment_prefix << endl;

    *result = tmp.str();
}

const char *NameRender::GetServiceClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sService", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetServiceFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "phxrpc_%s%c%s_service", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetServiceImplClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sServiceImpl", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetServiceImplFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%s_service_impl", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

const char *NameRender::GetDispatcherClassName(const char *name, char *dest, int size) {
    snprintf(dest, size, "%s%c%sDispatcher", prefix_, toupper(*name), name + 1);

    return dest;
}

const char *NameRender::GetDispatcherFileName(const char *name, char *dest, int size) {
    snprintf(dest, size, "phxrpc_%s%c%s_dispatcher", prefix_, toupper(*name), name + 1);

    ToLower(dest);

    return dest;
}

