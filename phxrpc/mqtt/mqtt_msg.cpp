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


const char MqttMessage::SampleFixedHeader[]{
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

uint8_t MqttMessage::EncodeFixedHeader(const FixedHeader &fixed_header) {
    const int control_packet_type_int{static_cast<int>(fixed_header.control_packet_type)};
    const char fixed_header_char{SampleFixedHeader[control_packet_type_int]};
    uint8_t fixed_header_byte{static_cast<uint8_t>(fixed_header_char)};
    if (ControlPacketType::PUBLISH == fixed_header.control_packet_type) {
        fixed_header.dup ? (fixed_header_byte |= 0x8) : (fixed_header_byte &= ~0x8);
        fixed_header_byte &= ~0x6;
        fixed_header_byte |= (static_cast<uint8_t>(fixed_header.qos) << 1);
        fixed_header.retain ? (fixed_header_byte |= 0x1) : (fixed_header_byte &= ~0x1);
    }

    return fixed_header_byte;
}

MqttMessage::FixedHeader MqttMessage::DecodeFixedHeader(const uint8_t fixed_header_byte) {
    FixedHeader fixed_header;

    fixed_header.dup = static_cast<bool>((fixed_header_byte >> 3) & 0x01);
    fixed_header.qos = static_cast<int>((fixed_header_byte >> 1) & 0x03);
    fixed_header.retain = static_cast<bool>(fixed_header_byte & 0x01);

    uint8_t temp{fixed_header_byte};
    temp >>= 4;
    temp &= 0x0f;
    // must convert to unsigned first
    fixed_header.control_packet_type = static_cast<ControlPacketType>(temp);

    return fixed_header;
}

ReturnCode MqttMessage::SendFixedHeaderAndRemainingBuffer(
        BaseTcpStream &out_stream, const FixedHeader &fixed_header,
        const string &remaining_buffer) {
    uint8_t fixed_header_byte{EncodeFixedHeader(fixed_header)};
    ReturnCode ret{SendChar(out_stream, static_cast<char>(fixed_header_byte))};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    } else {
        phxrpc::log(LOG_DEBUG, "SendChar type %d fixed_header_byte %u",
                    static_cast<int>(fixed_header.control_packet_type),
                    static_cast<uint8_t>(fixed_header_byte));
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
        BaseTcpStream &in_stream, FixedHeader &fixed_header,
        string &remaining_buffer) {
    char fixed_header_char{0x0};
    ReturnCode ret{RecvChar(in_stream, fixed_header_char)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }

    fixed_header = DecodeFixedHeader(static_cast<uint8_t>(fixed_header_char));

    phxrpc::log(LOG_DEBUG, "RecvChar type %d fixed_header %x",
                static_cast<int>(fixed_header.control_packet_type),
                static_cast<uint8_t>(fixed_header_char));

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

    ret = SendFixedHeaderAndRemainingBuffer(socket, fixed_header(), ss.str());
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
        // TODO: remove
        //string remaining_buffer(out_stream.str());
        //printf("remaining_buffer %zu\n", remaining_buffer.size());
        //for (int i{0}; remaining_buffer.size() > i; ++i) {
        //printf("%d\t", remaining_buffer.at(i));
        //}
        //printf("\n");
        //for (int i{0}; remaining_buffer.size() > i; ++i) {
        //printf("%c\t", remaining_buffer.at(i));
        //}
        //printf("\n");

    ret = SendPayload(out_stream);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendPayload err %d", ret);

        return ret;
    }
        // TODO: remove
        //remaining_buffer = out_stream.str();
        //printf("remaining_buffer %zu\n", remaining_buffer.size());
        //for (int i{0}; remaining_buffer.size() > i; ++i) {
        //printf("%d\t", remaining_buffer.at(i));
        //}
        //printf("\n");
        //for (int i{0}; remaining_buffer.size() > i; ++i) {
        //printf("%c\t", remaining_buffer.at(i));
        //}
        //printf("\n");

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
    //phxrpc::log(LOG_DEBUG, "RecvPayload \"%s\"", GetContent().c_str());

    return ret;
}

