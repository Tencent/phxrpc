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

#include <cstdio>

#include <google/protobuf/message_lite.h>

#include "monitor_factory.h"
#include "http_caller.h"

#include "phxrpc/network.h"
#include "phxrpc/mqtt.h"


using namespace phxrpc;


int main(int argc, char **argv) {
    // 1. connect

    for (size_t i{0}; 20 > i; ++i) {
        phxrpc::BlockTcpStream socket;
        if(phxrpc::BlockTcpUtils::Open(&socket, "127.0.0.1", 26161, 200, nullptr, 0)) {
            socket.SetTimeout(5000);
            MqttConnect req;
            MqttConnack resp;

            int ret{MqttClient::Connect(socket, req, resp)};
            if (0 != ret) {
                printf("try %zu connect fail ret %d\n", i, ret);
                continue;
            }

            printf("try %zu connect ret %d connect_return_code %d\n",
                   i, ret, resp.connect_return_code());
        }
    }

    // 2. publish

    for (size_t i{0}; 20 > i; ++i) {
        phxrpc::BlockTcpStream socket;
        if(phxrpc::BlockTcpUtils::Open(&socket, "127.0.0.1", 26161, 200, nullptr, 0)) {
            socket.SetTimeout(5000);
            MqttPublish req;
            MqttPuback resp;
            req.set_topic_name("test_topic_1");
            req.set_packet_identifier(i);

            int ret{MqttClient::Publish(socket, req, resp)};
            if (0 != ret) {
                printf("try %zu publish fail ret %d\n", i, ret);
                continue;
            }

            printf("try %zu publish ret %d packet_identifier %u\n",
                   i, ret, resp.packet_identifier());
        }
    }

    // 3. disconnect

    for (size_t i{0}; 20 > i; ++i) {
        phxrpc::BlockTcpStream socket;
        if(phxrpc::BlockTcpUtils::Open(&socket, "127.0.0.1", 26161, 200, nullptr, 0)) {
            socket.SetTimeout(5000);
            MqttDisconnect req;

            int ret{MqttClient::Disconnect(socket, req)};
            if (0 != ret) {
                printf("try %zu disconnect fail ret %d\n", i, ret);
                continue;
            }

            printf("try %zu disconnect ret %d\n", i, ret);
        }
    }

    return 0;
}

