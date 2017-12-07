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
#include <cstdlib>
#include <string>


namespace phxrpc {


class SyntaxTree;

class NameRender {
  public:
    NameRender(const char *prefix);
    virtual ~NameRender();

    virtual const char *GetPrefix(char *dest, int size);

    virtual const char *GetMessageClasname(const char *type, char *name, int size);
    virtual const char *GetMessageFileName(const char *name, char *dest, int size);

    //================================================================

    virtual const char *GetStubClasname(const char *name, char *dest, int size);
    virtual const char *GetStubFileName(const char *name, char *dest, int size);

    virtual const char *GetClientClasname(const char *name, char *dest, int size);

    virtual const char *GetClientClasnameLower(const char *name, char *dest, int size);

    virtual const char *GetClientFileName(const char *name, char *dest, int size);

    virtual const char *GetClientEtcFileName(const char *name, char *dest, int size);

    //================================================================

    virtual const char *GetServiceClasname(const char *name, char *dest, int size);
    virtual const char *GetServiceFileName(const char *name, char *dest, int size);

    virtual const char *GetServiceImplClasname(const char *name, char *dest, int size);
    virtual const char *GetServiceImplFileName(const char *name, char *dest, int size);

    virtual const char *GetDispatcherClasname(const char *name, char *dest, int size);
    virtual const char *GetDispatcherFileName(const char *name, char *dest, int size);

    //================================================================

    virtual const char *GetServerConfigClasname(const char *name, char *dest, int size);
    virtual const char *GetServerConfigFileName(const char *name, char *dest, int size);

    virtual const char *GetServerEtcFileName(const char *name, char *dest, int size);

    virtual const char *GetServerMainFileName(const char *name, char *dest, int size);

    //================================================================

    virtual const char *GetToolClasname(const char *name, char *dest, int size);
    virtual const char *GetToolFileName(const char *name, char *dest, int size);

    virtual const char *GetToolImplClasname(const char *name, char *dest, int size);
    virtual const char *GetToolImplFileName(const char *name, char *dest, int size);

    virtual const char *GetToolMainFileName(const char *name, char *dest, int size);

    virtual void GetCopyright(const char *tool_name, const char *proto_file,
                              std::string *result, bool dont_edit = true,
                              const char *comment_prefix = "");

    static char *ToLower(register char *s);
    static char *ToUpper(register char *s);

  protected:
    char prefix_[128];
};


}

