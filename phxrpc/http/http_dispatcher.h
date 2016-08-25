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

#include <map>
#include <string>

#include "http_msg.h"

namespace phxrpc {

template<typename Dispatcher>
class HttpDispatcher {
 public:
    typedef int (Dispatcher::*URIFunc_t)(const HttpRequest & request, HttpResponse * response);

    typedef std::map<std::string, URIFunc_t> URIFuncMap;

 public:
    HttpDispatcher(Dispatcher & dispatcher, const URIFuncMap & uri_func_map)
            : dispatcher_(dispatcher),
              uri_func_map_(uri_func_map) {
    }

    ~HttpDispatcher() {
    }

    bool Dispatch(const HttpRequest & request, HttpResponse * response) {
        int ret = -1;

        typename URIFuncMap::const_iterator iter = uri_func_map_.find(request.GetURI());

        if (uri_func_map_.end() != iter) {
            ret = (dispatcher_.*iter->second)(request, response);
        }

        response->AddHeader(HttpMessage::HEADER_X_PHXRPC_RESULT, ret);

        return uri_func_map_.end() != iter;;
    }

 private:
    Dispatcher & dispatcher_;
    const URIFuncMap & uri_func_map_;
};

}

