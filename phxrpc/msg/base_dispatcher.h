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

#include "base_msg.h"


namespace phxrpc {


template<typename Dispatcher>
class BaseDispatcher {
  public:
    typedef int (Dispatcher::*MqttFunc_t)(const BaseRequest *const req,
                                          BaseResponse *const resp);
    typedef int (Dispatcher::*URIFunc_t)(const BaseRequest *const req,
                                         BaseResponse *const resp);

    typedef std::map<BaseMessage::Protocol, MqttFunc_t> MqttFuncMap;
    typedef std::map<std::string, URIFunc_t> URIFuncMap;

    BaseDispatcher(Dispatcher &dispatcher, const MqttFuncMap &mqtt_func_map,
                   const URIFuncMap &uri_func_map)
            : dispatcher_(dispatcher), mqtt_func_map_(mqtt_func_map),
              uri_func_map_(uri_func_map) {
    }

    virtual ~BaseDispatcher() = default;

    bool Dispatch(const BaseRequest *const req, BaseResponse *const resp) {
        int ret{-1};

        if (BaseMessage::Protocol::HTTP_POST == req->protocol() ||
            BaseMessage::Protocol::HTTP_GET == req->protocol() ||
            BaseMessage::Protocol::HTTP_HEAD == req->protocol()) {
            typename URIFuncMap::const_iterator iter(uri_func_map_.find(req->GetURI()));

            if (uri_func_map_.end() != iter) {
                ret = (dispatcher_.*iter->second)(req, resp);
            }

            resp->SetPhxRpcResult(ret);

            return uri_func_map_.end() != iter;
        } else {
            typename MqttFuncMap::const_iterator iter(mqtt_func_map_.find(req->protocol()));

            if (mqtt_func_map_.end() != iter) {
                ret = (dispatcher_.*iter->second)(req, resp);
            }

            resp->SetPhxRpcResult(ret);

            return mqtt_func_map_.end() != iter;
        }

        return ret;
    }

  private:
    Dispatcher &dispatcher_;
    const MqttFuncMap &mqtt_func_map_;
    const URIFuncMap &uri_func_map_;
};


}  // namespace phxrpc

