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

#include "mqtt_msg.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"
#include "phxrpc/rpc/phxrpc.pb.h"

#include "mqtt_protocol.h"


namespace phxrpc {


using namespace std;


const char MqttMessage::FixedHeader[]{
    '\x00',  // FAKE_NONE
    '\x10',  // CONNECT
    '\x20',  // CONNACK
    '\x32',  // PUBLISH QoS 1
    '\x40',  // PUBACK
    '\x50',  // PUBREC
    '\x62',  // PUBREL
    '\x70',  // PUBCOMP
    '\x82',  // SUBSCRIBE
    '\x90',  // SUBACK
    '\xa2',  // UNSUBSCRIBE
    '\xb0',  // UNSUBACK
    '\xc0',  // PINGREQ
    '\xd0',  // PINGRESP
    '\xe0',  // DISCONNECT
    '\xf0',  // FAKE_DISCONNACK
};

//const bool MqttMessage::NeedPacketIdentifier[]{
//    false,  // FAKE_NONE
//    false,  // CONNECT
//    false,  // CONNACK
//    true,  // PUBLISH QoS 1
//    true,  // PUBACK
//    true,  // PUBREC
//    true,  // PUBREL
//    true,  // PUBCOMP
//    true,  // SUBSCRIBE
//    true,  // SUBACK
//    true,  // UNSUBSCRIBE
//    true,  // UNSUBACK
//    false,  // PINGREQ
//    false,  // PINGRESP
//    false,  // DISCONNECT
//    false,  // FAKE_DISCONNACK
//};

int MqttMessage::EncodeUint16(string &dest, const uint16_t src) {
    dest.clear();
    dest.resize(2);
    dest[0] = static_cast<uint8_t>(src >> 8);
    dest[1] = static_cast<uint8_t>(src);

    return 0;
}

int MqttMessage::EncodeUint16(char *const dest, const size_t dest_size,
                              const uint16_t src) {
    if (2 != dest_size)
        return -1;

    dest[0] = static_cast<uint8_t>(src >> 8);
    dest[1] = static_cast<uint8_t>(src);

    return 0;
}

int MqttMessage::EncodeUnicode(string &dest, const string &src) {
    dest.clear();
    dest.resize(2 + src.size());
    uint16_t src_size{static_cast<uint16_t>(src.size())};
    dest[0] = static_cast<uint8_t>(src_size >> 8);
    dest[1] = static_cast<uint8_t>(src_size);
    for (int i{0}; src_size > i; ++i) {
        dest[i + 2] = src.at(i);
    }

    return 0;
}

int MqttMessage::EncodeUnicode(char *const dest, const size_t dest_size,
                               const string &src) {
    if (2 + src.size() != dest_size)
        return -1;

    uint16_t src_size{static_cast<uint16_t>(src.size())};
    dest[0] = static_cast<uint8_t>(src_size >> 8);
    dest[1] = static_cast<uint8_t>(src_size);
    for (int i{0}; src_size > i; ++i) {
        dest[i + 2] = src.at(i);
    }

    return 0;
}


ReturnCode MqttMessage::SendChar(ostringstream &out_stream,
                                 const char &content) {
    out_stream.put(content);

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvChar(istringstream &in_stream, char &content) {
    in_stream.get(content);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendUint16(ostringstream &out_stream,
                                   const uint16_t content) {
    out_stream.put(static_cast<char>(content >> 8));
    out_stream.put(static_cast<char>(content));

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvUint16(istringstream &in_stream,
                                   uint16_t &content) {
    char temp;
    in_stream.get(temp);
    content = (static_cast<uint8_t>(temp) << 8);
    temp = '\0';
    in_stream.get(temp);
    content |= static_cast<uint8_t>(temp);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendChars(ostringstream &out_stream,
                                  const char *const content,
                                  const int content_length) {
    out_stream.write(content, content_length);

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvChars(istringstream &in_stream,
                                  char *const content,
                                  const int content_length) {
    in_stream.read(content, content_length);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendUnicode(ostringstream &out_stream,
                                    const string &content) {
    uint16_t content_size{static_cast<uint16_t>(content.size())};
    ReturnCode ret{SendUint16(out_stream, content_size)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUint16 err %d", ret);

        return ret;
    }

    out_stream.write(content.data(), content.size());

    // TODO: check stream
    return ret;
}

ReturnCode MqttMessage::RecvUnicode(istringstream &in_stream, string &content) {
    uint16_t content_size{0};
    ReturnCode ret{RecvUint16(in_stream, content_size)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUint16 err %d", ret);

        return ret;
    }

    content.resize(content_size);
    in_stream.read(&content[0], content_size);

    return ret;
}


ReturnCode MqttMessage::SendChar(BaseTcpStream &out_stream,
                                 const char &content) {
    out_stream.put(content);

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvChar(BaseTcpStream &in_stream, char &content) {
    in_stream.get(content);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendUint16(BaseTcpStream &out_stream,
                                   const uint16_t content) {
    out_stream.put(static_cast<char>(content >> 8));
    out_stream.put(static_cast<char>(content));

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvUint16(BaseTcpStream &in_stream, uint16_t &content) {
    char temp;
    in_stream.get(temp);
    content = (static_cast<uint8_t>(temp) << 8);
    temp = '\0';
    in_stream.get(temp);
    content |= static_cast<uint8_t>(temp);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendChars(BaseTcpStream &out_stream,
                                  const char *const content,
                                  const int content_length) {
    out_stream.write(content, content_length);

    // TODO: check stream
    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvChars(BaseTcpStream &in_stream,
                                  char *const content,
                                  const int content_length) {
    in_stream.read(content, content_length);

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendUnicode(BaseTcpStream &out_stream,
                                    const string &content) {
    uint16_t content_size{static_cast<uint16_t>(content.size())};
    ReturnCode ret{SendUint16(out_stream, content_size)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUint16 err %d", ret);

        return ret;
    }

    out_stream.write(content.data(), content.size());

    // TODO: check stream
    return ret;
}

ReturnCode MqttMessage::RecvUnicode(BaseTcpStream &in_stream, string &content) {
    uint16_t content_size{0};
    ReturnCode ret{RecvUint16(in_stream, content_size)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUint16 err %d", ret);

        return ret;
    }

    content.resize(content_size);
    in_stream.read(&content[0], content_size);

    return ret;
}


MqttMessage::MqttMessage() {
    SetVersion("MQTT/3.1.1");
    memset(client_ip_, 0, sizeof(client_ip_));
}

MqttMessage::~MqttMessage() {}

ReturnCode MqttMessage::SendFixedHeaderAndRemainingBuffer(
        BaseTcpStream &out_stream,
        const ControlPacketType control_packet_type,
        const string &remaining_buffer) {
    char fixed_header{FixedHeader[static_cast<int>(control_packet_type)]};
    ReturnCode ret{SendChar(out_stream, fixed_header)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    } else {
        phxrpc::log(LOG_DEBUG, "SendChar type %d fixed_header %u",
                    static_cast<int>(control_packet_type),
                    static_cast<uint8_t>(fixed_header));
    }

    const int remaining_length{static_cast<const int>(remaining_buffer.size())};
    ret = SendRemainingLength(out_stream, remaining_length);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendRemainingLength err %d", ret);

        return ret;
    }

    ret = SendChars(out_stream, remaining_buffer.data(),
                    remaining_buffer.size());
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChars err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvFixedHeaderAndRemainingBuffer(
        BaseTcpStream &in_stream, ControlPacketType &control_packet_type,
        string &remaining_buffer) {
    char fixed_header{0x0};
    ReturnCode ret{RecvChar(in_stream, fixed_header)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }

    uint8_t temp{static_cast<uint8_t>(fixed_header)};
    temp >>= 4;
    temp &= 0x0f;
    // must convert to unsigned first
    control_packet_type = static_cast<ControlPacketType>(temp);

    phxrpc::log(LOG_DEBUG, "RecvChar type %d fixed_header %u",
                static_cast<int>(control_packet_type),
                static_cast<uint8_t>(fixed_header));

    int remaining_length{0};
    ret = RecvRemainingLength(in_stream, remaining_length);

    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvRemainingLength err %d", ret);

        return ret;
    }

    remaining_buffer.resize(remaining_length);
    ret = RecvChars(in_stream, &remaining_buffer[0], remaining_length);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChars err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttMessage::SendRemainingLength(BaseTcpStream &out_stream,
                                            const int remaining_length) {
    char temp{0x0};
    char continue_bit{0x0};
    uint32_t temp_remaining_length{static_cast<uint32_t>(remaining_length)};

    for (int i{0}; 4 > i; ++i) {
        temp = (temp_remaining_length & 0x7f);
        temp_remaining_length >>= 8;
        continue_bit = (temp_remaining_length > 0) ? 0x80 : 0x0;
        out_stream.put(temp | continue_bit);
        if (0x0 == continue_bit) {
            return ReturnCode::OK;
        }
    }

    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvRemainingLength(BaseTcpStream &in_stream,
                                            int &remaining_length) {
    uint32_t temp_remaining_length{0};

    char temp{0x0};
    in_stream.get(temp);
    temp_remaining_length = (static_cast<uint8_t>(temp) & 0x7f);

    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return ReturnCode::OK;
    }

    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 7;
    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return ReturnCode::OK;
    }

    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 14;
    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return ReturnCode::OK;
    }

    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 21;

    remaining_length = temp_remaining_length;

    return ReturnCode::OK;
}

ReturnCode MqttMessage::Send(BaseTcpStream &socket) const {
    ostringstream ss;
    ReturnCode ret{SendRemaining(ss)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendRemaining err %d", ret);

        return ret;
    }

    ret = SendFixedHeaderAndRemainingBuffer(socket, control_packet_type(), ss.str());
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

ReturnCode MqttMessage::SendRemaining(ostringstream &out_stream) const {
    ReturnCode ret{SendVariableHeader(out_stream)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendVariableHeader err %d", ret);

        return ret;
    }

    ret = SendPayload(out_stream);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendPayload err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttMessage::RecvRemaining(istringstream &in_stream) {
    ReturnCode ret{RecvVariableHeader(in_stream)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvVariableHeader err %d", ret);

        return ret;
    }

    ret = RecvPayload(in_stream);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvPayload err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttMessage::SendPacketIdentifier(ostringstream &out_stream) const {
    //const int packet_type{static_cast<int>(control_packet_type_)};
    //if (!NeedPacketIdentifier[packet_type]) {
    //    phxrpc::log(LOG_WARNING, "type %d ignored packet identifier",
    //                packet_type);

    //    return ReturnCode::OK;
    //}

    ReturnCode ret{SendUint16(out_stream, packet_identifier_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUint16 err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvPacketIdentifier(istringstream &in_stream) {
    //const int packet_type{static_cast<int>(control_packet_type_)};
    //if (!NeedPacketIdentifier[packet_type]) {
    //    phxrpc::log(LOG_WARNING, "type %d ignored packet identifier",
    //                packet_type);

    //    return ReturnCode::OK;
    //}

    ReturnCode ret{RecvUint16(in_stream, packet_identifier_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUint16 err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttResponse::ModifyResp(const bool keep_alive, const string &version) {
    return ReturnCode::OK;
}


MqttConnect::MqttConnect() : MqttRequest(Protocol::MQTT_CONNECT) {
    set_control_packet_type(ControlPacketType::CONNECT);

    proto_name_.resize(6);
    proto_name_[0] = 0;
    proto_name_[1] = 4;
    proto_name_[2] = 'M';
    proto_name_[3] = 'Q';
    proto_name_[4] = 'T';
    proto_name_[5] = 'T';
}

ReturnCode MqttConnect::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttConnectPb connect;

    connect.set_clean_session(clean_session_);
    connect.set_keep_alive(keep_alive_);
    connect.set_client_identifier(client_identifier_);
    connect.set_proto_name(proto_name_);
    connect.set_proto_level(proto_level_);

    try {
        message->CopyFrom(connect);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttConnect::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttConnectPb connect;

    try {
        connect.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    clean_session_ = connect.clean_session();
    keep_alive_ = connect.keep_alive();
    client_identifier_ = connect.client_identifier();
    proto_name_ = connect.proto_name();
    proto_level_ = connect.proto_level();

    return ReturnCode::OK;
}

BaseResponse *MqttConnect::GenResponse() const { return new MqttConnack; }

ReturnCode MqttConnect::SendVariableHeader(ostringstream &out_stream) const {
    ReturnCode ret{SendChars(out_stream, proto_name_.data(),
                             proto_name_.size())};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChars err %d", ret);

        return ret;
    }

    ret = SendChar(out_stream, proto_level_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    }

    uint8_t connect_flags{0};
    connect_flags |= ((clean_session_ ? 1u : 0u) << 1);
    ret = SendChar(out_stream, static_cast<char>(connect_flags));
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    }

    ret = SendUint16(out_stream, keep_alive_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUint16 err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttConnect::RecvVariableHeader(istringstream &in_stream) {
    char proto_name[6]{0x0};
    ReturnCode ret{RecvChars(in_stream, proto_name, 6)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChars err %d", ret);

        return ret;
    }
    proto_name_.resize(6);
    proto_name_[0] = proto_name[0];
    proto_name_[1] = proto_name[1];
    proto_name_[2] = proto_name[2];
    proto_name_[3] = proto_name[3];
    proto_name_[4] = proto_name[4];
    proto_name_[5] = proto_name[5];

    ret = RecvChar(in_stream, proto_level_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }

    char connect_flags{0x0};
    ret = RecvChar(in_stream, connect_flags);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }
    clean_session_ = ((connect_flags & 0x2) >> 1);

    ret = RecvUint16(in_stream, keep_alive_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUint16 err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttConnect::SendPayload(ostringstream &out_stream) const {
    return SendUnicode(out_stream, client_identifier_);
}

ReturnCode MqttConnect::RecvPayload(istringstream &in_stream) {
    return RecvUnicode(in_stream, client_identifier_);
}


MqttConnack::MqttConnack() : MqttResponse(Protocol::MQTT_CONNECT) {
    set_control_packet_type(ControlPacketType::CONNACK);
}

ReturnCode MqttConnack::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttConnackPb connack;

    connack.set_session_present(session_present_);
    connack.set_connect_return_code(connect_return_code_);

    try {
        message->CopyFrom(connack);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttConnack::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttConnackPb connack;

    try {
        connack.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    session_present_ = connack.session_present();
    connect_return_code_ = connack.connect_return_code();

    return ReturnCode::OK;
}

ReturnCode MqttConnack::SendVariableHeader(ostringstream &out_stream) const {
    ReturnCode ret{SendChar(out_stream, session_present_ ? 0x1 : 0x0)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    }

    ret = SendChar(out_stream, connect_return_code_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttConnack::RecvVariableHeader(istringstream &in_stream) {
    char connect_acknowledge_flags{0x0};
    ReturnCode ret{RecvChar(in_stream, connect_acknowledge_flags)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }
    session_present_ = (0x1 == (connect_acknowledge_flags & 0x1));

    ret = RecvChar(in_stream, connect_return_code_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}


MqttPublish::MqttPublish() : MqttRequest(Protocol::MQTT_PUBLISH) {
    set_control_packet_type(ControlPacketType::PUBLISH);
}

ReturnCode MqttPublish::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPublishPb publish;

    publish.set_topic_name(topic_name_);
    publish.set_content(GetContent());
    publish.set_package_identifier(packet_identifier());

    try {
        message->CopyFrom(publish);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPublish::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPublishPb publish;

    try {
        publish.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    topic_name_ = publish.topic_name();
    SetContent(publish.content().data(), publish.content().length());
    set_packet_identifier(publish.package_identifier());

    return ReturnCode::OK;
}

BaseResponse *MqttPublish::GenResponse() const { return new MqttPuback; }

ReturnCode MqttPublish::SendVariableHeader(ostringstream &out_stream) const {
    ReturnCode ret{SendUnicode(out_stream, topic_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

        return ret;
    }

    ret = SendPacketIdentifier(out_stream);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendPacketIdentifier err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttPublish::RecvVariableHeader(istringstream &in_stream) {
    ReturnCode ret{RecvUnicode(in_stream, topic_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

        return ret;
    }

    ret = RecvPacketIdentifier(in_stream);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvPacketIdentifier err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttPublish::SendPayload(ostringstream &out_stream) const {
    return SendUnicode(out_stream, GetContent());
}

ReturnCode MqttPublish::RecvPayload(istringstream &in_stream) {
    return RecvUnicode(in_stream, GetContent());
}


MqttPuback::MqttPuback() : MqttResponse(Protocol::MQTT_PUBLISH) {
    set_control_packet_type(ControlPacketType::PUBACK);
}

ReturnCode MqttPuback::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPubackPb puback;

    puback.set_package_identifier(packet_identifier());

    try {
        message->CopyFrom(puback);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPuback::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPubackPb puback;

    try {
        puback.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    set_packet_identifier(puback.package_identifier());

    return ReturnCode::OK;
}

ReturnCode MqttPuback::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttPuback::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}


MqttSubscribe::MqttSubscribe() : MqttRequest(Protocol::MQTT_SUBSCRIBE) {
    set_control_packet_type(ControlPacketType::SUBSCRIBE);
}

BaseResponse *MqttSubscribe::GenResponse() const { return new MqttSuback; }

ReturnCode MqttSubscribe::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttSubscribe::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}

ReturnCode MqttSubscribe::SendPayload(ostringstream &out_stream) const {
    for (int i{0}; topic_filters_.size() > i; ++i) {
        ReturnCode ret{SendUnicode(out_stream, topic_filters_.at(i))};
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }

        ret = SendChar(out_stream, 0x0);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendChar err %d", ret);

            return ret;
        }
    }

    return ReturnCode::OK;
}

ReturnCode MqttSubscribe::RecvPayload(istringstream &in_stream) {
    while (EOF != in_stream.peek()) {
        string topic_filter;
        ReturnCode ret{RecvUnicode(in_stream, topic_filter)};
        if (ReturnCode::ERROR_LENGTH_OVERFLOW != ret) {
            return ReturnCode::OK;
        }

        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }

        char requested_qos{0x0};
        ret = RecvChar(in_stream, requested_qos);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

            return ret;
        }

        topic_filters_.emplace_back(topic_filter);
    }

    return ReturnCode::OK;
}


MqttSuback::MqttSuback() : MqttResponse(Protocol::MQTT_SUBSCRIBE) {
    set_control_packet_type(ControlPacketType::SUBACK);
}

ReturnCode MqttSuback::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttSuback::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}

ReturnCode MqttSuback::SendPayload(ostringstream &out_stream) const {
    for (int i{0}; return_codes_.size() > i; ++i) {
        ReturnCode ret{SendChar(out_stream, return_codes_.at(i))};
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendChar err %d", ret);

            return ret;
        }
    }

    return ReturnCode::OK;
}

ReturnCode MqttSuback::RecvPayload(istringstream &in_stream) {
    while (EOF != in_stream.peek()) {
        char return_code{0x0};
        ReturnCode ret{RecvChar(in_stream, return_code)};
        if (ReturnCode::ERROR_LENGTH_OVERFLOW != ret) {
            return ReturnCode::OK;
        }

        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

            return ret;
        }

        return_codes_.resize(return_codes_.size() + 1);
        return_codes_[return_codes_.size()] = return_code;
    }

    return ReturnCode::OK;
}


MqttUnsubscribe::MqttUnsubscribe() : MqttRequest(Protocol::MQTT_UNSUBSCRIBE) {
    set_control_packet_type(ControlPacketType::UNSUBSCRIBE);
}

BaseResponse *MqttUnsubscribe::GenResponse() const { return new MqttUnsuback; }

ReturnCode
MqttUnsubscribe::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode
MqttUnsubscribe::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}

ReturnCode
MqttUnsubscribe::SendPayload(ostringstream &out_stream) const {
    for (int i{0}; topic_filters_.size() > i; ++i) {
        ReturnCode ret{SendUnicode(out_stream, topic_filters_.at(i))};
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }
    }

    return ReturnCode::OK;
}

ReturnCode
MqttUnsubscribe::RecvPayload(istringstream &in_stream) {
    while (EOF != in_stream.peek()){
        string topic_filter;
        ReturnCode ret{RecvUnicode(in_stream, topic_filter)};
        if (ReturnCode::ERROR_LENGTH_OVERFLOW != ret) {
            return ReturnCode::OK;
        }

        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }

        topic_filters_.emplace_back(topic_filter);
    }

    return ReturnCode::OK;
}


MqttUnsuback::MqttUnsuback() : MqttResponse(Protocol::MQTT_UNSUBSCRIBE) {
    set_control_packet_type(ControlPacketType::UNSUBACK);
}

ReturnCode MqttUnsuback::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttUnsuback::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}


MqttPingreq::MqttPingreq() : MqttRequest(Protocol::MQTT_PING) {
    set_control_packet_type(ControlPacketType::PINGREQ);
}

BaseResponse *MqttPingreq::GenResponse() const { return new MqttPingresp; }


MqttPingresp::MqttPingresp() : MqttResponse(Protocol::MQTT_PING) {
    set_control_packet_type(ControlPacketType::PINGRESP);
}


MqttDisconnect::MqttDisconnect() : MqttRequest(Protocol::MQTT_DISCONNECT) {
    set_control_packet_type(ControlPacketType::DISCONNECT);
}

ReturnCode MqttDisconnect::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttDisconnectPb disconnect;

    try {
        message->CopyFrom(disconnect);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttDisconnect::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttDisconnectPb disconnect;

    try {
        disconnect.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

BaseResponse *MqttDisconnect::GenResponse() const { return new MqttFakeDisconnack; }


MqttFakeDisconnack::MqttFakeDisconnack()
        : MqttResponse(Protocol::MQTT_FAKE_DISCONNACK) {
    set_control_packet_type(ControlPacketType::FAKE_DISCONNACK);
    set_fake(true);
}


}  // namespace phxrpc

