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
#include <sstream>

#include "mqtt_msg.h"
#include "mqtt_protocol.h"

#include "phxrpc/file/file_utils.h"
#include "phxrpc/file/opt_map.h"
#include "phxrpc/network/socket_stream_block.h"


using namespace phxrpc;
using namespace std;


void ShowUsage(const char *program) {
    printf("\n%s [-r CONNECT|PUBLISH|SUBSCRIBE|UNSUBSCRIBE|PING|DISCONNECT] [-f file] [-v]\n", program);

    printf("\t-r mqtt method, only support CONNECT|PUBLISH|SUBSCRIBE|UNSUBSCRIBE|PING|DISCONNECT\n");
    printf("\t-f the file for content\n");
    printf("\t-v show this usage\n");
    printf("\n");

    exit(0);
}

void TraceMsg(const MqttMessage &msg) {
    ostringstream ss_req;
    msg.SendRemaining(ss_req);
    const string &s_req(ss_req.str());
    cout << s_req.size() << ":" << endl;
    for (int i{0}; s_req.size() > i; ++i) {
        cout << static_cast<int>(s_req.data()[i]) << "\t";
    }
    cout << endl;
    for (int i{0}; s_req.size() > i; ++i) {
        if (isalnum(s_req.data()[i]) || '_' ==  s_req.data()[i])
            cout << s_req.data()[i] << "\t";
        else
            cout << '.' << "\t";
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    assert(sigset(SIGPIPE, SIG_IGN) != SIG_ERR);

    OptMap optMap("r:f:v");

    if ((!optMap.Parse(argc, argv)) || optMap.Has('v'))
        ShowUsage(argv[0]);

    const char *method{optMap.Get('r')};
    const char *file{optMap.Get('f')};

    if (nullptr == method) {
        printf("\nPlease specify method!\n");
        ShowUsage(argv[0]);
    }

    int ret{0};

    if (0 == strcasecmp(method, "CONNECT")) {
        cout << "Req:" << endl;
        TraceMsg(MqttConnect());

        cout << "Resp:" << endl;
        TraceMsg(MqttConnack());
    } else if (0 == strcasecmp(method, "PUBLISH")) {
        cout << "Req:" << endl;
        MqttPublish publish;
        publish.set_topic_name("test_topic_1");
        publish.set_packet_identifier(11);
        TraceMsg(publish);

        cout << "Resp:" << endl;
        MqttPuback puback;
        puback.set_packet_identifier(11);
        TraceMsg(puback);
    } else if (0 == strcasecmp(method, "SUBSCRIBE")) {
        cout << "Req:" << endl;
        TraceMsg(MqttSubscribe());
        cout << "Resp:" << endl;
        TraceMsg(MqttSuback());
    } else if (0 == strcasecmp(method, "UNSUBSCRIBE")) {
        cout << "Req:" << endl;
        TraceMsg(MqttUnsubscribe());
        cout << "Resp:" << endl;
        TraceMsg(MqttUnsuback());
    } else if (0 == strcasecmp(method, "PING")) {
        cout << "Req:" << endl;
        TraceMsg(MqttPingreq());
        cout << "Resp:" << endl;
        TraceMsg(MqttPingresp());
    } else if (0 == strcasecmp(method, "DISCONNECT")) {
        cout << "Req:" << endl;
        TraceMsg(MqttDisconnect());
    } else {
        printf("unsupport method %s\n", method);
    }

    //if (0 == ret) {
    //    printf("response:\n");

    //    printf("%s %d %s\n", response.GetVersion(), response.GetStatusCode(),
    //           response.GetReasonPhrase());

    //    printf("%zu headers\n", response.GetHeaderCount());
    //    for (size_t i{0}; i < response.GetHeaderCount(); ++i) {
    //        const char *name{response.GetHeaderName(i)};
    //        const char *val{response.GetHeaderValue(i)};
    //        printf("%s: %s\r\n", name, val);
    //    }

    //    printf("%zu bytes body\n", response.GetContent().size());
    //    if (response.GetContent().size() > 0) {
    //        //printf("%s\n", (char*)response.getContent());
    //    }
    //} else {
    //    printf("mqtt request fail\n");
    //}

    return 0;
}

