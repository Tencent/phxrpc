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

#include "http_caller.h"

#include <syslog.h>

#include <google/protobuf/message_lite.h>

#include "monitor_factory.h"

#include "phxrpc/network.h"
#include "phxrpc/file.h"


namespace phxrpc {


HttpCaller::HttpCaller(BaseTcpStream &socket, ClientMonitor &client_monitor)
        : socket_(socket), client_monitor_(client_monitor), cmd_id_(-1) {
}

HttpCaller::~HttpCaller() {
}

HttpRequest &HttpCaller::GetRequest() {
    return req_;
}

HttpResponse &HttpCaller::GetResponse() {
    return resp_;
}

void HttpCaller::MonitorReport(ClientMonitor &client_monitor, bool send_error,
                               bool recv_error, size_t send_size,
                               size_t recv_size, uint64_t call_begin,
                               uint64_t call_end) {
    if (send_error) {
        client_monitor.SendError();
    }

    if (recv_error) {
        client_monitor.RecvError();
    }

    client_monitor.SendBytes(send_size);
    client_monitor.RecvBytes(recv_size);
    client_monitor.RequestCost(call_begin, call_end);
    if (0 < cmd_id_) {
        client_monitor.ClientCall(cmd_id_, GetRequest().GetURI());
    }
}

int HttpCaller::Call(const google::protobuf::MessageLite &req,
                     google::protobuf::MessageLite *resp) {
    if (!req.SerializeToString(&req_.GetContent())) {
        return -1;
    }

    uint64_t call_begin{Timer::GetSteadyClockMS()};
    req_.AddHeader(HttpMessage::HEADER_CONTENT_LENGTH, req_.GetContent().size());
    HttpClient::PostStat post_stat;
    int ret{HttpClient::Post(socket_, req_, &resp_, &post_stat)};
    MonitorReport(client_monitor_, post_stat.send_error_,
                  post_stat.recv_error_, req_.GetContent().size(),
                  resp_.GetContent().size(), call_begin,
                  Timer::GetSteadyClockMS());

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "http call err %d", ret);
        return ret;
    }

    if (!resp->ParseFromString(resp_.GetContent())) {
        return -1;
    }

    const char *result{resp_.GetHeaderValue(HttpMessage::HEADER_X_PHXRPC_RESULT)};
    ret = atoi(nullptr == result ? "-1" : result);

    if (ret < 0) {
        phxrpc::log(LOG_ERR, "http call %s err %d", req_.GetURI(), ret);
    }

    return ret;
}

void HttpCaller::SetURI(const char *const uri, const int cmdid) {
    cmd_id_ = cmdid;
    GetRequest().SetURI(uri);
}

void HttpCaller::SetKeepAlive(const bool keep_alive) {
    if (keep_alive) {
        GetRequest().AddHeader(HttpMessage::HEADER_CONNECTION, "Keep-Alive");
    } else {
        GetRequest().AddHeader(HttpMessage::HEADER_CONNECTION, "");
    }
}


}  // namespace phxrpc

