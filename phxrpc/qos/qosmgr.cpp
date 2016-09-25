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

#include <cstring>
#include <vector>
#include <random>
#include <functional>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phxrpc/file.h"
#include "qosmgr.h"

namespace phxrpc {

typedef struct _QoSInfo {
    char req_qos_info[1024];
} QoSInfo;

static __thread QoSInfo g_qos_info;

void FastRejectQoSMgr::SetReqQoSInfo(const char * req_qos_info) {
    if(req_qos_info) {
        snprintf(g_qos_info.req_qos_info, sizeof(g_qos_info.req_qos_info), 
                "%s", req_qos_info);
    } else {
        g_qos_info.req_qos_info[0] = '\0';
    }
}

const char * FastRejectQoSMgr::GetReqQoSInfo() {

    if(g_qos_info.req_qos_info[0] != '\0') {
        return g_qos_info.req_qos_info;
    } else {
        return NULL;
    }
}
    
FastRejectQoSMgr * FastRejectQoSMgr::GetDefault() {
    static FastRejectQoSMgr mgr;
    return &mgr;
}

FastRejectQoSMgr::FastRejectQoSMgr() {
    user_priority_cnt_ = 0;
    curr_user_priority_ = user_priority_cnt_;

    business_priority_cnt_ = 0;
    curr_business_priority_ = 0;

    user_priority_elevate_cnt_ = 0;
    user_priority_lower_cnt_ = 0;
    
    is_init_ = false;
}

FastRejectQoSMgr::~FastRejectQoSMgr() {

}

int FastRejectQoSMgr::Init(const char * business_priority_conf,
        int user_priority_cnt,
        int user_priority_elevate_percent,
        int user_priority_lower_percent) {

    user_priority_cnt_ = user_priority_cnt;
    if(user_priority_cnt < 0) {
        log(LOG_ERR, "FastRejectQoSMgr::%s user_priority_cnt < 0", __func__);
        return -1;
    }

    if(!business_priority_config_.Read(business_priority_conf)) {
        log(LOG_ERR, "FastRejectQoSMgr::%s business_priority_config_ read failed", __func__);
        return -1;
    }

    curr_business_priority_ = business_priority_config_.GetPriorityCnt();
    business_priority_cnt_ = curr_business_priority_;

    curr_user_priority_ = user_priority_cnt;

    user_priority_elevate_cnt_ = (int)((double)user_priority_cnt * ((double)user_priority_elevate_percent / 100.0));
    user_priority_lower_cnt_ = (int)((double)user_priority_cnt * ((double)user_priority_lower_percent / 100.0));


    log(LOG_DEBUG, "FastRejectQoSMgr::%s business_priority_cnt_ %d user_priority_cnt_ %d "
            "user_priority_elevate_cnt_ %d user_priority_lower_cnt_ %d", __func__,
            business_priority_cnt_, user_priority_cnt_,
            user_priority_elevate_cnt_, user_priority_lower_cnt_);

    is_init_ = true;
    return 0;
}

int FastRejectQoSMgr::GetUserPriority(const char * user_name) {
    std::hash<std::string> h;
    return h(user_name) % user_priority_cnt_ + 1;
}

bool FastRejectQoSMgr::IsReject(const char * business_name,
        const char * user_name) {

    if(!is_init_ || 0 == user_priority_cnt_ || 0 == business_priority_cnt_) {
        return false;
    }

    int business_priority = business_priority_config_.GetPriority(business_name);
    int user_priority = GetUserPriority(user_name);

    log(LOG_DEBUG, "FastRejectQoSMgr::%s business_name %s user_name %s "
            "business_priority %d user_priority %d curr_business_priority_ %d curr_user_priority_ %d",
            __func__, business_name, user_name,
            business_priority, user_priority, curr_business_priority_,
            curr_user_priority_);

    if(business_priority < curr_business_priority_) {
        return false;
    }
    if(business_priority > curr_business_priority_) {
        return true;
    }
    if(user_priority <= curr_user_priority_) {
        return false;
    }
    return false;
}


bool FastRejectQoSMgr::IsReject(const char * http_header_qos_value) {

    if(!http_header_qos_value) {
        return false;
    }

    char * pos = strstr(http_header_qos_value, "_");
    if(!pos) {
        return false;
    }

    (*pos) = '\0';

    const char * business_name = http_header_qos_value;
    const char * user_name = pos + 1;

    bool is_reject = IsReject(business_name, user_name);

    (*pos) = '_';

    return is_reject;
}

void FastRejectQoSMgr::LogStat() {
    log(LOG_DEBUG, "FastRejectQoSMgr::%s curr_business_priority_ %d curr_user_priority_ %d", __func__,
            curr_business_priority_, curr_user_priority_);
}

int FastRejectQoSMgr::ElevatePriority() {

    int tmp_business_priority = curr_business_priority_;
    int tmp_user_priority = curr_user_priority_;

    if(1 == curr_business_priority_ && 1 == curr_user_priority_) {
        goto OUT;
    }

    tmp_user_priority = curr_user_priority_ - user_priority_elevate_cnt_;

    if(1 > tmp_user_priority) {
        if(1 < tmp_business_priority) {
            tmp_user_priority = user_priority_cnt_;
            tmp_business_priority -= 1;
        } else {
            tmp_user_priority = 1;
        }
    } 
    curr_business_priority_ = tmp_business_priority;
    curr_user_priority_ = tmp_user_priority;
OUT:
    LogStat();
    return 0;
}

int FastRejectQoSMgr::LowerPriority() {

    int tmp_business_priority = curr_business_priority_;
    int tmp_user_priority = curr_user_priority_;

    if(business_priority_cnt_ == curr_business_priority_ && user_priority_cnt_ == curr_user_priority_) {
        goto OUT;
    }

    tmp_user_priority = curr_user_priority_ + user_priority_lower_cnt_;
    if(tmp_user_priority > user_priority_cnt_) {
        if(tmp_business_priority < business_priority_cnt_) {
            tmp_user_priority = 0;
            tmp_business_priority += 1;
        } else {
            tmp_user_priority = user_priority_cnt_;
        }
    } 

    curr_business_priority_ = tmp_business_priority;
    curr_user_priority_ = tmp_user_priority;
OUT:
    LogStat();
    return 0;
}


bool FastRejectQoSMgr::IsLowestPriority(int business_priority,
        int user_priority) {

    if(business_priority == business_priority_cnt_ &&
            user_priority == user_priority_cnt_) {
        return true;
    } else {
        return false;
    }
}

int FastRejectQoSMgr::LowerPriority(int curr_business_priority, 
        int curr_user_priority,
        int * new_business_priority,
        int * new_user_priority) {

    int tmp_business_priority = curr_business_priority;
    int tmp_user_priority = curr_user_priority + user_priority_lower_cnt_;

    if(tmp_user_priority > user_priority_cnt_) {
        if(tmp_business_priority < business_priority_cnt_) {
            tmp_user_priority = 0;
            tmp_business_priority += 1;
        } else {
            tmp_user_priority = user_priority_cnt_;
        }
    } 

    (*new_business_priority) = tmp_business_priority;
    (*new_user_priority) = tmp_user_priority;

    return 0;
}

int FastRejectQoSMgr::GetQoSInfo(int * business_priority,
        int * user_priority) {
    if(!is_init_ || 0 == user_priority_cnt_ || 0 == business_priority_cnt_) {
        return -1;
    }

    *business_priority = curr_business_priority_;
    *user_priority = curr_user_priority_;
    return 0;
}

bool FastRejectQoSMgr::IsRejectCliReq(const char * req_qos_info,
        int svr_business_priority,
        int svr_user_priority) {

    if(!is_init_ || 0 == user_priority_cnt_ || 0 == business_priority_cnt_) {
        return false;
    }

    if(!req_qos_info) {
        return false;
    }

    char * pos = strstr(req_qos_info, "_");
    if(!pos) {
        return false;
    }

    (*pos) = '\0';

    const char * business_name = req_qos_info;
    const char * user_name = pos + 1;

    int cli_business_priority = business_priority_config_.GetPriority(business_name);
    int cli_user_priority = GetUserPriority(user_name);

    (*pos) = '_';

    log(LOG_DEBUG, "FastRejectQoSMgr::%s req_qos_info %s cli_business_priority %d cli_user_priority %d "
            "svr_business_priority %d svr_user_priority %d", __func__,
            req_qos_info,
            cli_business_priority, cli_user_priority,
            svr_business_priority, svr_user_priority);

    if(cli_business_priority < svr_business_priority) {
        return false;
    }
    if(cli_business_priority > svr_business_priority) {
        return true;
    }
    if(cli_user_priority > svr_user_priority) {
        return true;
    }
    return false;
}


}


