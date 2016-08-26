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

#include "server_monitor.h"

#include <stdint.h>
#include <stdio.h>

namespace phxrpc {

//ServerMonitor begin
ServerMonitor :: ServerMonitor() {
}

ServerMonitor :: ~ServerMonitor() {
}

void ServerMonitor :: Accept( int count) {
}

void ServerMonitor :: AcceptFail( int count ) {
}

void ServerMonitor :: RequestCount( int count ) {
}

void ServerMonitor :: ResponseCount( int count ) {
}

void ServerMonitor :: SendBytes( size_t bytes ) {
}

void ServerMonitor :: RecvBytes( size_t bytes ) {
}

void ServerMonitor :: RequestCost( uint64_t cost_ms ) {
}

void ServerMonitor :: ReadError( int count ) {
}

void ServerMonitor :: SendError( int count ) {
}

void ServerMonitor :: OutOfQueue( int count ) {
}

void ServerMonitor :: QueueDelay( uint64_t cost_ms ) {
}

void ServerMonitor :: FastRejectAfterAccept( int count ) {
}
    
void ServerMonitor :: FastRejectAfterRead( int count ) {
}

void ServerMonitor :: WrokerInQueueTimeout( int count ) {
}

void ServerMonitor :: WaitInInQueue( uint64_t cost_ms ) {
}

void ServerMonitor :: WaitInOutQueue( uint64_t cost_ms ) {
}

void ServerMonitor :: SvrCall( int cmdid, const char * method_name, int count ) {
}

//ServerMonitor end

}

