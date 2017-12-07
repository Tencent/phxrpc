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

#include "phxrpc/msg.h"


namespace phxrpc {


class MqttMessage : virtual public BaseMessage {
  public:
    enum class ControlPacketType {
        FAKE_NONE = 0,
        CONNECT = 1,
        CONNACK,
        PUBLISH,
        PUBACK,
        PUBREC,
        PUBREL,
        PUBCOMP,
        SUBSCRIBE,
        SUBACK,
        UNSUBSCRIBE,
        UNSUBACK,
        PINGREQ,
        PINGRESP,
        DISCONNECT,
        FAKE_DISCONNACK,
        FAKE_MAX,
    };

    static const char FixedHeader[];

    //static const bool NeedPacketIdentifier[];

    static int EncodeUint16(std::string &dest, const uint16_t src);
    static int EncodeUint16(char *const dest, const size_t dest_size,
                            const uint16_t src);
    static int EncodeUnicode(std::string &dest, const std::string &src);
    static int EncodeUnicode(char *const dest, const size_t dest_size,
                             const std::string &src);

    static ReturnCode SendChar(std::ostringstream &out_stream, const char &content);
    static ReturnCode RecvChar(std::istringstream &in_stream, char &content);
    static ReturnCode SendUint16(std::ostringstream &out_stream,
                                 const uint16_t content);
    static ReturnCode RecvUint16(std::istringstream &in_stream,
                                 uint16_t &content);
    static ReturnCode SendChars(std::ostringstream &out_stream,
                                const char *const content,
                                const int content_length);
    static ReturnCode RecvChars(std::istringstream &in_stream,
                                char *const content,
                                const int content_length);
    static ReturnCode SendUnicode(std::ostringstream &out_stream,
                                  const std::string &content);
    static ReturnCode RecvUnicode(std::istringstream &in_stream,
                                  std::string &content);

    static ReturnCode SendChar(BaseTcpStream &out_stream, const char &content);
    static ReturnCode RecvChar(BaseTcpStream &in_stream, char &content);
    static ReturnCode SendUint16(BaseTcpStream &out_stream,
                                 const uint16_t content);
    static ReturnCode RecvUint16(BaseTcpStream &in_stream, uint16_t &content);
    static ReturnCode SendChars(BaseTcpStream &out_stream,
                                const char *const content,
                                const int content_length);
    static ReturnCode RecvChars(BaseTcpStream &in_stream,
                                char *const content,
                                const int content_length);
    static ReturnCode SendUnicode(BaseTcpStream &out_stream,
                                  const std::string &content);
    static ReturnCode RecvUnicode(BaseTcpStream &in_stream,
                                  std::string &content);

    // control packet type and flags
    static ReturnCode SendFixedHeaderAndRemainingBuffer(
            BaseTcpStream &out_stream,
            const ControlPacketType control_packet_type,
            const std::string &remaining_buffer);
    static ReturnCode RecvFixedHeaderAndRemainingBuffer(
            BaseTcpStream &in_stream,
            ControlPacketType &control_packet_type,
            std::string &remaining_buffer);

    // remaining length
    static ReturnCode SendRemainingLength(BaseTcpStream &out_stream,
                                          const int remaining_length);
    static ReturnCode RecvRemainingLength(BaseTcpStream &in_stream,
                                          int &remaining_length);

    MqttMessage();
    virtual ~MqttMessage() override;

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const = 0;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) = 0;

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const = 0;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) = 0;

    virtual ReturnCode Send(BaseTcpStream &socket) const override;

    ReturnCode SendRemaining(std::ostringstream &out_stream) const;
    ReturnCode RecvRemaining(std::istringstream &in_stream);

    // packet identifier
    ReturnCode SendPacketIdentifier(std::ostringstream &out_stream) const;
    ReturnCode RecvPacketIdentifier(std::istringstream &in_stream);

    ControlPacketType control_packet_type() const {
        return control_packet_type_;
    }

    int remaining_length() const { return remaining_length_; }

    uint16_t packet_identifier() const { return packet_identifier_; }

    void set_packet_identifier(const uint16_t packet_identifier) {
        packet_identifier_ = packet_identifier;
    }

  protected:
    void set_control_packet_type(const ControlPacketType control_packet_type) {
        control_packet_type_ = control_packet_type;
    }

    void set_remaining_length(const int remaining_length) {
        remaining_length_ = remaining_length;
    }

  private:
    ControlPacketType control_packet_type_{ControlPacketType::FAKE_NONE};
    uint16_t packet_identifier_{0x0};
    int remaining_length_{0};
};


class MqttRequest : public MqttMessage, public BaseRequest {
  public:
    MqttRequest(const Protocol protocol) : BaseRequest(protocol) {}
    virtual ~MqttRequest() = default;

  private:
};