ReturnCode MqttMessage::SendPacketIdentifier(ostringstream &out_stream) const {
    ReturnCode ret{SendUint16(out_stream, packet_identifier_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUint16 err %d", ret);

        return ret;
    }

    return ReturnCode::OK;
}

ReturnCode MqttMessage::RecvPacketIdentifier(istringstream &in_stream) {
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


MqttFakeResponse::MqttFakeResponse() {
    set_protocol(Protocol::MQTT_FAKE_NONE);
    mutable_fixed_header().control_packet_type = ControlPacketType::FAKE_NONE;
    set_fake(true);
}


MqttConnect::MqttConnect() {
    set_protocol(Protocol::MQTT_CONNECT);
    SetURI("/phxrpc/mqtt/PhxMqttConnect");
    mutable_fixed_header().control_packet_type = ControlPacketType::CONNECT;
}

ReturnCode MqttConnect::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttConnectPb connect;

    connect.set_client_identifier(client_identifier_);
    connect.set_proto_name(proto_name_);
    connect.set_proto_level(proto_level_);
    connect.set_clean_session(clean_session_);
    connect.set_keep_alive(keep_alive_);
    connect.set_user_name(user_name_);
    connect.set_password(password_);
    connect.set_will_flag(will_flag_);
    connect.set_will_qos(will_qos_);
    connect.set_will_retain(will_retain_);
    connect.set_will_topic(will_topic_);
    connect.set_will_message(will_message_);

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

    client_identifier_ = connect.client_identifier();
    proto_name_ = connect.proto_name();
    proto_level_ = connect.proto_level();
    clean_session_ = connect.clean_session();
    keep_alive_ = connect.keep_alive();
    user_name_ = connect.user_name();
    password_ = connect.password();
    will_flag_ = connect.will_flag();
    will_qos_ = connect.will_qos();
    will_retain_ = connect.will_retain();
    will_topic_ = connect.will_topic();
    will_message_ = connect.will_message();

    return ReturnCode::OK;
}

BaseResponse *MqttConnect::GenResponse() const { return new MqttConnack; }

ReturnCode MqttConnect::SendVariableHeader(ostringstream &out_stream) const {
    ReturnCode ret{SendUnicode(out_stream, proto_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

        return ret;
    }

    ret = SendChar(out_stream, proto_level_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendChar err %d", ret);

        return ret;
    }

    uint8_t connect_flags{0x0};
    connect_flags |= ((clean_session_ ? 0x1 : 0x0) << 1);
    connect_flags |= ((will_flag_ ? 0x1 : 0x0) << 2);
    connect_flags |= (will_qos_ << 3);
    connect_flags |= ((will_retain_ ? 0x1 : 0x0) << 5);
    connect_flags |= ((user_name_flag_ ? 0x0 : 0x1) << 6);
    connect_flags |= ((password_flag_ ? 0x0 : 0x1) << 7);
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
    ReturnCode ret{RecvUnicode(in_stream, proto_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

        return ret;
    }

    if ("MQTT" != proto_name_) {
        phxrpc::log(LOG_ERR, "violate mqtt protocol");

        return ReturnCode::ERROR_VIOLATE_PROTOCOL;
    }

    char proto_level{'\0'};
    ret = RecvChar(in_stream, proto_level);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }
    proto_level_ = proto_level;

    char connect_flags{0x0};
    ret = RecvChar(in_stream, connect_flags);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", ret);

        return ret;
    }
    clean_session_ = (0x0 != (connect_flags & 0x2));
    will_flag_ = (0x0 != (connect_flags & 0x4));
    will_qos_ = ((connect_flags & 0x8) >> 3);
    will_qos_ |= ((connect_flags & 0x10) >> 3);
    will_retain_ = (0x0 != (connect_flags & 0x20));
    user_name_flag_ = (0x0 != (connect_flags & 0x40));
    password_flag_ = (0x0 != (connect_flags & 0x80));

    ret = RecvUint16(in_stream, keep_alive_);
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUint16 err %d", ret);

        return ret;
    }

    return ret;
}

