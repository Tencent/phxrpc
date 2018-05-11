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

#include "mqtt_client.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"

#include "mqtt_msg.h"
#include "mqtt_protocol.h"


namespace {


phxrpc::ReturnCode DoMethod(phxrpc::BaseTcpStream &socket, const phxrpc::MqttMessage *const req,
                            phxrpc::MqttMessage *const resp,
                            phxrpc::MqttClient::MqttStat &mqtt_stat) {
    phxrpc::ReturnCode ret{phxrpc::MqttProtocol::SendMessage(socket, req)};
    if (phxrpc::ReturnCode::OK != ret) {
        if (phxrpc::ReturnCode::ERROR_SOCKET_STREAM_NORMAL_CLOSED != ret) {
            mqtt_stat.send_error_ = true;
            phxrpc::log(LOG_ERR, "SendMessage err %d", ret);
        }

        return ret;
    }

    if (!socket.flush().good()) {
        phxrpc::log(LOG_ERR, "socket err %d", socket.LastError());

        return static_cast<phxrpc::ReturnCode>(socket.LastError());
    }

    if (!resp->fake()) {
        ret = phxrpc::MqttProtocol::RecvMessage(socket, resp);
        if (phxrpc::ReturnCode::OK != ret) {
            if (phxrpc::ReturnCode::ERROR_SOCKET_STREAM_NORMAL_CLOSED != ret) {
                mqtt_stat.recv_error_ = true;
                phxrpc::log(LOG_ERR, "RecvMessage err %d", ret);
            }

            return ret;
        }
    }

    return ret;
}

phxrpc::ReturnCode DoMethod(phxrpc::BaseTcpStream &socket, const phxrpc::MqttMessage *const req,
                            phxrpc::MqttMessage *const resp) {
    phxrpc::MqttClient::MqttStat mqtt_stat;
    return DoMethod(socket, req, resp, mqtt_stat);
}


}  // namespace


namespace phxrpc {


int MqttClient::Connect(BaseTcpStream &socket, const MqttConnect &req,
                        MqttConnack &resp, MqttClient::MqttStat &mqtt_stat) {
    return static_cast<int>(DoMethod(socket, &req, &resp, mqtt_stat));
}

int MqttClient::Connect(BaseTcpStream &socket, const MqttConnect &req,
                        MqttConnack &resp) {
    return static_cast<int>(DoMethod(socket, &req, &resp));
}

int MqttClient::Publish(BaseTcpStream &socket, const MqttPublish &req,
                        MqttPuback &resp, MqttClient::MqttStat &mqtt_stat) {
    return static_cast<int>(DoMethod(socket, &req, &resp, mqtt_stat));
}

int MqttClient::Publish(BaseTcpStream &socket, const MqttPublish &req,
                        MqttPuback &resp) {
    return static_cast<int>(DoMethod(socket, &req, &resp));
}

int MqttClient::Subscribe(BaseTcpStream &socket, const MqttSubscribe &req,
                          MqttSuback &resp) {
    return static_cast<int>(DoMethod(socket, &req, &resp));
}

int MqttClient::Unsubscribe(BaseTcpStream &socket, const MqttUnsubscribe &req,
                            MqttUnsuback &resp) {
    return static_cast<int>(DoMethod(socket, &req, &resp));
}

int MqttClient::Ping(BaseTcpStream &socket, const MqttPingreq &req,
                     MqttPingresp &resp) {
    return static_cast<int>(DoMethod(socket, &req, &resp));
}

int MqttClient::Disconnect(BaseTcpStream &socket, const MqttDisconnect &req,
                           MqttClient::MqttStat &mqtt_stat) {
    MqttFakeDisconnack resp;
    return static_cast<int>(DoMethod(socket, &req, &resp, mqtt_stat));
}

int MqttClient::Disconnect(BaseTcpStream &socket, const MqttDisconnect &req) {
    MqttFakeDisconnack resp;
    return static_cast<int>(DoMethod(socket, &req, &resp));
}


}  // namespace phxrpc

