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

#include "lbfrtablemgr.h"

namespace phxrpc {

static char * g_mgr_buffer = NULL;

bool operator == (const LBFRLevelTuple& left, const LBFRLevelTuple& right)
{
	return (left.ip == right.ip && left.port == right.port);
}

bool operator < (const LBFRLevelTuple& left, const LBFRLevelTuple& right)
{
	if (left.ip < right.ip)
	{
		return true;
	}
	else
	{
		if (left.ip == right.ip)
		{
            return left.port < right.port;
		}
		else
		{
			return false;
		}
	}
}

bool operator > (const LBFRLevelTuple& left, const LBFRLevelTuple& right)
{
	if (left.ip > right.ip)
	{
		return true;
	}
	else
	{
		if (left.ip == right.ip)
		{
            return left.port > right.port;
		}
		else
		{
			return false;
		}
	}
}


LBFastRejectLevelTableMgr::LBFastRejectLevelTableMgr() : mem_size_(0), lb_fr_level_shm_(NULL)
{

}

LBFastRejectLevelTableMgr::~LBFastRejectLevelTableMgr()
{
}

int32_t LBFastRejectLevelTableMgr::Create(int32_t size)
{
    g_mgr_buffer = new char [size];
	lb_fr_level_shm_ = (LBFRLevelShm*) g_mgr_buffer;
	if (NULL == lb_fr_level_shm_)
	{
		return -1;
	}
	else
	{
        lb_fr_level_shm_->Reset();
        mem_size_ = size;
		return 0;
	}
}

int32_t LBFastRejectLevelTableMgr::Attach(int32_t size)
{
	lb_fr_level_shm_ = (LBFRLevelShm*) g_mgr_buffer;
	if (NULL == lb_fr_level_shm_)
	{
		return -1;
	}
	else
	{
		if (IsPassMagicTagCheck() )
		{
			mem_size_ = size;
			return 0;
		}
		else
		{
			lb_fr_level_shm_ = NULL;
			return -1;
		}
	}
}

int32_t LBFastRejectLevelTableMgr::Detach(bool is_del)
{
	if (is_del)
	{
        delete [] g_mgr_buffer;
        g_mgr_buffer = NULL;
        lb_fr_level_shm_ = NULL;
        return 0;
	}
	else
    {
        lb_fr_level_shm_ = NULL;
        return 0;
    }
}

LBFRLevelTable* LBFastRejectLevelTableMgr::GetReadFRLevelTable()
{
	if (lb_fr_level_shm_ != NULL
			&& (0 == lb_fr_level_shm_->fr_level_table_flag || 1 == lb_fr_level_shm_->fr_level_table_flag))
	{
		if (0 == lb_fr_level_shm_->fr_level_table_flag)
		{
			return &(lb_fr_level_shm_->fr_level_table_array[0]);
		}
		else
		{
			return &(lb_fr_level_shm_->fr_level_table_array[1]);
		}
	}
	else
	{
		return NULL;
	}
}

LBFRLevelTable* LBFastRejectLevelTableMgr::GetWriteFRLevelTable()
{
	if (lb_fr_level_shm_ != NULL
			&& (0 == lb_fr_level_shm_->fr_level_table_flag || 1 == lb_fr_level_shm_->fr_level_table_flag))
	{
		if (0 == lb_fr_level_shm_->fr_level_table_flag)
		{
			return &(lb_fr_level_shm_->fr_level_table_array[1]);
		}
		else
		{
			return &(lb_fr_level_shm_->fr_level_table_array[0]);
		}
	}
	else
	{
		return NULL;
	}
}

void LBFastRejectLevelTableMgr::SwapFRLevelTableFlag()
{
	if (lb_fr_level_shm_ != NULL
			&& (0 == lb_fr_level_shm_->fr_level_table_flag || 1 == lb_fr_level_shm_->fr_level_table_flag))
	{
		if (0 == lb_fr_level_shm_->fr_level_table_flag)
		{
			lb_fr_level_shm_->fr_level_table_flag = 1;
		}
		else
		{
			lb_fr_level_shm_->fr_level_table_flag = 0;
		}
	}
}

LBFRLevelShm* LBFastRejectLevelTableMgr::GetLBFRLevelShm()
{
	return lb_fr_level_shm_;
}

void LBFastRejectLevelTableMgr::SetMagicTag()
{
	if (lb_fr_level_shm_ != NULL)
	{
		lb_fr_level_shm_->magic_tag = 0x6789555;
	}
}

bool LBFastRejectLevelTableMgr::IsPassMagicTagCheck()
{
	if (lb_fr_level_shm_ != NULL && 0x6789555 == lb_fr_level_shm_->magic_tag
			&& (0 == lb_fr_level_shm_->fr_level_table_flag || 1 == lb_fr_level_shm_->fr_level_table_flag))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LBFastRejectLevelTableMgr::IsFRLevelTableSwitchFlagValid()
{
	if (lb_fr_level_shm_ != NULL && (lb_fr_level_shm_->fr_level_table_flag != 0 && lb_fr_level_shm_->fr_level_table_flag != 1))
	{
		lb_fr_level_shm_->Reset();
		return false;
	}
	else
	{
		return true;
	}
}

LBFRLevelTuple* LBFastRejectLevelTableMgr::GetFRLevelTuple(const uint32_t& ip, const uint32_t& port)
{
	LBFRLevelTable* read_table = GetReadFRLevelTable();
	if (read_table != NULL
			&& read_table->fr_level_tuple_num > 0 && read_table->fr_level_tuple_num <= MAX_FRLEVEL_TUPLE_NUM)
	{
		int32_t iIndex = BSearch(read_table, 0, read_table->fr_level_tuple_num - 1, ip, port);
		if (iIndex >= 0 && iIndex < MAX_FRLEVEL_TUPLE_NUM)
		{
			return &(read_table->fr_level_tuple_array[iIndex]);
		}
	}

	return NULL;

}

int32_t LBFastRejectLevelTableMgr::BSearch(LBFRLevelTable fr_level_table[], int32_t low, int32_t high,
				uint32_t ip, uint32_t port)
{
	// low, high is ptStrategyTable array's item index, array max size is MAX_FRLEVEL_TUPLE_NUM
	if (low < 0 || high < 0 || low > high || high >= MAX_FRLEVEL_TUPLE_NUM)
	{
		return -1;
	}

	int32_t iMid = 0;
	curr_tuple_mock_.SetTupleIndentity(ip, port);
	while (low <= high)
	{
		iMid = low + ((high - low) >> 1);
		if (fr_level_table->fr_level_tuple_array[iMid] > curr_tuple_mock_)
		{
			high = iMid - 1;
		}
		else
		{
			if (fr_level_table->fr_level_tuple_array[iMid] < curr_tuple_mock_)
			{
				low = iMid + 1;
			}
			else
			{
				return iMid;
			}
		}
	}

	return -1;
}

int32_t LBFastRejectLevelTableMgr::GetFRLevelInfo(const uint32_t& ip, const uint32_t& port, 
        int32_t& business_priority, int32_t& user_priority)
{
	LBFRLevelTuple* curr_fr_level_tuple = GetFRLevelTuple(ip, port);
	if (curr_fr_level_tuple != NULL)
	{
		business_priority = curr_fr_level_tuple->business_priority;
		user_priority = curr_fr_level_tuple->user_priority;
		return 0;
	}
	else
	{
		return -1;
	}
}

}





