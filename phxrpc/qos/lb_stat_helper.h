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

#include <stdint.h>
#include "lb_stat_hash.h"
#include "lbfrtablemgr.h"
#include "qosmgr.h"

namespace phxrpc {

typedef void (*lb_stat_regular_report_pfn) (lb_key_mng *,uint32_t ); 
void lb_stat_set_regular_report_pfn(lb_stat_regular_report_pfn pfn);
void lb_stat_run(int worker_cnt,
        int keycnt,
        const char * business_priority_conf,
        int user_priority_cnt,
        int user_priority_elevate_percent,
        int user_priority_lower_percent);
void lb_stat_stop();
void lb_stat_attach(int worker_idx);
void lb_stat_attach_munmap(int worker_idx);

void lb_stat_report(const char *remote_ip_str, uint16_t remote_port, int32_t business_priority, int32_t user_priority);
void lb_stat_report(uint32_t remote_ip, uint16_t remote_port, int32_t business_priority, int32_t user_priority);


//-----------------------------------------------------------------------------------------------

class CliFRMgr
{
public:
    static CliFRMgr* GetDefault();

    CliFRMgr();
    virtual ~CliFRMgr();

    int Init(const char * business_priority_conf,
            int user_priority_cnt,
            int user_priority_elevate_percent,
            int user_priority_lower_percent);

    void Run();
    void Stop();

private:
    bool IsReportTupleSkip(lb_key_item_t * key_item );
    int AddReportFRInfoData(lb_key_mng * key_mng );
    void ClearFRRouteIPLevelInfoMap(FRRouteIPLevelInfoMap& info_map);
    bool IsNoramlFRLevelInfo(const FRLevelInfo& info);
    bool IsNoramlFRLevelInfo(int business_priority,
            int user_priority);
    int PeriodGenerateFRLevelInfo(const struct timeval& period_time);
    int ProcessPeriodFRReportData(const struct timeval& period_time);

private:
    struct timeval curr_time_;
    struct timeval last_process_time_;

    FRRouteIPInfo fr_route_ip_info_;
    FRLevelInfo fr_level_info_;
    FRRouteIPLevelInfoMap period_info_map_;
    FRRouteIPLevelInfoMap history_info_map_;

    LBFastRejectLevelTableMgr table_mgr_;
    FastRejectQoSMgr * qos_mgr_;
    bool is_last_strategy_not_empty_;
    bool is_init_;
};


}



