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

#include <string>
#include <sstream>
#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <error.h>


#include "lbfrdefine.h"

using std::string;
using std::stringstream;
using std::endl;

#pragma pack (push, 1)

#define MAX_FRLEVEL_TUPLE_NUM 2048
#define MAX_BINARY_FIND_LOOP_NUM 11
#define LB_FRLEVEL_SHM_KEY 0x151e773
#define LB_FRLEVEL_MAGIC_TAG 0x6789555

namespace phxrpc {

typedef struct tagLBFRLevelTuple
{
	uint32_t ip;			
	uint32_t port;			
	int32_t business_priority;	
	int32_t user_priority;	

	tagLBFRLevelTuple()
	{
		ip = 0;
		port = 0;
		business_priority = 0;
		user_priority = 0;
	}

	void SetTupleIndentity(const uint32_t& ip, const uint32_t& port)
	{
		this->ip = ip;
		this->port = port;
	}

	void SetFRLevelTuple(const uint32_t& ip, const uint32_t& port, 
			const int32_t& business_priority, const int32_t& user_priority)
	{
		this->ip = ip;
		this->port = port;
		this->business_priority = business_priority;
		this->user_priority = user_priority;
	}

} LBFRLevelTuple;

typedef struct tagLBFRLevelTable
{
	volatile uint32_t fr_level_tuple_num;
	LBFRLevelTuple fr_level_tuple_array[MAX_FRLEVEL_TUPLE_NUM];

	tagLBFRLevelTable()
	{
		fr_level_tuple_num = 0;
	}
} LBFRLevelTable;


typedef struct tagLBFRLevelShm
{
	uint32_t magic_tag;
	volatile uint32_t fr_level_table_flag;

	LBFRLevelTable fr_level_table_array[2];	

	tagLBFRLevelShm()
	{
		magic_tag = LB_FRLEVEL_MAGIC_TAG;
		fr_level_table_flag = 0;
	}

	void Reset()
	{
		magic_tag = LB_FRLEVEL_MAGIC_TAG;
		fr_level_table_flag = 0;

		memset(fr_level_table_array, 0, sizeof(LBFRLevelTable));
	}
} LBFRLevelShm;

#pragma pack(pop)

#define LB_FRLEVEL_SHM_SIZE sizeof(LBFRLevelShm)

/////////////////////////////////////////////////////////////////////////////////////////////////

/// define LBFRLevelTuple compare operator for strategy tuple find operation
bool operator == (const LBFRLevelTuple& left, const LBFRLevelTuple& right);
bool operator < (const LBFRLevelTuple& left, const LBFRLevelTuple& right);
bool operator > (const LBFRLevelTuple& left, const LBFRLevelTuple& right);

/////////////////////////////////////////////////////////////////////////////////////////////////

class LBFastRejectLevelTableMgr
{
public:
	LBFastRejectLevelTableMgr();
	virtual ~LBFastRejectLevelTableMgr();

	int32_t Create(int32_t size = LB_FRLEVEL_SHM_SIZE);
	int32_t Attach(int32_t size = LB_FRLEVEL_SHM_SIZE);
	int32_t Detach(bool is_del = false);

	int32_t GetFRLevelInfo(const uint32_t& ip, const uint32_t& port, 
			int32_t& business_priority, int32_t& user_priority);

	LBFRLevelTable* GetReadFRLevelTable();
	LBFRLevelTable* GetWriteFRLevelTable();

	void SwapFRLevelTableFlag();

	LBFRLevelShm* GetLBFRLevelShm();

private:
	void SetMagicTag();
	bool IsPassMagicTagCheck();
	bool IsFRLevelTableSwitchFlagValid();

	LBFRLevelTuple* GetFRLevelTuple(const uint32_t& ip, const uint32_t& port);

	int32_t BSearch(LBFRLevelTable fr_level_table[], int32_t low, int32_t high,
					uint32_t ip, uint32_t port);

private:
	LBFRLevelTuple curr_tuple_mock_;

	uint32_t mem_size_;
	LBFRLevelShm* lb_fr_level_shm_;
};

}

