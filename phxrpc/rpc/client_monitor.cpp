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

#include <memory>
#include <stdint.h>
#include <stdio.h>

#include "client_monitor.h"


namespace phxrpc {


ClientMonitor::ClientMonitor() {
}

ClientMonitor::~ClientMonitor() {
}

void ClientMonitor::ClientConnect(bool result) {
}

void ClientMonitor::SendBytes(size_t bytes) {
}

void ClientMonitor::RecvBytes(size_t bytes) {
}

void ClientMonitor::RequestCost(uint64_t begin_time, uint64_t end_time) {
}

void ClientMonitor::SendError() {
}

void ClientMonitor::SendCount() {
}

void ClientMonitor::RecvError() {
}

void ClientMonitor::RecvCount() {
}

void ClientMonitor::GetEndpointFail() {
}

void ClientMonitor::ClientCall(const int cmd_id, const char *method_name) {
}


}  // namespace phxrpc

