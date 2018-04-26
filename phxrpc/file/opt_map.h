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
#include <map>


namespace phxrpc {


class OptMap {
 public:
    OptMap(const char *optstring);

    virtual ~OptMap();

    bool Parse(int argc, char * argv[]);

    bool Has(char c) const;

    size_t Count(char c) const;

    const char *Get(char c, size_t index = 0) const;

    bool GetInt(char c, int *val, size_t index = 0) const;

    bool GetInt32(char c, int *val, size_t index = 0) const;

    bool GetInt64(char c, int64_t *val, size_t index = 0) const;

    bool GetUInt(char c, uint32_t *val, size_t index = 0) const;

    bool GetUInt32(char c, uint32_t *val, size_t index = 0) const;

    bool GetUInt64(char c, uint64_t *val, size_t index = 0) const;

    size_t GetNonOptCount();

    const char *GetNonOpt(size_t index);

 private:
    char *opt_string_;

    typedef std::map<char, std::vector<const char *>, std::less<char> > option_map_;
    option_map_ opt_;

    std::vector<std::string> non_opt_;
};


}

