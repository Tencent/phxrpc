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

#include "phxrpc/msg/common.h"
#include "phxrpc/network.h"


namespace google {

namespace protobuf {

class Message;

}  // protobuf

}  // google


namespace phxrpc {


class BaseMessage {
  public:
    BaseMessage();
    virtual ~BaseMessage();

    virtual int Send(BaseTcpStream &socket) const = 0;
    virtual int ToPb(google::protobuf::Message *const message) const = 0;
    virtual int FromPb(const google::protobuf::Message &message) = 0;
    virtual size_t size() const = 0;

    bool fake() const { return fake_; };

  protected:
    void set_fake(const bool fake) { fake_ = fake; }

  private:
    bool fake_{false};
};


class BaseResponse;

class BaseRequest : virtual public BaseMessage {
  public:
    BaseRequest();
    virtual ~BaseRequest() override;

    virtual BaseResponse *GenResponse() const = 0;
    virtual bool keep_alive() const = 0;
    virtual void set_keep_alive(const bool keep_alive) = 0;

    void set_uri(const char *uri);
    const char *uri() const;

  private:
    std::string uri_;
};


class BaseResponse : virtual public BaseMessage {
  public:
    enum class FakeReason {
        NONE = 0,
        DISPATCH_ERROR = 1
    };

    BaseResponse();
    virtual ~BaseResponse() override;

    virtual void SetFake(FakeReason reason) = 0;
    virtual int Modify(const bool keep_alive, const std::string &version) = 0;

    virtual int result() = 0;
    virtual void set_result(const int result) = 0;
};


}

