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
#include <pthread.h>

namespace phxrpc
{

enum enRWLockType
{
	RWLockWriteLock = 1,
	RWLockReadLock = 2
};

class RWLockMgr
{
public:
	
    RWLockMgr();

	~RWLockMgr();
	
	void ReadLock();
	void WriteLock();
	int TryReadLock();
	int TryWriteLock();
	void ReadUnLock();
	void WriteUnLock();

private:
	pthread_rwlock_t * lock_;
};


class RWLockGuard
{
public:
	RWLockGuard(RWLockMgr * lock, int type);
	~RWLockGuard();

	void UnLock();
private:

	RWLockMgr * lock_;
	int type_;
	bool is_unlock_;
};

}
