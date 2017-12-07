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


namespace phxrpc {


enum class ReturnCode {
    OK = 0,
    ERROR = -1,
    ERROR_UNIMPLEMENT = -101,
    ERROR_STEAM_BAD_OR_FAILED = -102,
    ERROR_LENGTH_OVERFLOW = -103,
    ERROR_SOCKET_STREAM_TIMEOUT = -202,
    ERROR_SOCKET_STREAM_NORMAL_CLOSED = -303,
    MAX,
};


enum class Direction {
    NONE = 0,
    REQUEST,
    RESPONSE,
    MAX,
};

enum class Protocol {
    NONE = 0,
    HTTP_GET = 101,
    HTTP_POST = 102,
    HTTP_HEAD = 103,
    MQTT_CONNECT = 201,
    MQTT_PUBLISH = 202,
    MQTT_PUBREL = 203,
    MQTT_SUBSCRIBE = 204,
    MQTT_UNSUBSCRIBE = 205,
    MQTT_PING = 206,
    MQTT_DISCONNECT = 207,
    MQTT_FAKE_DISCONNACK = 208,
    MAX,
};


}

