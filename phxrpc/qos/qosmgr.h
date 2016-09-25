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

#include "priority_config.h"

#define DEFAULT_USER_PRIORITY_CNT 1024

namespace phxrpc {

class FastRejectQoSMgr
{
public:
    static FastRejectQoSMgr * GetDefault();

    FastRejectQoSMgr();
    ~FastRejectQoSMgr();

    int Init(const char * business_priority_conf,
            int user_priority_cnt,
            int user_priority_elevate_percent,
            int user_priority_lower_percent);

    bool IsReject(const char * business_name,
            const char * user_name);
    bool IsReject(const char * http_header_qos_value);

    int ElevatePriority();
    int LowerPriority();

    bool IsLowestPriority(int business_priority,
            int user_priority);

    int LowerPriority(int curr_business_priority, 
            int curr_user_priority,
            int * new_business_priority,
            int * new_user_priority);

    bool IsRejectCliReq(const char * req_qos_info,
            int svr_business_priority,
            int svr_user_priority);

    int GetQoSInfo(int * business_priority,
            int * user_priority);

public:
    static void SetReqQoSInfo(const char * req_qos_info);
    static const char * GetReqQoSInfo();
 
private:
    int GetUserPriority(const char * user_name);
    void LogStat();

    BusinessPriorityConfig business_priority_config_;
    int user_priority_cnt_;
    int business_priority_cnt_;
    int curr_business_priority_;
    int curr_user_priority_;
    int user_priority_elevate_cnt_;
    int user_priority_lower_cnt_;
    bool is_init_;
};




}
