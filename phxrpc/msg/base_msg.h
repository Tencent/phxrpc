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

#include "phxrpc/network.h"

#include "common.h"


namespace google {

namespace protobuf {

class Message;

}  // protobuf

}  // google


namespace phxrpc {


class BaseMessage {
  public:
    enum class Direction {
        NONE = 0,
        REQUEST,
        RESPONSE,
        MAX,
    };

    enum class Protocol {
        NONE = 0,
        HTTP_GET = 101,
        HTTP_POST = 102,
        HTTP_HEAD = 103,
        MQTT_CONNECT = 201,
        MQTT_PUBLISH = 202,
        MQTT_PUBREL = 203,
        MQTT_SUBSCRIBE = 204,
        MQTT_UNSUBSCRIBE = 205,
        MQTT_PING = 206,
        MQTT_DISCONNECT = 207,
        MQTT_FAKE_DISCONNACK = 208,
        MAX,
    };

    BaseMessage();
    virtual ~BaseMessage();

    virtual ReturnCode Send(BaseTcpStream &socket) const = 0;
    virtual ReturnCode ToPb(google::protobuf::Message *const message) const = 0;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) = 0;

    void SetVersion(const char *version);
    const char *GetVersion() const;

    void SetClientIP(const char *client_ip);
    const char *GetClientIP() const;

    void AppendContent(const void *content, const int length = 0, const int max_length = 0);
    void SetContent(const void *content, const int length = 0);
    const std::string &GetContent() const;
    std::string &GetContent();

    Direction direction() const { return direction_; }
    Protocol protocol() const { return protocol_; }
    bool fake() const { return fake_; };

  protected:
    void set_direction(const Direction direction) { direction_ = direction; }
    void set_protocol(const Protocol protocol) { protocol_ = protocol; }
    void set_fake(const bool fake) { fake_ = fake; }

    char client_ip_[16];

  private:
    Direction direction_{Direction::NONE};
    Protocol protocol_{Protocol::NONE};
    char version_[16];
    std::string content_;
    bool fake_{false};
};


class BaseResponse;

class BaseRequest : virtual public BaseMessage {
  public:
    BaseRequest(const Protocol protocol);
    virtual ~BaseRequest() override;

    void SetURI(const char *uri);
    const char *GetURI() const;

    virtual BaseResponse *GenResponse() const = 0;
    virtual int IsKeepAlive() const = 0;

  private:
    std::string uri_;
};


class BaseResponse : virtual public BaseMessage {
  public:
    BaseResponse(const Protocol protocol);
    virtual ~BaseResponse() override;

    virtual void SetPhxRpcResult(const int result) = 0;
    virtual void DispatchErr() = 0;
    virtual ReturnCode ModifyResp(const bool keep_alive, const std::string &version) = 0;
};


}

