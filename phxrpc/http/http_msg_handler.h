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

#include "phxrpc/msg/base_msg_handler.h"


namespace phxrpc {


class BaseTcpStream;

class HttpMessage;
class HttpRequest;
class HttpResponse;

class HttpMessageHandler : public BaseMessageHandler {
  public:
    HttpMessageHandler() = default;
    virtual ~HttpMessageHandler() override = default;

    virtual int RecvRequest(BaseTcpStream &socket, BaseRequest *&req) override;
    virtual int RecvResponse(BaseTcpStream &socket, BaseResponse *&resp) override;

    virtual int GenRequest(BaseRequest *&req) override;
    virtual int GenResponse(BaseResponse *&resp) override;

    virtual bool keep_alive() const override;

  private:
    std::string version_;
    bool keep_alive_{false};
};


}  // namespace phxrpc

