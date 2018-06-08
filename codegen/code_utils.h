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

#include <string>


namespace phxrpc {


void StrTrim(std::string *str, const char *to_trim = "\t\r\n");

void StrReplaceAll(std::string *haystack, std::string needle, std::string s);

void StrAppendFormat(std::string *result, const char *fmt, ...) __attribute__((format(printf, 2, 3)));


}

