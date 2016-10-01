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

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <pthread.h>
#include <assert.h>

#include "phxrpc/file.h"
#include "lb_stat_helper.h"

namespace phxrpc {

static __thread lb_key_mng **g_lb_key_mng = NULL;
static lb_key_mng **lb_key_mng_arr= NULL;
static lb_key_mng **lb_key_mng_arr_src = NULL;
static lb_stat_regular_report_pfn g_lb_stat_regular_report_pfn = NULL;

void lb_stat_set_regular_report_pfn(lb_stat_regular_report_pfn pfn)
{
    g_lb_stat_regular_report_pfn = pfn;
}

static int *g_lb_stat_stop = NULL;
void lb_stat_stop()
{
    if (g_lb_stat_stop) *g_lb_stat_stop = 1;
    CliFRMgr::GetDefault()->Stop();
}

static int g_worker_cnt = 0;
static int g_fr_run = 0;

static void *lb_stat_run_func(void *args)
{
    g_fr_run = 1;
    CliFRMgr::GetDefault()->Run();
    return NULL;
}

void lb_stat_run(int worker_cnt,
        int keycnt,
        const char * business_priority_conf,
        int user_priority_cnt,
        int user_priority_elevate_percent,
        int user_priority_lower_percent)
{
    //init
    g_lb_stat_stop = (int *)mmap(NULL, sizeof(int), 
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0);
    if ( !g_lb_stat_stop ) assert(false);
    *g_lb_stat_stop = 0;
    g_worker_cnt = worker_cnt;
    int size = worker_cnt*sizeof(lb_key_mng *);
    lb_key_mng_arr = (lb_key_mng **)mmap(NULL, size,
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0);
    lb_key_mng_arr_src = (lb_key_mng **)calloc(worker_cnt*2, sizeof(lb_key_mng *));

    for ( int i = 0; i < worker_cnt; i++ )
    {
        lb_key_mng_arr_src[ i*2 ] = new lb_key_mng( keycnt, keycnt );
        lb_key_mng_arr_src[ i*2+1 ] = new lb_key_mng( keycnt, keycnt );
        lb_key_mng_arr[i] = lb_key_mng_arr_src[ i*2 ];
    }


    CliFRMgr::GetDefault()->Init(business_priority_conf,
        user_priority_cnt,
        user_priority_elevate_percent,
        user_priority_lower_percent);

    pthread_t tid ;
    pthread_create ( &tid, NULL, lb_stat_run_func, NULL);
    pthread_detach ( tid );
}

void lb_stat_attach(int worker_idx)
{
    if (lb_key_mng_arr==NULL) return;
    g_lb_key_mng = &(lb_key_mng_arr[worker_idx]);
}


void lb_stat_attach_munmap(int worker_idx)
{
    if (lb_key_mng_arr==NULL) return;
    g_lb_key_mng = &(lb_key_mng_arr[worker_idx]);
    for ( int i = 0; i < g_worker_cnt; i++ )
    {
        if (i != worker_idx)
        {
            if (lb_key_mng_arr_src[ i*2 ])
            {
                delete lb_key_mng_arr_src[ i*2 ];
                lb_key_mng_arr_src[ i*2 ] = NULL;
            }
            if (lb_key_mng_arr_src[ i*2 +1 ])
            {
                delete lb_key_mng_arr_src[ i*2+1 ];
                lb_key_mng_arr_src[ i*2+1 ] = NULL;
            }
        }
    }

}

void lb_stat_report(const char* remote_ip_str, uint16_t remote_port, int32_t business_priority, int32_t user_priority)
{
    uint32_t remote_ip = 0;
    struct in_addr addr;
    memset(&addr, 0x0, sizeof(in_addr));
    if ( inet_aton(remote_ip_str, &addr) )
    {
        remote_ip = (uint32_t)addr.s_addr;
    }

    lb_stat_report(remote_ip, remote_port, business_priority, user_priority);
}

void lb_stat_report(uint32_t remote_ip, uint16_t remote_port, int32_t business_priority, int32_t user_priority)
{
    if ((g_lb_key_mng == NULL)|| (*g_lb_key_mng == NULL)) return;
    lb_key_t key = {0};
    key.remote_ip = remote_ip;
    key.remote_port = remote_port;

    lb_key_item_t *key_item = (*g_lb_key_mng)->at( key );	
    if ( !key_item ) return;

    phxrpc::log(LOG_DEBUG, "%s remote_ip %u remote_port %u "
            "business_priority %d:%d user_priority %d:%d",
            __func__,
            remote_ip, remote_port,
            business_priority, key_item->business_priority,
            user_priority, key_item->user_priority);

    if (0 == key_item->business_priority ||
            0 == key_item->user_priority) {
        key_item->business_priority = business_priority;
        key_item->user_priority = user_priority;
    }
    
    if (key_item->business_priority > business_priority)
    {
        key_item->business_priority = business_priority;
        key_item->user_priority = user_priority;
    }
    else if (key_item->business_priority == business_priority
            && key_item->user_priority > user_priority)
    {
        key_item->user_priority = user_priority;
    }

    phxrpc::log(LOG_DEBUG, "%s remote_ip %u remote_port %u "
            "business_priority %d:%d user_priority %d:%d",
            __func__,
            remote_ip, remote_port,
            business_priority, key_item->business_priority,
            user_priority, key_item->user_priority);


}


//-----------------------------------------------------------------------------------------------------------

CliFRMgr* CliFRMgr::GetDefault()
{
    static CliFRMgr mgr;
    return &mgr;
}

CliFRMgr::CliFRMgr()
{
    memset(&curr_time_, 0x0, sizeof(struct timeval));
    memset(&last_process_time_, 0x0, sizeof(struct timeval));
    is_last_strategy_not_empty_ = false;
    is_init_                    = false;
}

CliFRMgr::~CliFRMgr()
{
}

int CliFRMgr::Init(const char * business_priority_conf,
        int user_priority_cnt,
        int user_priority_elevate_percent,
        int user_priority_lower_percent)
{
    if (is_init_)
    {
        return 0;
    }

    if (table_mgr_.Create() != 0)
    {
        return -1;
    }

    qos_mgr_ = FastRejectQoSMgr::GetDefault();

    if (qos_mgr_->Init(business_priority_conf,
                user_priority_cnt,
                user_priority_elevate_percent,
                user_priority_lower_percent) == 0)
    {
        is_init_ = true;
        return 0;
    } else {
        table_mgr_.Detach();
        return -1;
    }
}

bool CliFRMgr::IsReportTupleSkip(lb_key_item_t * key_item )
{
    if (0 == key_item->business_priority && 0 == key_item->user_priority)
    {
        return true;
    }
    return false;
}

int CliFRMgr::AddReportFRInfoData(lb_key_mng * key_mng )
{
    bool is_skip_tuple         = false;
    int key_cnt                = key_mng->get_key_cnt();

    phxrpc::log(LOG_DEBUG, "%s key_cnt %d ", __func__,  key_cnt);

    lb_key_item_t * key_item = NULL;
    for (int i = 0; i < key_cnt; ++i)
    {
        key_item = key_mng->get_key(i);

        phxrpc::log(LOG_DEBUG, "%s ip %u port %u business_priority %d "
                "user_priority %d ",
                __func__, 
                key_item->key.remote_ip, key_item->key.remote_port,
                key_item->business_priority, key_item->user_priority);


        is_skip_tuple = false;
        if (IsReportTupleSkip(key_item))
        {
            is_skip_tuple = true;
        }

        if (is_skip_tuple)
        {
            fr_route_ip_info_.SetFRRouteIPInfo(key_item->key.remote_ip, key_item->key.remote_port);
            FRRouteIPLevelInfoMap::iterator iter = history_info_map_.find(fr_route_ip_info_);
            if (history_info_map_.end() != iter)
            {
                history_info_map_.erase(iter);
            }
            continue;
        }

        fr_route_ip_info_.SetFRRouteIPInfo(key_item->key.remote_ip, key_item->key.remote_port);

        FRRouteIPLevelInfoMap::iterator find_iter = period_info_map_.find(fr_route_ip_info_);
        if (period_info_map_.end() != find_iter)
        {
            phxrpc::log(LOG_DEBUG, "%s ip %u port %u business_priority %d "
                    "user_priority %d ",
                    __func__, 
                    key_item->key.remote_ip, key_item->key.remote_port,
                    key_item->business_priority, key_item->user_priority);

            fr_level_info_.SetFRLevelInfo(key_item->business_priority, key_item->user_priority);
            if (fr_level_info_ > *(find_iter->second))
            {
                find_iter->second->SetFRLevelInfo(fr_level_info_);
            }
        }
        else
        {

            phxrpc::log(LOG_DEBUG, "%s ip %u port %u business_priority %d "
                    "user_priority %d ",
                    __func__, 
                    key_item->key.remote_ip, key_item->key.remote_port,
                    key_item->business_priority, key_item->user_priority);

            FRLevelInfo* curr_level_info = new FRLevelInfo(key_item->business_priority, key_item->user_priority);
            period_info_map_[fr_route_ip_info_] = curr_level_info;
        }
    }
    return 0;
}

void CliFRMgr::ClearFRRouteIPLevelInfoMap(FRRouteIPLevelInfoMap& info_map)
{
    FRRouteIPLevelInfoMap::iterator iter = info_map.begin();
    for (; info_map.end() != iter; ++iter)
    {
        if (iter->second != NULL)
        {
            delete iter->second;
        }
    }
    info_map.clear();
}

bool CliFRMgr::IsNoramlFRLevelInfo(const FRLevelInfo& info)
{
    return qos_mgr_->IsLowestPriority(
            info.business_priority, info.user_priority);
}

bool CliFRMgr::IsNoramlFRLevelInfo(int business_priority,
        int user_priority)
{
    return qos_mgr_->IsLowestPriority(business_priority, user_priority);
}

int CliFRMgr::PeriodGenerateFRLevelInfo(const struct timeval& period_time)
{
    FRRouteIPLevelInfoMap::iterator iter = period_info_map_.begin();
    for (; period_info_map_.end() != iter; ++iter)
    {
        FRRouteIPLevelInfoMap::iterator find_iter = history_info_map_.find(iter->first);
        if (history_info_map_.end() != find_iter)
        {
            find_iter->second->SetFRLevelInfo(*(iter->second));
        }
        else
        {
            if (!IsNoramlFRLevelInfo(*(iter->second)))
            {
                history_info_map_[iter->first] = new FRLevelInfo(*(iter->second));
            }
        }
    }

    bool has_new_strategy = false;
    LBFRLevelTable* curr_write_table = table_mgr_.GetWriteFRLevelTable();
    if (NULL == curr_write_table)
    {
        return -1;
    }
    else
    {
        unsigned int index                       = 0;
        FRRouteIPLevelInfoMap::iterator iter     = history_info_map_.begin();
        FRRouteIPLevelInfoMap::iterator del_iter = iter;
        for (; history_info_map_.end() != iter;)
        {
            bool is_cur_del = false;

            has_new_strategy = true;

            FRLevelInfo* curr_level_info = iter->second;

            if (IsNoramlFRLevelInfo(*(curr_level_info)))
            {
                del_iter = iter++;
                delete curr_level_info;
                history_info_map_.erase(del_iter);
                is_cur_del = true;
            }
            else
            {
                if (false == curr_level_info->is_period_report_update)
                {
                    int business_priority = 0;
                    int user_priority = 0;
                    qos_mgr_->LowerPriority(curr_level_info->business_priority,
                            curr_level_info->user_priority,
                            &business_priority,
                            &user_priority);

                    if (IsNoramlFRLevelInfo(business_priority, user_priority))
                    {
                        del_iter = iter++;
                        delete curr_level_info;
                        history_info_map_.erase(del_iter);
                        is_cur_del = true;
                    }
                    else
                    {
                        curr_level_info->business_priority   = business_priority;
                        curr_level_info->user_priority   = user_priority;
                        curr_level_info->is_period_report_update = false;
                    }
                }
                else
                {
                    curr_level_info->is_period_report_update = false;
                }

                if (!is_cur_del)
                {
                    //log
                }
            }

            if (!is_cur_del)
            {
                if (index > (MAX_FRLEVEL_TUPLE_NUM - 1))
                {
                    continue;
                }

                LBFRLevelTuple& curr_tuple = curr_write_table->fr_level_tuple_array[index++];
                curr_tuple.SetFRLevelTuple(iter->first.ip,
                        iter->first.port,
                        iter->second->business_priority,
                        iter->second->user_priority);
                phxrpc::log(LOG_DEBUG, "%s add fr tuple ip %u port %u business_priority %d "
                        "user_priority %d ",
                        __func__, 
                        iter->first.ip,
                        iter->first.port,
                        iter->second->business_priority,
                        iter->second->user_priority);
                iter++;
            }
        }

        phxrpc::log(LOG_DEBUG, "%s fr_level_tuple_num %d", __func__, index);
        if (has_new_strategy || is_last_strategy_not_empty_)
        {
            curr_write_table->fr_level_tuple_num = index;
            table_mgr_.SwapFRLevelTableFlag();
        }
        is_last_strategy_not_empty_ = has_new_strategy;
    }

    return 0;
}

int CliFRMgr::ProcessPeriodFRReportData(const struct timeval& period_time)
{
    ClearFRRouteIPLevelInfoMap(period_info_map_);
    return 0;
}

void CliFRMgr::Run()
{
    if (!is_init_)
    {
        return;
    }

    int state = 1;
    while (g_fr_run)
    {
        usleep(1000*1000);
        for( int i = 0; i < g_worker_cnt; i++ )
        {       
            lb_key_mng *p = lb_key_mng_arr[i];
            lb_key_mng_arr[i] = lb_key_mng_arr_src[ i*2+state%2];
            AddReportFRInfoData(p);
            p->clear();
        }       
        state++;

        gettimeofday(&curr_time_, NULL);
        int64_t alg_time_cost = (1000 * (curr_time_.tv_sec - last_process_time_.tv_sec) + (curr_time_.tv_usec - last_process_time_.tv_usec) / 1000);

        if (alg_time_cost >= 1000) //hardcode 1 sec
        {
            PeriodGenerateFRLevelInfo(curr_time_);
            ProcessPeriodFRReportData(curr_time_);
            last_process_time_ = curr_time_;
        }
    }
}

void CliFRMgr::Stop()
{
    g_fr_run = 0;
}

}





