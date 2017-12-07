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

#include "phxrpc/mqtt.h"
#include "phxrpc/rpc/phxrpc.pb.h"

#include "client_monitor.h"


namespace google {

namespace protobuf {


class MessageLite;


}

}


namespace phxrpc {


class BaseTcpStream;

class MqttCaller {
  public:
    MqttCaller(BaseTcpStream &socket, ClientMonitor &client_monitor);

    virtual ~MqttCaller();

    MqttConnect &GetConnect();
    MqttPublish &GetPublish();
    MqttDisconnect &GetDisconnect();

    MqttConnack &GetConnack();
    MqttPuback &GetPuback();

    int PhxMqttConnectCall(const phxrpc::MqttConnectPb &connect,
                           phxrpc::MqttConnackPb *connack);
    int PhxMqttPublishCall(const phxrpc::MqttPublishPb &publish,
                           phxrpc::MqttPubackPb *puback);
    int PhxMqttDisconnectCall(const phxrpc::MqttDisconnectPb &disconnect);

    void SetCmdId(const int cmd_id);

  private:
    void MonitorReport(phxrpc::ClientMonitor &client_monitor, bool send_error,
                       bool recv_error, size_t send_size, size_t recv_size,
                       uint64_t call_begin, uint64_t call_end);

    BaseTcpStream &socket_;
    ClientMonitor &client_monitor_;
    int cmd_id_;

    MqttConnect connect_;
    MqttConnack connack_;
    MqttPublish publish_;
    MqttPuback puback_;
    MqttDisconnect disconnect_;

    uint16_t packet_identifier_{1};
};


}  // namespace phxrpc