class MqttResponse : public MqttMessage, public BaseResponse {
  public:
    MqttResponse(const Protocol protocol) : BaseResponse(protocol) {}
    virtual ~MqttResponse() = default;

    virtual void SetPhxRpcResult(const int result) override {}
    virtual void DispatchErr() override {}

    virtual ReturnCode ModifyResp(const bool keep_alive, const std::string &version) override;

  private:
};


class MqttConnect final : public MqttRequest {
  public:
    MqttConnect();
    virtual ~MqttConnect() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) override;

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override;

    bool clean_session() const { return clean_session_; }
    void set_clean_session(const bool clean_session) {
        clean_session_ = clean_session;
    }

    uint16_t keep_alive() const { return keep_alive_; }
    void set_keep_alive(const uint16_t keep_alive) {
        keep_alive_ = keep_alive;
    }

    const std::string &client_identifier() const { return client_identifier_; }
    void set_client_identifier(const std::string &client_identifier) {
        client_identifier_ = client_identifier;
    }

    // read only
    const std::string &proto_name() const { return proto_name_; }

    // read only
    char proto_level() const { return proto_level_; }

  private:
    bool clean_session_{false};
    uint16_t keep_alive_{0};
    std::string client_identifier_;
    std::string proto_name_;
    char proto_level_{4};
};


class MqttConnack final : public MqttResponse {
  public:
    MqttConnack();
    virtual ~MqttConnack() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) override;

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }

    bool session_present() const { return session_present_; }
    void set_session_present(const bool session_present) {
        session_present_ = session_present;
    }

    char connect_return_code() const { return connect_return_code_; }
    void set_connect_return_code(const char connect_return_code) {
        connect_return_code_ = connect_return_code;
    }

  private:
    bool session_present_{0};
    char connect_return_code_{0};
};


class MqttPublish final : public MqttRequest {
  public:
    MqttPublish();
    virtual ~MqttPublish() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) override;

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override;

    const std::string &topic_name() const { return topic_name_; }
    void set_topic_name(const std::string &topic_name) {
        topic_name_ = topic_name;
    }

  private:
    std::string topic_name_;
};


class MqttPuback final : public MqttResponse {
  public:
    MqttPuback();
    virtual ~MqttPuback() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) override;

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;
    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


class MqttSubscribe final : public MqttRequest {
  public:
    MqttSubscribe();
    virtual ~MqttSubscribe() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;
    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override;

    const std::vector<std::string> &topic_filters() const {
        return topic_filters_;
    }
    void set_topic_filters(const std::vector<std::string> &topic_filters) {
        topic_filters_ = topic_filters;
    }

  private:
    std::vector<std::string> topic_filters_;
};


class MqttSuback final : public MqttResponse {
  public:
    MqttSuback();
    virtual ~MqttSuback() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;
    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override;

    const std::string &return_codes() const { return return_codes_; }
    void set_return_codes(const std::string &return_codes) {
        return_codes_ = return_codes;
    }

  private:
    std::string return_codes_;
};


class MqttUnsubscribe final : public MqttRequest {
  public:
    MqttUnsubscribe();
    virtual ~MqttUnsubscribe() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;
    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override;

    const std::vector<std::string> &topic_filters() const {
        return topic_filters_;
    }
    void set_topic_filters(const std::vector<std::string> &topic_filters) {
        topic_filters_ = topic_filters;
    }

  private:
    std::vector<std::string> topic_filters_;
};


class MqttUnsuback final : public MqttResponse {
  public:
    MqttUnsuback();
    virtual ~MqttUnsuback() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override;
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override;

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


class MqttPingreq final : public MqttRequest {
  public:
    MqttPingreq();
    virtual ~MqttPingreq() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


class MqttPingresp final : public MqttResponse {
  public:
    MqttPingresp();
    virtual ~MqttPingresp() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


class MqttDisconnect final : public MqttRequest {
  public:
    MqttDisconnect();
    virtual ~MqttDisconnect() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override;
    virtual ReturnCode FromPb(const google::protobuf::Message &message) override;

    virtual BaseResponse *GenResponse() const override;
    virtual int IsKeepAlive() const override { return 1; };

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


class MqttFakeDisconnack final : public MqttResponse {
  public:
    MqttFakeDisconnack();
    virtual ~MqttFakeDisconnack() = default;

    virtual ReturnCode ToPb(google::protobuf::Message *const message) const override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode FromPb(const google::protobuf::Message &message) override {
        return ReturnCode::ERROR_UNIMPLEMENT;
    }

    virtual ReturnCode SendVariableHeader(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvVariableHeader(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }

    virtual ReturnCode SendPayload(std::ostringstream &out_stream) const override {
        return ReturnCode::OK;
    }
    virtual ReturnCode RecvPayload(std::istringstream &in_stream) override {
        return ReturnCode::OK;
    }
};


}  // namespace phxrpc

