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

#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "mqtt_msg.h"
#include "mqtt_client.h"

#include "phxrpc/file/file_utils.h"
#include "phxrpc/file/opt_map.h"
#include "phxrpc/network/socket_stream_block.h"


using namespace phxrpc;
using namespace std;


void ShowUsage(const char *program) {
    printf("\n%s [-h host] [-p port] [-r CONNECT|PUBLISH|SUBSCRIBE|UNSUBSCRIBE|PING|DISCONNECT] [-f file] [-v]\n", program);

    printf("\t-h mqtt host\n");
    printf("\t-p mqtt port\n");
    printf("\t-r mqtt method, only support CONNECT|PUBLISH|SUBSCRIBE|UNSUBSCRIBE|PING|DISCONNECT\n");
    printf("\t-f the file for content\n");
    printf("\t-v show this usage\n");
    printf("\n");

    exit(0);
}

int main(int argc, char *argv[]) {
    assert(sigset(SIGPIPE, SIG_IGN) != SIG_ERR);

    OptMap optMap("h:p:r:f:v");

    if ((!optMap.Parse(argc, argv)) || optMap.Has('v'))
        ShowUsage(argv[0]);

    int port{0};
    const char *host{optMap.Get('h')};
    const char *method{optMap.Get('r')};
    const char *file{optMap.Get('f')};

    if ((nullptr == host) || (!optMap.GetInt('p', &port))) {
        printf("\nPlease specify host and port!\n");
        ShowUsage(argv[0]);
    }

    if (nullptr == method) {
        printf("\nPlease specify method!\n");
        ShowUsage(argv[0]);
    }

    BlockTcpStream socket;
    if (!BlockTcpUtils::Open(&socket, host, port, 100, nullptr, 0)) {
        printf("Connect %s:%d fail\n", host, port);
        exit(-1);
    }

    int ret{0};

    if (0 == strcasecmp(method, "CONNECT")) {
        MqttConnect req;
        MqttConnack resp;
        ret = MqttClient::Connect(socket, req, resp);
        if (0 == ret) {
            printf("mqtt connect ret %d connect_return_code %d\n",
                   ret, resp.connect_return_code());
        } else {
            printf("mqtt connect fail ret %d\n", ret);
        }
    } else if (0 == strcasecmp(method, "PUBLISH")) {
        MqttPublish req;
        MqttPuback resp;
        ret = MqttClient::Publish(socket, req, resp);
        if (0 == ret) {
            printf("mqtt publish ret %d packet_identifier %u\n",
                   ret, resp.packet_identifier());
        } else {
            printf("mqtt publish fail ret %d\n", ret);
        }
    } else if (0 == strcasecmp(method, "SUBSCRIBE")) {
        MqttSubscribe req;
        MqttSuback resp;
        ret = MqttClient::Subscribe(socket, req, resp);
        if (0 == ret) {
            printf("mqtt subscribe ret %d\n", ret);
        } else {
            printf("mqtt subscribe fail ret %d\n", ret);
        }
    } else if (0 == strcasecmp(method, "UNSUBSCRIBE")) {
        MqttUnsubscribe req;
        MqttUnsuback resp;
        ret = MqttClient::Unsubscribe(socket, req, resp);
        if (0 == ret) {
            printf("mqtt unsubscribe ret %d\n", ret);
        } else {
            printf("mqtt unsubscribe fail ret %d\n", ret);
        }
    } else if (0 == strcasecmp(method, "PING")) {
        MqttPingreq req;
        MqttPingresp resp;
        ret = MqttClient::Ping(socket, req, resp);
        if (0 == ret) {
            printf("mqtt ping ret %d\n", ret);
        } else {
            printf("mqtt ping fail ret %d\n", ret);
        }
    } else if (0 == strcasecmp(method, "DISCONNECT")) {
        MqttDisconnect req;
        ret = MqttClient::Disconnect(socket, req);
        if (0 == ret) {
            printf("mqtt disconnect ret %d\n", ret);
        } else {
            printf("mqtt disconnect fail ret %d\n", ret);
        }
    } else {
        printf("unsupport method %s\n", method);
    }

    return 0;
}

