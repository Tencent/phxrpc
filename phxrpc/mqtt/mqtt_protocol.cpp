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


// client send
ReturnCode MqttProtocol::SendMessage(BaseTcpStream &socket,
                                     const MqttMessage *const msg) {
    ostringstream ss;
    ReturnCode ret{msg->SendRemaining(ss)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendRemaining err %d", ret);

        return ret;
    }

    ret = MqttMessage::SendFixedHeaderAndRemainingBuffer(socket,
            msg->fixed_header(), ss.str());
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

// client receive
ReturnCode MqttProtocol::RecvMessage(BaseTcpStream &socket,
                                     MqttMessage *const msg) {
    MqttMessage::FixedHeader fixed_header;
    string remaining_buffer;
    ReturnCode ret{MqttMessage::RecvFixedHeaderAndRemainingBuffer(socket,
            fixed_header, remaining_buffer)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", ret);

        return ret;
    }

    istringstream ss(remaining_buffer);

    if (msg->fixed_header().control_packet_type ==
        fixed_header.control_packet_type) {
        msg->set_fixed_header(fixed_header);
        msg->set_remaining_length(remaining_buffer.size());
        return msg->RecvRemaining(ss);
    }
    phxrpc::log(LOG_ERR, "msg_type %d != recv_type %d",
                static_cast<int>(msg->fixed_header().control_packet_type),
                static_cast<int>(fixed_header.control_packet_type));

    return ReturnCode::ERROR;
}

// server receive
ReturnCode MqttProtocol::ServerRecv(BaseTcpStream &socket, BaseRequest *&req) {
    MqttMessage::FixedHeader fixed_header;
    string remaining_buffer;
    ReturnCode ret{MqttMessage::RecvFixedHeaderAndRemainingBuffer(socket,
            fixed_header, remaining_buffer)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", ret);

        return ret;
    }

    istringstream ss(remaining_buffer);

    if (MqttMessage::ControlPacketType::CONNECT ==
        fixed_header.control_packet_type) {
        MqttConnect *connect{new MqttConnect};
        connect->set_fixed_header(fixed_header);
        connect->set_remaining_length(remaining_buffer.size());
        req = connect;
        return connect->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::PUBLISH ==
               fixed_header.control_packet_type) {
        MqttPublish *publish{new MqttPublish};
        publish->set_fixed_header(fixed_header);
        publish->set_remaining_length(remaining_buffer.size());
        req = publish;
        return publish->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::SUBSCRIBE ==
               fixed_header.control_packet_type) {
        MqttSubscribe *subscribe{new MqttSubscribe};
        subscribe->set_fixed_header(fixed_header);
        subscribe->set_remaining_length(remaining_buffer.size());
        req = subscribe;
        return subscribe->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::UNSUBSCRIBE ==
               fixed_header.control_packet_type) {
        MqttUnsubscribe *unsubscribe{new MqttUnsubscribe};
        unsubscribe->set_fixed_header(fixed_header);
        unsubscribe->set_remaining_length(remaining_buffer.size());
        req = unsubscribe;
        return unsubscribe->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::PINGREQ ==
               fixed_header.control_packet_type) {
        MqttPingreq *pingreq{new MqttPingreq};
        pingreq->set_fixed_header(fixed_header);
        pingreq->set_remaining_length(remaining_buffer.size());
        req = pingreq;
        return pingreq->RecvRemaining(ss);
    } else if (MqttMessage::ControlPacketType::DISCONNECT ==
               fixed_header.control_packet_type) {
        MqttDisconnect *disconnect{new MqttDisconnect};
        disconnect->set_fixed_header(fixed_header);
        disconnect->set_remaining_length(remaining_buffer.size());
        req = disconnect;
        return disconnect->RecvRemaining(ss);
    }
    phxrpc::log(LOG_ERR, "type %d not supported",
                static_cast<int>(fixed_header.control_packet_type));

    return ReturnCode::ERROR;
}


}  // namespace phxrpc

