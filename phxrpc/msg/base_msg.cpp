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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base_msg.h"


namespace phxrpc {


using namespace std;


BaseMessage::BaseMessage() {
}

BaseMessage::~BaseMessage() {
}

void BaseMessage::SetVersion(const char *version) {
    snprintf(version_, sizeof(version_), "%s", version);
}

const char *BaseMessage::GetVersion() const {
    return version_;
}

void BaseMessage::SetClientIP(const char *client_ip) {
    if (client_ip != nullptr) {
        snprintf(client_ip_, sizeof(client_ip_), "%s", client_ip);
    }
}

const char *BaseMessage::GetClientIP() const {
    return client_ip_;
}

void BaseMessage::AppendContent(const void *content, const int length, const int max_length) {
    int valid_length{length};
    if (valid_length <= 0)
        valid_length = strlen((char *)content);

    int total = content_.size() + valid_length;
    total = total > max_length ? total : max_length;

    //content_.reserve(total);

    content_.append((char *) content, valid_length);
}

void BaseMessage::SetContent(const void *content, const int length) {
    content_.clear();
    content_.append((char *)content, length);
}

const string &BaseMessage::GetContent() const {
    return content_;
}

string &BaseMessage::GetContent() {
    return content_;
}


BaseRequest::BaseRequest(const BaseMessage::Protocol protocol) {
    set_direction(BaseMessage::Direction::REQUEST);
    set_protocol(protocol);
}

BaseRequest::~BaseRequest() {
}

void BaseRequest::SetURI(const char *uri) {
    if (nullptr != uri) {
        uri_ = string(uri);
    }
}

const char *BaseRequest::GetURI() const {
    return uri_.c_str();
}


BaseResponse::BaseResponse(const BaseMessage::Protocol protocol) {
    set_direction(BaseMessage::Direction::RESPONSE);
    set_protocol(protocol);
}

BaseResponse::~BaseResponse() {}


}