ReturnCode MqttConnect::SendPayload(ostringstream &out_stream) const {
    ReturnCode ret{SendUnicode(out_stream, client_identifier_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

        return ret;
    }

    if (will_flag_) {
        ret = SendUnicode(out_stream, will_topic_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }

        ret = SendUnicode(out_stream, will_message_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }
    }

    if (user_name_flag_) {
        ret = SendUnicode(out_stream, user_name_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }
    }

    if (password_flag_) {
        ret = SendUnicode(out_stream, password_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

            return ret;
        }
    }

    return ret;
}

ReturnCode MqttConnect::RecvPayload(istringstream &in_stream) {
    ReturnCode ret{RecvUnicode(in_stream, client_identifier_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

        return ret;
    }

    if (will_flag_) {
        ret = RecvUnicode(in_stream, will_topic_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }

        ret = RecvUnicode(in_stream, will_message_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }
    }

    if (user_name_flag_) {
        ret = RecvUnicode(in_stream, user_name_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }
    }

    if (password_flag_) {
        ret = RecvUnicode(in_stream, password_);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }
    }

    return ret;
}


MqttConnack::MqttConnack() {
    set_protocol(Protocol::MQTT_CONNECT);
    mutable_fixed_header().control_packet_type = ControlPacketType::CONNACK;
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


MqttPublish::MqttPublish() {
    set_protocol(Protocol::MQTT_PUBLISH);
    SetURI("/phxrpc/mqtt/PhxMqttPublish");
    mutable_fixed_header().control_packet_type = ControlPacketType::PUBLISH;
}

ReturnCode MqttPublish::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPublishPb publish;

    publish.set_dup(fixed_header().dup);
    publish.set_qos(fixed_header().qos);
    publish.set_retain(fixed_header().retain);

    publish.set_topic_name(topic_name_);
    publish.set_content(GetContent());
    publish.set_packet_identifier(packet_identifier());

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

    FixedHeader &fixed_header(mutable_fixed_header());
    fixed_header.dup = publish.dup();
    fixed_header.qos = publish.qos();
    fixed_header.retain = publish.retain();

    topic_name_ = publish.topic_name();
    SetContent(publish.content().data(), publish.content().length());
    set_packet_identifier(publish.packet_identifier());

    return ReturnCode::OK;
}

BaseResponse *MqttPublish::GenResponse() const { return new MqttFakeResponse; }

ReturnCode MqttPublish::SendVariableHeader(ostringstream &out_stream) const {
    ReturnCode ret{SendUnicode(out_stream, topic_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "SendUnicode err %d", ret);

        return ret;
    }

    if (0 < fixed_header().qos) {
        ret = SendPacketIdentifier(out_stream);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "SendPacketIdentifier err %d", ret);

            return ret;
        }
    }

    return ret;
}

ReturnCode MqttPublish::RecvVariableHeader(istringstream &in_stream) {
    ReturnCode ret{RecvUnicode(in_stream, topic_name_)};
    if (ReturnCode::OK != ret) {
        phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

        return ret;
    }

    if (0 < fixed_header().qos) {
        ret = RecvPacketIdentifier(in_stream);
        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvPacketIdentifier err %d", ret);

            return ret;
        }
    }

    return ret;
}

ReturnCode MqttPublish::SendPayload(ostringstream &out_stream) const {
    return SendChars(out_stream, GetContent().data(), GetContent().size());
}

ReturnCode MqttPublish::RecvPayload(istringstream &in_stream) {
    int variable_header_length{static_cast<int>(topic_name_.length()) + 2};
    if (0 < fixed_header().qos) {
        variable_header_length += 2;
    }
    const int payload_length{remaining_length() - variable_header_length};
    if (0 == payload_length)
      return ReturnCode::OK;
    if (0 > payload_length)
      return ReturnCode::ERROR_LENGTH_UNDERFLOW;

    string &payload_buffer(GetContent());
    payload_buffer.resize(payload_length);
    return RecvChars(in_stream, &payload_buffer[0], payload_length);
}


MqttPuback::MqttPuback() {
    set_protocol(Protocol::MQTT_PUBACK);
    SetURI("/phxrpc/mqtt/PhxMqttPuback");
    mutable_fixed_header().control_packet_type = ControlPacketType::PUBACK;
}

ReturnCode MqttPuback::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPubackPb puback;

    puback.set_packet_identifier(packet_identifier());

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

    set_packet_identifier(puback.packet_identifier());

    return ReturnCode::OK;
}

BaseResponse *MqttPuback::GenResponse() const { return new MqttFakeResponse; }

ReturnCode MqttPuback::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttPuback::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}


MqttPubrec::MqttPubrec() {
    set_protocol(Protocol::MQTT_PUBREC);
    SetURI("/phxrpc/mqtt/PhxMqttPubrec");
    mutable_fixed_header().control_packet_type = ControlPacketType::PUBREC;
}

