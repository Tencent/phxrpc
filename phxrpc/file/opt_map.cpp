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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "opt_map.h"


namespace phxrpc {


OptMap::OptMap(const char *optstring) {
    opt_string_ = strdup(optstring);
}

OptMap::~OptMap() {
    if (NULL != opt_string_)
        free(opt_string_);
}

bool OptMap::Parse(int argc, char * argv[]) {
    bool ret = true;

    int c = 0;

    while ((c = getopt(argc, argv, opt_string_)) != EOF) {
        if ('?' == c || ':' == c) {
            ret = false;
        } else {
            opt_[c].push_back((NULL == ::optarg) ? "" : ::optarg);
        }
    }

    for (int i = optind; i < argc; i++) {
        non_opt_.push_back(argv[i]);
    }

    return ret;
}

size_t OptMap::GetNonOptCount() {
    return non_opt_.size();
}

const char *OptMap::GetNonOpt(size_t index) {
    if (index > 0 && index < non_opt_.size()) {
        return non_opt_[index].c_str();
    }

    return NULL;
}

bool OptMap::Has(char c) const {
    const option_map_::const_iterator iter = opt_.find(c);

    return opt_.end() != iter;
}

size_t OptMap::Count(char c) const {
    const option_map_::const_iterator iter = opt_.find(c);

    return (opt_.end() != iter) ? iter->second.size() : 0;
}

const char *OptMap::Get(char c, size_t index) const {
    const option_map_::const_iterator iter = opt_.find(c);

    if (opt_.end() != iter) {
        if (index >= iter->second.size()) {
            return NULL;
        } else {
            return iter->second[index];
        }
    } else {
        return NULL;
    }
}

bool OptMap::GetInt(char c, int *val, size_t index) const {
    return GetInt32(c, val, index);
}

bool OptMap::GetInt32(char c, int *val, size_t index) const {
    const char *tmp = Get(c, index);

    if (tmp)
        *val = strtol(tmp, nullptr, 10);

    return nullptr != tmp;
}

bool OptMap::GetInt64(char c, int64_t *val, size_t index) const {
    const char *tmp = Get(c, index);

    if (tmp)
        *val = strtoll(tmp, nullptr, 10);

    return nullptr != tmp;
}

bool OptMap::GetUInt(char c, uint32_t *val, size_t index) const {
    return GetUInt32(c, val, index);
}

bool OptMap::GetUInt32(char c, uint32_t *val, size_t index) const {
    const char *tmp = Get(c, index);

    if (tmp)
        *val = strtoul(tmp, nullptr, 10);

    return nullptr != tmp;
}

bool OptMap::GetUInt64(char c, uint64_t *val, size_t index) const {
    const char *tmp = Get(c, index);

    if (tmp)
        *val = strtoull(tmp, nullptr, 10);

    return nullptr != tmp;
}


}

