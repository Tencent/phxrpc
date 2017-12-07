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

#include "mqtt_caller.h"
#include "monitor_factory.h"

#include "phxrpc/network.h"
#include "phxrpc/http.h"
#include "phxrpc/file.h"


namespace phxrpc {


MqttCaller::MqttCaller(BaseTcpStream &socket, ClientMonitor &client_monitor)
        : socket_(socket), client_monitor_(client_monitor), cmd_id_(-1) {
}

MqttCaller::~MqttCaller() {
}

MqttConnect &MqttCaller::GetConnect() {
    return connect_;
}

MqttPublish &MqttCaller::GetPublish() {
    return publish_;
}

MqttDisconnect &MqttCaller::GetDisconnect() {
    return disconnect_;
}

MqttConnack &MqttCaller::GetConnack() {
    return connack_;
}

MqttPuback &MqttCaller::GetPuback() {
    return puback_;
}

void MqttCaller::MonitorReport(ClientMonitor &client_monitor, bool send_error,
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
        client_monitor.ClientCall(cmd_id_, "");
    }
}

int MqttCaller::PhxMqttConnectCall(const phxrpc::MqttConnectPb &connect,
                                   phxrpc::MqttConnackPb *connack) {
    int ret{-1};

    // unpack request
    {
        ret = static_cast<int>(connect_.FromPb(connect));
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "FromPb err %d", ret);

            return ret;
        }
    }

    uint64_t call_begin{Timer::GetSteadyClockMS()};
    MqttClient::MqttStat mqtt_stat;
    ret = MqttClient::Connect(socket_, connect_, connack_, mqtt_stat);
    MonitorReport(client_monitor_, mqtt_stat.send_error_,
                  mqtt_stat.recv_error_, connect_.GetContent().size(),
                  connack_.GetContent().size(), call_begin,
                  Timer::GetSteadyClockMS());

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "mqtt connect call err %d", ret);
        return ret;
    }

    // pack response
    {
        ret = static_cast<int>(connack_.ToPb(connack));
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "ToPb ret %d", ret);

            return ret;
        }
    }

    return ret;
}

int MqttCaller::PhxMqttPublishCall(const phxrpc::MqttPublishPb &publish,
                                   phxrpc::MqttPubackPb *puback) {
    int ret{-1};

    // unpack request
    {
        ret = static_cast<int>(publish_.FromPb(publish));
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "FromPb err %d", ret);

            return ret;
        }
    }

    uint64_t call_begin{Timer::GetSteadyClockMS()};
    MqttClient::MqttStat mqtt_stat;
    publish_.set_packet_identifier(packet_identifier_++);
    ret = MqttClient::Publish(socket_, publish_, puback_, mqtt_stat);
    MonitorReport(client_monitor_, mqtt_stat.send_error_,
                  mqtt_stat.recv_error_, publish_.GetContent().size(),
                  puback_.GetContent().size(), call_begin,
                  Timer::GetSteadyClockMS());

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "mqtt publish call err %d", ret);
        return ret;
    }

    // pack response
    {
        ret = static_cast<int>(puback_.ToPb(puback));
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "ToPb ret %d", ret);

            return ret;
        }
    }

    return ret;
}

int MqttCaller::PhxMqttDisconnectCall(const phxrpc::MqttDisconnectPb &disconnect) {
    int ret{-1};

    // unpack request
    {
        ret = static_cast<int>(disconnect_.FromPb(disconnect));
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "FromPb err %d", ret);

            return ret;
        }
    }

    uint64_t call_begin{Timer::GetSteadyClockMS()};
    MqttClient::MqttStat mqtt_stat;
    ret = MqttClient::Disconnect(socket_, disconnect_, mqtt_stat);
    MonitorReport(client_monitor_, mqtt_stat.send_error_,
                  mqtt_stat.recv_error_, disconnect_.GetContent().size(),
                  0, call_begin, Timer::GetSteadyClockMS());

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "mqtt disconnect call err %d", ret);
        return ret;
    }

    return ret;
}

void MqttCaller::SetCmdId(const int cmd_id) {
    cmd_id_ = cmd_id;
}


}  // namespace phxrpc

