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

#include "http_msg.h"

namespace phxrpc {

const char * HttpMessage::HEADER_CONTENT_LENGTH = "Content-Length";
const char * HttpMessage::HEADER_CONTENT_TYPE = "Content-Type";
const char * HttpMessage::HEADER_CONNECTION = "Connection";
const char * HttpMessage::HEADER_PROXY_CONNECTION = "Proxy-Connection";
const char * HttpMessage::HEADER_TRANSFER_ENCODING = "Transfer-Encoding";
const char * HttpMessage::HEADER_DATE = "Date";
const char * HttpMessage::HEADER_SERVER = "Server";

const char * HttpMessage::HEADER_X_PHXRPC_RESULT = "X-PHXRPC-Result";

HttpMessage::HttpMessage(int type)
        : type_(type) {
    snprintf(version_, sizeof(version_), "%s", "HTTP/1.0");
}

HttpMessage::~HttpMessage() {
}

int HttpMessage::GetType() const {
    return type_;
}

void HttpMessage::SetVersion(const char * version) {
    snprintf(version_, sizeof(version_), "%s", version);
}

const char * HttpMessage::GetVersion() const {
    return version_;
}

void HttpMessage::AppendContent(const void * content, int length, int max_length) {
    if (length <= 0)
        length = strlen((char*) content);

    int total = content_.size() + length;
    total = total > max_length ? total : max_length;

    //content_.reserve(total);

    content_.append((char*) content, length);
}

void HttpMessage::SetContent(const void * content, int length) {
    content_.clear();
    content_.append((char*) content, length);
}

const std::string & HttpMessage::GetContent() const {
    return content_;
}

std::string & HttpMessage::GetContent() {
    return content_;
}

void HttpMessage::AddHeader(const char * name, const char * value) {
    header_name_list_.push_back(name);
    header_value_list_.push_back(value);
}

void HttpMessage::AddHeader(const char * name, int value) {
    char tmp[32] = { 0 };
    snprintf(tmp, sizeof(tmp), "%d", value);

    AddHeader(name, tmp);
}

bool HttpMessage::RemoveHeader(const char * name) {
    bool ret = false;

    for (size_t i = 0; i < header_name_list_.size() && false == ret; i++) {
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

const char * HttpMessage::GetHeaderName(size_t index) const {
    return index < header_name_list_.size() ? header_name_list_[index].c_str() : NULL;
}

const char * HttpMessage::GetHeaderValue(size_t index) const {
    return index < header_value_list_.size() ? header_value_list_[index].c_str() : NULL;
}

const char * HttpMessage::GetHeaderValue(const char * name) const {
    const char * value = NULL;

    for (size_t i = 0; i < header_name_list_.size() && NULL == value; i++) {
        if (0 == strcasecmp(name, header_name_list_[i].c_str())) {
            value = header_value_list_[i].c_str();
        }
    }

    return value;
}

int HttpMessage::IsKeepAlive() const {
    const char * proxy = GetHeaderValue(HEADER_PROXY_CONNECTION);
    const char * local = GetHeaderValue(HEADER_CONNECTION);

    if ((NULL != proxy && 0 == strcasecmp(proxy, "Keep-Alive"))
            || (NULL != local && 0 == strcasecmp(local, "Keep-Alive"))) {
        return 1;
    }

    return 0;
}

//---------------------------------------------------------

HttpRequest::HttpRequest()
        : HttpMessage(eRequest) {
    memset(method_, 0, sizeof(method_));
    memset(client_ip_, 0, sizeof(client_ip_));
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::SetMethod(const char * method) {
    if (method != nullptr) {
        snprintf(method_, sizeof(method_), "%s", method);
    }
}

const char * HttpRequest::GetMethod() const {
    return method_;
}

int HttpRequest::IsMethod(const char * method) const {
    return 0 == strcasecmp(method, method_);
}

void HttpRequest::SetURI(const char * uri) {
    if (uri != nullptr) {
        uri_ = std::string(uri);
    }
}

const char * HttpRequest::GetURI() const {
    return uri_.c_str();
}

void HttpRequest::SetClientIP(const char * client_ip) {
    if (client_ip != nullptr) {
        snprintf(client_ip_, sizeof(client_ip_), "%s", client_ip);
    }
}

const char * HttpRequest::GetClientIP() const {
    return client_ip_;
}

void HttpRequest::AddParam(const char * name, const char * value) {
    param_name_list_.push_back(name);
    param_value_list_.push_back(value);
}

bool HttpRequest::RemoveParam(const char * name) {
    bool ret = false;

    for (size_t i = 0; i < param_name_list_.size() && false == ret; i++) {
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

const char * HttpRequest::GetParamName(size_t index) const {
    return index < param_name_list_.size() ? param_name_list_[index].c_str() : NULL;
}

const char * HttpRequest::GetParamValue(size_t index) const {
    return index < param_value_list_.size() ? param_value_list_[index].c_str() : NULL;
}

const char * HttpRequest::GetParamValue(const char * name) const {
    const char * value = NULL;

    for (size_t i = 0; i < param_name_list_.size() && NULL == value; i++) {
        if (0 == strcasecmp(name, param_name_list_[i].c_str())) {
            value = param_value_list_[i].c_str();
        }
    }

    return value;
}

//---------------------------------------------------------

HttpResponse::HttpResponse()
        : HttpMessage(eResponse) {
    status_code_ = 200;
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", "OK");
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::SetStatusCode(int status_code) {
    status_code_ = status_code;
}

int HttpResponse::GetStatusCode() const {
    return status_code_;
}

void HttpResponse::SetReasonPhrase(const char * reason_phrase) {
    snprintf(reason_phrase_, sizeof(reason_phrase_), "%s", reason_phrase);
}

const char * HttpResponse::GetReasonPhrase() const {
    return reason_phrase_;
}

}

