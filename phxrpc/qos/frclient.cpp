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

#include "phxrpc/file.h"
#include "qosmgr.h"
#include "frclient.h"

namespace phxrpc {

FRClient * FRClient::GetDefault() {
    static FRClient client;
    return &client;
}

FRClient::FRClient() {
    is_init_ = false;
}

FRClient::~FRClient() {

}

int FRClient::Init() {
    if(!is_init_) {
        if(0 == table_mgr_.Attach()) {

            is_init_ = true;
            return 0;
        } else {
            return -1;
        }
    }
    return 0;
}

bool FRClient::IsSvrBlocked(const uint32_t ip, const uint32_t port,
        const char * req_qos_info) {
    if(!is_init_ && (0 != Init())) {
        return false;
    }

    int32_t svr_business_priority = 0; 
    int32_t svr_user_priority = 0;
	int32_t ret = table_mgr_.GetFRLevelInfo(ip, port, 
            svr_business_priority, svr_user_priority);
    if(0 != ret) {
        return false;
    }

    bool is_blocked = FastRejectQoSMgr::GetDefault()->IsRejectCliReq(req_qos_info,
            svr_business_priority,
            svr_user_priority);

    log(LOG_DEBUG, "FRClient::%s ip %u port %u svr_business_priority %d "
            "svr_user_priority %d req_qos_info %s is_blocked %d",
            __func__,
            ip, port,
            svr_business_priority, svr_user_priority,
            req_qos_info, is_blocked);
    return is_blocked;
}






}



