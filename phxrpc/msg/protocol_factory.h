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


namespace phxrpc {


class BaseRequest;
class BaseProtocol;


class BaseProtocolFactory {
  public:
    BaseProtocolFactory() = default;
    virtual ~BaseProtocolFactory() = default;

    static BaseProtocolFactory *CreateFactory(UThreadTcpStream &in_stream);

    virtual BaseProtocol *GenProtocol() = 0;
};


class HttpProtocolFactory : public BaseProtocolFactory {
  public:
    HttpProtocolFactory() = default;
    virtual ~HttpProtocolFactory() = default;

    virtual BaseProtocol *GenProtocol() override;
};


class MqttProtocolFactory : public BaseProtocolFactory {
  public:
    MqttProtocolFactory() = default;
    virtual ~MqttProtocolFactory() = default;

    virtual BaseProtocol *GenProtocol() override;
};


}

