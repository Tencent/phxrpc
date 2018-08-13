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

#include "phxrpc/http/http_msg.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "phxrpc/http/http_protocol.h"
#include "phxrpc/rpc/phxrpc.pb.h"


namespace phxrpc {


using namespace std;


const char *HttpMessage::HEADER_CONTENT_LENGTH = "Content-Length";
const char *HttpMessage::HEADER_CONTENT_TYPE = "Content-Type";
const char *HttpMessage::HEADER_CONNECTION = "Connection";
const char *HttpMessage::HEADER_PROXY_CONNECTION = "Proxy-Connection";
const char *HttpMessage::HEADER_TRANSFER_ENCODING = "Transfer-Encoding";
const char *HttpMessage::HEADER_DATE = "Date";
const char *HttpMessage::HEADER_SERVER = "Server";

const char *HttpMessage::HEADER_X_PHXRPC_RESULT = "X-PHXRPC-Result";


int HttpMessage::ToPb(google::protobuf::Message *const message) const {
    if (!message->ParseFromString(content()))
        return -1;

    return 0;
}

int HttpMessage::FromPb(const google::protobuf::Message &message) {
    if (!message.SerializeToString(mutable_content()))
        return -1;

    return 0;
}

size_t HttpMessage::size() const {
    return content().size();
}

void HttpMessage::AddHeader(const char *name, const char *value) {
    header_name_list_.push_back(name);
    header_value_list_.push_back(value);
}

void HttpMessage::AddHeader(const char *name, int value) {
    char tmp[32]{0};
    snprintf(tmp, sizeof(tmp), "%d", value);

    AddHeader(name, tmp);
}

bool HttpMessage::RemoveHeader(const char *name) {
    bool ret{false};

    for (size_t i{0}; header_name_list_.size() > i && false == ret; ++i) {
        if (0 == strcasecmp(name, header_name_list_[i].c_str())) {
            header_name_list_.erase(header_name_list_.begin() + i);
            header_value_list_.erase(header_value_list_.begin() + i);
            ret = true;
        }
    }

    return ret;
}

size_t HttpMessage::GetHeaderCount() const {
    return header_name_list_.size();
}

const char *HttpMessage::GetHeaderName(size_t index) const {
    return index < header_name_list_.size() ? header_name_list_[index].c_str() : nullptr;
}

const char *HttpMessage::GetHeaderValue(size_t index) const {
    return index < header_value_list_.size() ? header_value_list_[index].c_str() : nullptr;
}

const char *HttpMessage::GetHeaderValue(const char *name) const {
    const char *value{nullptr};

    for (size_t i{0}; header_name_list_.size() > i && nullptr == value; ++i) {
        if (0 == strcasecmp(name, header_name_list_[i].c_str())) {
            value = header_value_list_[i].c_str();
        }
    }

    return value;
}

void HttpMessage::AppendContent(const void *content, const int length, const int max_length) {
    int valid_length{length};
    if (valid_length <= 0)
        valid_length = strlen((char *)content);

    int total = content_.size() + valid_length;
    total = total > max_length ? total : max_length;

    //content_.reserve(total);

    content_.append((char *) content, valid_length);
}

const string &HttpMessage::content() const {
    return content_;
}

void HttpMessage::set_content(const char *const content, const int length) {
    content_.clear();
    content_.append(content, length);
}

string *HttpMessage::mutable_content() {
    return &content_;
}

const char *HttpMessage::version() const {
    return version_;
}

void HttpMessage::set_version(const char *version) {
    snprintf(version_, sizeof(version_), "%s", version);
}



//---------------------------------------------------------

HttpRequest::HttpRequest() {
    set_version("HTTP/1.0");
    set_direction(Direction::REQUEST);
    memset(method_, 0, sizeof(method_));
}

HttpRequest::~HttpRequest() {
}

int HttpRequest::Send(BaseTcpStream &socket) const {
    int ret{HttpProtocol::SendReqHeader(socket, "POST", *this)};

    if (0 == ret) {
        socket << content();
        if (!socket.flush().good())
            ret = static_cast<int>(socket.LastError());
    }

    return ret;
}

BaseResponse *HttpRequest::GenResponse() const {
    return new HttpResponse;
}

bool HttpRequest::keep_alive() const {
    const char *proxy{GetHeaderValue(HEADER_PROXY_CONNECTION)};
    const char *local{GetHeaderValue(HEADER_CONNECTION)};

    if ((nullptr != proxy && 0 == strcasecmp(proxy, "Keep-Alive"))
        || (nullptr != local && 0 == strcasecmp(local, "Keep-Alive"))) {
        return true;
    }

    return false;
}

void HttpRequest::set_keep_alive(const bool keep_alive) {
    if (keep_alive) {
        AddHeader(HttpMessage::HEADER_CONNECTION, "Keep-Alive");
    } else {
        AddHeader(HttpMessage::HEADER_CONNECTION, "");
    }
}

void HttpRequest::AddParam(const char *name, const char *value) {
    param_name_list_.push_back(name);
    param_value_list_.push_back(value);
}

bool HttpRequest::RemoveParam(const char *name) {
    bool ret{false};

    for (size_t i{0}; param_name_list_.size() > i && false == ret; ++i) {
        if (0 == strcasecmp(name, param_name_list_[i].c_str())) {
            param_name_list_.erase(param_name_list_.begin() + i);
            param_value_list_.erase(param_value_list_.begin() + i);
            ret = true;
        }
    }

    return ret;
}

size_t HttpRequest::GetParamCount() const {
    return param_name_list_.size();
}

const char *HttpRequest::GetParamName(size_t index) const {
    return index < param_name_list_.size() ? param_name_list_[index].c_str() : nullptr;
}

const char *HttpRequest::GetParamValue(size_t index) const {
    return index < param_value_list_.size() ? param_value_list_[index].c_str() : nullptr;
}

const char *HttpRequest::GetParamValue(const char *name) const {
    const char *value{nullptr};

    for (size_t i{0}; param_name_list_.size() > i && nullptr == value; ++i) {
        if (0 == strcasecmp(name, param_name_list_[i].c_str())) {
            value = param_value_list_[i].c_str();
        }
    }

    return value;
}

int HttpRequest::IsMethod(const char *method) const {
    return 0 == strcasecmp(method, method_);
}

const char *HttpRequest::method() const {
    return method_;
}

void HttpRequest::set_method(const char *method) {
    if (nullptr != method) {
        snprintf(method_, sizeof(method_), "%s", method);
    }
}



//---------------------------------------------------------

HttpResponse::HttpResponse() {
    set_version("HTTP/1.0");
    set_direction(Direction::RESPONSE);
    status_code_ = 200;
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", "OK");
}

HttpResponse::~HttpResponse() {
}

int HttpResponse::Send(BaseTcpStream &socket) const {
    socket << version() << " " << status_code() << " " << reason_phrase() << "\r\n";

    for (size_t i{0}; GetHeaderCount() > i; ++i) {
        socket << GetHeaderName(i) << ": " << GetHeaderValue(i) << "\r\n";
    }

    if (content().size() > 0) {
        if (nullptr == GetHeaderValue(HttpMessage::HEADER_CONTENT_LENGTH)) {
            socket << HttpMessage::HEADER_CONTENT_LENGTH << ": " << content().size() << "\r\n";
        }
    }

    socket << "\r\n";

    if (content().size() > 0)
        socket << content();

    if (socket.flush().good()) {
        return 0;
    } else {
        return static_cast<int>(socket.LastError());
    }
}

void HttpResponse::SetFake(FakeReason reason) {
  switch (reason) {
    case FakeReason::DISPATCH_ERROR:
      set_status_code(404);
      set_reason_phrase("Not Found");
      break;
    default:
      set_status_code(520);
      set_reason_phrase("Unknown Error");
  };
}

int HttpResponse::Modify(const bool keep_alive, const string &version) {
    HttpProtocol::FixRespHeaders(keep_alive, version.c_str(), this);

    return 0;
}

int HttpResponse::result() {
    const char *result{GetHeaderValue(HttpMessage::HEADER_X_PHXRPC_RESULT)};
    return atoi(nullptr == result ? "-1" : result);
}

void HttpResponse::set_result(const int result) {
    AddHeader(HttpMessage::HEADER_X_PHXRPC_RESULT, result);
}

void HttpResponse::set_status_code(int status_code) {
    status_code_ = status_code;
}

int HttpResponse::status_code() const {
    return status_code_;
}

void HttpResponse::set_reason_phrase(const char *reason_phrase) {
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", reason_phrase);
}

const char *HttpResponse::reason_phrase() const {
    return reason_phrase_;
}


}  // namespace phxrpc

