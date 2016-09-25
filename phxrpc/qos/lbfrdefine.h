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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <set>

using std::string;
using std::stringstream;
using std::map;
using std::set;

namespace phxrpc {

typedef struct tagFRRouteIPInfo
{
	uint32_t ip;			
	uint32_t port;			
	tagFRRouteIPInfo()
	{
		ip = 0;
		port = 0;
	}

	tagFRRouteIPInfo(uint32_t ip, uint32_t port)
	{
		this->ip = ip;
		this->port = port;
	}

	void SetFRRouteIPInfo(const uint32_t& ip, const uint32_t& port)
	{
		this->ip = ip;
		this->port = port;
	}
} FRRouteIPInfo;

class FRRouteIPInfoLess
{
public:
	bool operator()(const FRRouteIPInfo& left,const FRRouteIPInfo& right) const
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
};

typedef struct tagFRLevelInfo
{
	int32_t business_priority;
	int32_t user_priority;
	bool is_period_report_update;

	tagFRLevelInfo()
	{
		business_priority = 0;
		user_priority = 0;
		is_period_report_update = true;
	}

	tagFRLevelInfo(int32_t business_priority, int32_t user_priority, bool update = true)
	{
		this->business_priority = business_priority;
		this->user_priority = user_priority;

		this->is_period_report_update = update;
	}

	tagFRLevelInfo(const tagFRLevelInfo& tFRLevelInfo, bool update = true)
	{
		this->business_priority = tFRLevelInfo.business_priority;
		this->user_priority = tFRLevelInfo.user_priority;
		this->is_period_report_update = update;
	}

	void SetFRLevelInfo(int32_t business_priority, int32_t user_priority, bool update = true)
	{
		this->business_priority = business_priority;
		this->user_priority = user_priority;
		this->is_period_report_update = update;
	}

	void SetFRLevelInfo(const tagFRLevelInfo& tFRLevelInfo, bool update = true)
	{
		this->business_priority = tFRLevelInfo.business_priority;
		this->user_priority = tFRLevelInfo.user_priority;
		this->is_period_report_update = update;
	}

	bool IsNoramlFRLevelInfo()
	{
		if (0 == business_priority && 0 == user_priority)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
} FRLevelInfo;

bool operator > (const FRLevelInfo& left, const FRLevelInfo& right);

typedef map<FRRouteIPInfo, FRLevelInfo*, FRRouteIPInfoLess> FRRouteIPLevelInfoMap;


}

