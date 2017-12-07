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

#include "phxrpc/rpc/phxrpc.pb.h"

#include "http_msg.h"
#include "http_protocol.h"


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

/*
HttpMessage::HttpMessage(int type)
        : BaseMessage(type, BaseMessage::Protocol::HTTP) {
    SetVersion("HTTP/1.0");
}
*/

ReturnCode HttpMessage::ToPb(google::protobuf::Message *const message) const {
    google::protobuf::StringValue string_value;

    string_value.set_value(GetContent());

    try {
        message->CopyFrom(string_value);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode HttpMessage::FromPb(const google::protobuf::Message &message) {
    google::protobuf::StringValue string_value;

    try {
        string_value.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    SetContent(string_value.value().data(), string_value.value().length());

    return ReturnCode::OK;
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

    for (size_t i{0}; i < header_name_list_.size() && nullptr == value; i++) {
        if (0 == strcasecmp(name, header_name_list_[i].c_str())) {
            value = header_value_list_[i].c_str();
        }
    }

    return value;
}



//---------------------------------------------------------

HttpRequest::HttpRequest()
        : HttpMessage(), BaseRequest(Protocol::HTTP_POST) {
    SetVersion("HTTP/1.0");
    memset(method_, 0, sizeof(method_));
    memset(client_ip_, 0, sizeof(client_ip_));
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::SetMethod(const char *method) {
    if (nullptr != method) {
        snprintf(method_, sizeof(method_), "%s", method);
    }
}

const char *HttpRequest::GetMethod() const {
    return method_;
}

int HttpRequest::IsMethod(const char *method) const {
    return 0 == strcasecmp(method, method_);
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

BaseResponse *HttpRequest::GenResponse() const {
    return new HttpResponse;
}

int HttpRequest::IsKeepAlive() const {
    const char *proxy{GetHeaderValue(HEADER_PROXY_CONNECTION)};
    const char *local{GetHeaderValue(HEADER_CONNECTION)};

    if ((nullptr != proxy && 0 == strcasecmp(proxy, "Keep-Alive"))
        || (nullptr != local && 0 == strcasecmp(local, "Keep-Alive"))) {
        return 1;
    }

    return 0;
}



//---------------------------------------------------------

HttpResponse::HttpResponse()
        : HttpMessage(), BaseResponse(Protocol::HTTP_POST) {
    SetVersion("HTTP/1.0");
    status_code_ = 200;
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", "OK");
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::SetPhxRpcResult(const int result) {
    AddHeader(HttpMessage::HEADER_X_PHXRPC_RESULT, result);
}

void HttpResponse::DispatchErr() {
    SetStatusCode(404);
    SetReasonPhrase("Not Found");
}

ReturnCode HttpResponse::Send(BaseTcpStream &socket) const {
    socket << GetVersion() << " " << GetStatusCode() << " " << GetReasonPhrase() << "\r\n";

    for (size_t i{0}; GetHeaderCount() > i; ++i) {
        socket << GetHeaderName(i) << ": " << GetHeaderValue(i) << "\r\n";
    }

    if (GetContent().size() > 0) {
        if (nullptr == GetHeaderValue(HttpMessage::HEADER_CONTENT_LENGTH)) {
            socket << HttpMessage::HEADER_CONTENT_LENGTH << ": " << GetContent().size() << "\r\n";
        }
    }

    socket << "\r\n";

    if (GetContent().size() > 0)
        socket << GetContent();

    if (socket.flush().good()) {
        return ReturnCode::OK;
    } else {
        return static_cast<ReturnCode>(socket.LastError());
    }
}

ReturnCode HttpResponse::ModifyResp(const bool keep_alive, const string &version) {
    HttpProtocol::FixRespHeaders(keep_alive, version.c_str(), this);

    return ReturnCode::OK;
}

void HttpResponse::SetStatusCode(int status_code) {
    status_code_ = status_code;
}

int HttpResponse::GetStatusCode() const {
    return status_code_;
}

void HttpResponse::SetReasonPhrase(const char *reason_phrase) {
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", reason_phrase);
}

const char *HttpResponse::GetReasonPhrase() const {
    return reason_phrase_;
}


}  // namespace phxrpc

