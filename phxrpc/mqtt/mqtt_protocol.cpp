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

#include "mqtt_protocol.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

#include "mqtt_msg.h"

#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"


namespace phxrpc {


using namespace std;


ReturnCode MqttProtocol::SendMessage(BaseTcpStream &socket,
                                     const MqttMessage *const msg) {
    ostringstream ss;
    ReturnCode ret{msg->SendRemaining(ss)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendRemaining err %d", ret);

        return ret;
    }

    ret = MqttMessage::SendFixedHeaderAndRemainingBuffer(socket,
            msg->control_packet_type(), ss.str());
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendFixedHeaderAndRemainingBuffer err %d", ret);

        return ret;
    }

    if (!socket.flush().good()) {
        phxrpc::log(LOG_ERR, "socket err %d", socket.LastError());

        return static_cast<ReturnCode>(socket.LastError());
    }

    return ret;
}

ReturnCode MqttProtocol::RecvMessage(BaseTcpStream &socket,
                                     MqttMessage *const msg) {
    MqttMessage::ControlPacketType control_packet_type{
            MqttMessage::ControlPacketType::FAKE_NONE};
    string remaining_buffer;
    ReturnCode ret{MqttMessage::RecvFixedHeaderAndRemainingBuffer(socket,
            control_packet_type, remaining_buffer)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", ret);

        return ret;
    }

    istringstream ss(remaining_buffer);

    if (msg->control_packet_type() == control_packet_type) {
        return msg->RecvRemaining(ss);
    }
    phxrpc::log(LOG_ERR, "msg_type %d != recv_type %d",
                static_cast<int>(msg->control_packet_type()),
                static_cast<int>(control_packet_type));

    return ReturnCode::ERROR;
}

ReturnCode MqttProtocol::ServerRecv(BaseTcpStream &socket, BaseRequest *&req) {
    MqttMessage::ControlPacketType control_packet_type{
            MqttMessage::ControlPacketType::FAKE_NONE};
    string remaining_buffer;
    ReturnCode ret{MqttMessage::RecvFixedHeaderAndRemainingBuffer(socket,
            control_packet_type, remaining_buffer)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", ret);

        return ret;
    }

    istringstream ss(remaining_buffer);

    if (MqttMessage::ControlPacketType::CONNECT == control_packet_type) {
        MqttConnect *connect{new MqttConnect};
        req = connect;
        return connect->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::PUBLISH == control_packet_type) {
        MqttPublish *publish{new MqttPublish};
        req = publish;
        return publish->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::SUBSCRIBE == control_packet_type) {
        MqttSubscribe *subscribe{new MqttSubscribe};
        req = subscribe;
        return subscribe->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::UNSUBSCRIBE == control_packet_type) {
        MqttUnsubscribe *unsubscribe{new MqttUnsubscribe};
        req = unsubscribe;
        return unsubscribe->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::PINGREQ == control_packet_type) {
        MqttPingreq *pingreq{new MqttPingreq};
        req = pingreq;
        return pingreq->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::DISCONNECT == control_packet_type) {
        MqttDisconnect *disconnect{new MqttDisconnect};
        req = disconnect;
        return disconnect->RecvRemaining(ss);
    }
    phxrpc::log(LOG_ERR, "type %d not supported", static_cast<int>(control_packet_type));

    return ReturnCode::ERROR;
}


}  // namespace phxrpc

