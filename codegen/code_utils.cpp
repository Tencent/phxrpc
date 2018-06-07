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

#include <cstdarg>
#include <cstdio>

#include "code_utils.h"


using namespace std;


namespace phxrpc {


void StrTrim(string *str, const char *to_trim) {
    string::size_type start_pos = 0;
    string::size_type end_pos = 0;

    start_pos = str->find_first_not_of(to_trim);

    end_pos = str->find_last_not_of(to_trim);

    if (start_pos == string::npos || end_pos == string::npos) {
        *str = "";
    } else {
        *str = str->substr(start_pos, end_pos - start_pos + 1);
    }
}

void StrReplaceAll(string *haystack, string needle, string s) {
    string::size_type pos = 0;
    while ((pos = haystack->find(needle, pos)) != string::npos) {
        haystack->erase(pos, needle.length());
        haystack->insert(pos, s);
        pos += s.length();
    }
}

void StrAppendFormat(string *result, const char *fmt, ...) {
    if (nullptr == fmt)
        return;

    size_t len = 0;
    {
        va_list va;
        va_start(va, fmt);
        len = vsnprintf(NULL, 0, fmt, va);
        va_end(va);
    }

    if (len > 0) {
        size_t old_len = result->size();
        result->resize(old_len + len);

        va_list va;
        va_start(va, fmt);
        vsnprintf(((char*) result->c_str()) + old_len, len + 1, fmt, va);
        va_end(va);
    }
}


}  // namespace phxrpc

