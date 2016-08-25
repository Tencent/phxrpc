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

namespace phxrpc {

class HttpMessage {
 public:
    static const char * HEADER_CONTENT_LENGTH;
    static const char * HEADER_CONTENT_TYPE;
    static const char * HEADER_CONNECTION;
    static const char * HEADER_PROXY_CONNECTION;
    static const char * HEADER_TRANSFER_ENCODING;
    static const char * HEADER_DATE;
    static const char * HEADER_SERVER;

    static const char * HEADER_X_PHXRPC_RESULT;

 public:
    HttpMessage(int type);
    virtual ~HttpMessage();

    enum {
        eRequest,
        eResponse
    };
    int GetType() const;

    void SetVersion(const char * version);
    const char * GetVersion() const;

    void AppendContent(const void * content, int length = 0, int max_length = 0);
    void SetContent(const void * content, int length = 0);
    const std::string & GetContent() const;
    std::string & GetContent();

    void AddHeader(const char * name, const char * value);
    void AddHeader(const char * name, int value);
    bool RemoveHeader(const char * name);
    size_t GetHeaderCount() const;
    const char * GetHeaderName(size_t index) const;
    const char * GetHeaderValue(size_t index) const;
    const char * GetHeaderValue(const char * name) const;

    int IsKeepAlive() const;

 protected:
    const int type_;

    char version_[16];
    std::string content_;

    std::vector<std::string> header_name_list_, header_value_list_;
};

class HttpRequest : public HttpMessage {
 public:
    HttpRequest();
    virtual ~HttpRequest();

    void SetMethod(const char * method);
    const char * GetMethod() const;

    int IsMethod(const char * method) const;

    void SetURI(const char * uri);
    const char * GetURI() const;

    void SetClientIP(const char * client_ip);
    const char * GetClientIP() const;

    void AddParam(const char * name, const char * value);
    bool RemoveParam(const char * name);
    size_t GetParamCount() const;
    const char * GetParamName(size_t index) const;
    const char * GetParamValue(size_t index) const;
    const char * GetParamValue(const char * name) const;

 private:
    char method_[16], client_ip_[16];
    std::string uri_;

    std::vector<std::string> param_name_list_, param_value_list_;
};

class HttpResponse : public HttpMessage {
 public:
    HttpResponse();
    virtual ~HttpResponse();

    void SetStatusCode(int status_code);
    int GetStatusCode() const;

    void SetReasonPhrase(const char * reason_phrase);
    const char * GetReasonPhrase() const;

 private:
    int status_code_;
    char reason_phrase_[128];
};

}