ReturnCode MqttPubrec::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPubrecPb pubrec;

    try {
        message->CopyFrom(pubrec);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPubrec::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPubrecPb pubrec;

    try {
        pubrec.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

BaseResponse *MqttPubrec::GenResponse() const { return new MqttFakeResponse; }

ReturnCode MqttPubrec::SendVariableHeader(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrec::RecvVariableHeader(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrec::SendPayload(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrec::RecvPayload(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}


MqttPubrel::MqttPubrel() {
    set_protocol(Protocol::MQTT_PUBREL);
    SetURI("/phxrpc/mqtt/PhxMqttPubrel");
    mutable_fixed_header().control_packet_type = ControlPacketType::PUBREL;
}

ReturnCode MqttPubrel::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPubrelPb pubrel;

    try {
        message->CopyFrom(pubrel);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPubrel::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPubrelPb pubrel;

    try {
        pubrel.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

BaseResponse *MqttPubrel::GenResponse() const { return new MqttFakeResponse; }

ReturnCode MqttPubrel::SendVariableHeader(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrel::RecvVariableHeader(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrel::SendPayload(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubrel::RecvPayload(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}


MqttPubcomp::MqttPubcomp() {
    set_protocol(Protocol::MQTT_PUBCOMP);
    SetURI("/phxrpc/mqtt/PhxMqttPubcomp");
    mutable_fixed_header().control_packet_type = ControlPacketType::PUBCOMP;
}

ReturnCode MqttPubcomp::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPubcompPb pubcomp;

    try {
        message->CopyFrom(pubcomp);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPubcomp::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPubcompPb pubcomp;

    try {
        pubcomp.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

BaseResponse *MqttPubcomp::GenResponse() const { return new MqttFakeResponse; }

ReturnCode MqttPubcomp::SendVariableHeader(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubcomp::RecvVariableHeader(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubcomp::SendPayload(ostringstream &out_stream) const {
    return ReturnCode::ERROR_UNIMPLEMENT;
}

ReturnCode MqttPubcomp::RecvPayload(istringstream &in_stream) {
    return ReturnCode::ERROR_UNIMPLEMENT;
}


MqttSubscribe::MqttSubscribe() {
    set_protocol(Protocol::MQTT_SUBSCRIBE);
    SetURI("/phxrpc/mqtt/PhxMqttSubscribe");
    mutable_fixed_header().control_packet_type = ControlPacketType::SUBSCRIBE;
}

ReturnCode MqttSubscribe::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttSubscribePb subscribe;

    subscribe.set_packet_identifier(packet_identifier());
    google::protobuf::RepeatedPtrField<string> temp_topic_filters(
            topic_filters_.begin(), topic_filters_.end());
    subscribe.mutable_topic_filters()->Swap(&temp_topic_filters);
    google::protobuf::RepeatedField<uint32_t> temp_qoss(
            qoss_.begin(), qoss_.end());
    subscribe.mutable_qoss()->Swap(&temp_qoss);

    try {
        message->CopyFrom(subscribe);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttSubscribe::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttSubscribePb subscribe;

    try {
        subscribe.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    set_packet_identifier(subscribe.packet_identifier());
    copy(subscribe.topic_filters().begin(), subscribe.topic_filters().end(),
         back_inserter(topic_filters_));
    copy(subscribe.qoss().begin(), subscribe.qoss().end(),
         back_inserter(qoss_));

    return ReturnCode::OK;
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
    const int variable_header_length{2};
    const int payload_length{remaining_length() - variable_header_length};
    if (0 == payload_length)
      return ReturnCode::OK;
    if (0 > payload_length)
      return ReturnCode::ERROR_LENGTH_UNDERFLOW;

    int used_length{0};
    while (used_length < payload_length && EOF != in_stream.peek()) {
        string topic_filter;
        ReturnCode ret{RecvUnicode(in_stream, topic_filter)};
        if (ReturnCode::ERROR_LENGTH_OVERFLOW == ret) {
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

        used_length += topic_filter.length() + 3;
    }

    return ReturnCode::OK;
}


MqttSuback::MqttSuback() {
    set_protocol(Protocol::MQTT_SUBSCRIBE);
    mutable_fixed_header().control_packet_type = ControlPacketType::SUBACK;
}

ReturnCode MqttSuback::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttSubackPb suback;

    suback.set_packet_identifier(packet_identifier());
    google::protobuf::RepeatedField<uint32_t> temp_return_codes(
            return_codes_.begin(), return_codes_.end());
    suback.mutable_return_codes()->Swap(&temp_return_codes);

    try {
        message->CopyFrom(suback);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttSuback::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttSubackPb suback;

    try {
        suback.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    set_packet_identifier(suback.packet_identifier());
    copy(suback.return_codes().begin(), suback.return_codes().end(),
         back_inserter(return_codes_));

    return ReturnCode::OK;
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
        if (ReturnCode::ERROR_LENGTH_OVERFLOW == ret) {
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


MqttUnsubscribe::MqttUnsubscribe() {
    set_protocol(Protocol::MQTT_UNSUBSCRIBE);
    SetURI("/phxrpc/mqtt/PhxMqttUnsubscribe");
    mutable_fixed_header().control_packet_type = ControlPacketType::UNSUBSCRIBE;
}

ReturnCode MqttUnsubscribe::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttUnsubscribePb unsubscribe;

    unsubscribe.set_packet_identifier(packet_identifier());
    google::protobuf::RepeatedPtrField<string> temp_topic_filters(
            topic_filters_.begin(), topic_filters_.end());
    unsubscribe.mutable_topic_filters()->Swap(&temp_topic_filters);

    try {
        message->CopyFrom(unsubscribe);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttUnsubscribe::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttUnsubscribePb unsubscribe;

    try {
        unsubscribe.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    set_packet_identifier(unsubscribe.packet_identifier());
    copy(unsubscribe.topic_filters().begin(), unsubscribe.topic_filters().end(),
         back_inserter(topic_filters_));

    return ReturnCode::OK;
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
    const int variable_header_length{2};
    const int payload_length{remaining_length() - variable_header_length};
    if (0 == payload_length)
      return ReturnCode::OK;
    if (0 > payload_length)
      return ReturnCode::ERROR_LENGTH_UNDERFLOW;

    int used_length{0};
    while (used_length < payload_length && EOF != in_stream.peek()){
        string topic_filter;
        ReturnCode ret{RecvUnicode(in_stream, topic_filter)};
        if (ReturnCode::ERROR_LENGTH_OVERFLOW == ret) {
            return ReturnCode::OK;
        }

        if (ReturnCode::OK != ret) {
            phxrpc::log(LOG_ERR, "RecvUnicode err %d", ret);

            return ret;
        }

        topic_filters_.emplace_back(topic_filter);

        used_length += topic_filter.length() + 2;
    }

    return ReturnCode::OK;
}


MqttUnsuback::MqttUnsuback() {
    set_protocol(Protocol::MQTT_UNSUBSCRIBE);
    mutable_fixed_header().control_packet_type = ControlPacketType::UNSUBACK;
}

ReturnCode MqttUnsuback::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttUnsubackPb unsuback;

    unsuback.set_packet_identifier(packet_identifier());

    try {
        message->CopyFrom(unsuback);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttUnsuback::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttUnsubackPb unsuback;

    try {
        unsuback.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    set_packet_identifier(unsuback.packet_identifier());

    return ReturnCode::OK;
}

ReturnCode MqttUnsuback::SendVariableHeader(ostringstream &out_stream) const {
    return SendPacketIdentifier(out_stream);
}

ReturnCode MqttUnsuback::RecvVariableHeader(istringstream &in_stream) {
    return RecvPacketIdentifier(in_stream);
}


MqttPingreq::MqttPingreq() {
    set_protocol(Protocol::MQTT_PING);
    SetURI("/phxrpc/mqtt/PhxMqttPing");
    mutable_fixed_header().control_packet_type = ControlPacketType::PINGREQ;
}

ReturnCode MqttPingreq::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPingreqPb pingreq;

    try {
        message->CopyFrom(pingreq);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPingreq::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPingreqPb pingreq;

    try {
        pingreq.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

BaseResponse *MqttPingreq::GenResponse() const { return new MqttPingresp; }


MqttPingresp::MqttPingresp() {
    set_protocol(Protocol::MQTT_PING);
    mutable_fixed_header().control_packet_type = ControlPacketType::PINGRESP;
}

ReturnCode MqttPingresp::ToPb(google::protobuf::Message *const message) const {
    phxrpc::MqttPingrespPb pingresp;

    try {
        message->CopyFrom(pingresp);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}

ReturnCode MqttPingresp::FromPb(const google::protobuf::Message &message) {
    phxrpc::MqttPingrespPb pingresp;

    try {
        pingresp.CopyFrom(message);
    } catch (exception) {
        return ReturnCode::ERROR;
    }

    return ReturnCode::OK;
}


MqttDisconnect::MqttDisconnect() {
    set_protocol(Protocol::MQTT_DISCONNECT);
    SetURI("/phxrpc/mqtt/PhxMqttDisconnect");
    mutable_fixed_header().control_packet_type = ControlPacketType::DISCONNECT;
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

BaseResponse *MqttDisconnect::GenResponse() const { return new MqttFakeResponse; }


}  // namespace phxrpc

