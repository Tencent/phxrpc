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

#include <syslog.h>
#include <google/protobuf/message_lite.h>

#include "http_caller.h"
#include "monitor_factory.h"

#include "phxrpc/file.h"
#include "phxrpc/network.h"
#include "phxrpc/http.h"
#include "phxrpc/qos.h"

namespace phxrpc {

HttpCaller::HttpCaller(BaseTcpStream & socket, ClientMonitor & client_monitor )
        : socket_(socket), client_monitor_(client_monitor), cmdid_(-1), is_enable_cli_fr_(false) {
}

HttpCaller::~HttpCaller() {
}

HttpRequest & HttpCaller::GetRequest() {
    return request_;
}

HttpResponse & HttpCaller::GetResponse() {
    return response_;
}

void HttpCaller::SetIsEnableCliFr( const bool is_enable_cli_fr ) {
    phxrpc::log(LOG_DEBUG, "%s is_enable_cli_fr %d", __func__, is_enable_cli_fr);
    is_enable_cli_fr_ = is_enable_cli_fr;
}

void HttpCaller::MonitorReport( ClientMonitor & client_monitor, bool send_error, bool recv_error, size_t send_size, 
                                  size_t recv_size, uint64_t call_begin, uint64_t call_end ) {

    if ( send_error ) {
        client_monitor.SendError();
    }

    if ( recv_error ) {
        client_monitor.RecvError();
    }

    client_monitor.SendBytes( send_size );
    client_monitor.RecvBytes( recv_size );
    client_monitor.RequestCost( call_begin, call_end );
    if ( cmdid_ > 0 ) {
        client_monitor.ClientCall( cmdid_, GetRequest().GetURI() );
    }
}

int HttpCaller::Call(const google::protobuf::MessageLite & request, google::protobuf::MessageLite * response) {

    uint32_t svr_ip = 0;
    uint32_t svr_port = 0;

    phxrpc::log(LOG_DEBUG, "%s is_enable_cli_fr %d req qos info %s",
            __func__, is_enable_cli_fr_,
            FastRejectQoSMgr::GetReqQoSInfo()?FastRejectQoSMgr::GetReqQoSInfo():"");

    bool get_remote_host = socket_.GetRemoteHost(&svr_ip, (int*)&svr_port); 
    if(get_remote_host && is_enable_cli_fr_) {
        //cli fr
        if(FRClient::GetDefault()->IsSvrBlocked(svr_ip, svr_port, FastRejectQoSMgr::GetReqQoSInfo())) {
            phxrpc::log(LOG_DEBUG, "%s req hit cli rfr %s ", __func__, 
            FastRejectQoSMgr::GetReqQoSInfo()?FastRejectQoSMgr::GetReqQoSInfo():"");
            return -206;
        }
    } else {
        phxrpc::log(LOG_DEBUG, "%s get_remote_host failed ", __func__);
    }
    
    if (!request.SerializeToString(&request_.GetContent())) {
        return -1;
    }

    uint64_t call_begin = Timer::GetSteadyClockMS();
    request_.AddHeader(HttpMessage::HEADER_CONTENT_LENGTH, request_.GetContent().size());

    if(FastRejectQoSMgr::GetReqQoSInfo()) {
        request_.AddHeader(HttpMessage::HEADER_X_PHXRPC_QOS_REQ, FastRejectQoSMgr::GetReqQoSInfo());
    }

    HttpClient::PostStat post_stat;
    int ret = HttpClient::Post(socket_, request_, &response_, &post_stat);
    MonitorReport( client_monitor_, post_stat.send_error_, post_stat.recv_error_, request_.GetContent().size(),
                   response_.GetContent().size(), call_begin, Timer::GetSteadyClockMS() );

    if (ret != 0) {
        return ret;
    }

    if (!response->ParseFromString(response_.GetContent())) {
        return -1;
    }

    const char * result = response_.GetHeaderValue(HttpMessage::HEADER_X_PHXRPC_RESULT);
    ret = atoi(NULL == result ? "-1" : result);

    const char * resp_qos_info = response_.GetHeaderValue(HttpMessage::HEADER_X_PHXRPC_QOS_RESP);
    if(resp_qos_info) {

        phxrpc::log(LOG_DEBUG, "%s resp_qos_info %s",
                __func__, resp_qos_info);

        if(get_remote_host) {
            char * pos = (char*)strstr(resp_qos_info, "_");
            if(pos) {
                (*pos) = '\0';
                int svr_business_priority = atoi(resp_qos_info); 
                int svr_user_priority = atoi(pos + 1); 
                (*pos) = '_';


                phxrpc::log(LOG_DEBUG, "%s resp_qos_info %s svr_ip %u svr_port %u "
                        "svr_business_priority %d svr_user_priority %d", __func__,
                        resp_qos_info, svr_ip, svr_port, svr_business_priority,
                        svr_user_priority);
                lb_stat_report(svr_ip, svr_port, svr_business_priority, svr_user_priority);
            }
        }
    } else {
        phxrpc::log(LOG_DEBUG, "%s no resp_qos_info", __func__);
    }

    if( ret < 0 ) {
        phxrpc::log( LOG_ERR, "ERROR: httpcall( %s ) %d", request_.GetURI(), ret );
    }

    return ret;
}

void HttpCaller :: SetURI( const char * uri, int cmdid ) {
    cmdid_ = cmdid;
    GetRequest().SetURI( uri );
}

void HttpCaller :: SetKeepAlive( const bool keep_alive ) {
    if (keep_alive) {
        GetRequest().AddHeader(HttpMessage::HEADER_CONNECTION, "Keep-Alive");
    } else {
        GetRequest().AddHeader(HttpMessage::HEADER_CONNECTION, "");
    }
}

}
