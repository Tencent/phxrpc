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

#include "rwlock_mgr.h"

namespace phxrpc
{

RWLockMgr :: RWLockMgr()
{
	lock_ = new pthread_rwlock_t;
	pthread_rwlock_init(lock_, NULL);
}

RWLockMgr :: ~RWLockMgr()
{
	if (lock_ != NULL)
	{
		pthread_rwlock_destroy(lock_);
		delete lock_;
	}
}

void RWLockMgr :: ReadLock()
{
	pthread_rwlock_rdlock(lock_);
}

void RWLockMgr :: WriteLock()
{
	pthread_rwlock_wrlock(lock_);
}

int RWLockMgr :: TryReadLock()
{
    return pthread_rwlock_tryrdlock(lock_);
}

int RWLockMgr :: TryWriteLock()
{
    return pthread_rwlock_trywrlock(lock_);
}

void RWLockMgr :: ReadUnLock()
{
	pthread_rwlock_unlock(lock_);
}

void RWLockMgr :: WriteUnLock()
{
	pthread_rwlock_unlock(lock_);
}

RWLockGuard :: RWLockGuard(RWLockMgr * lock, int type)
{
	type_ = type;
	lock_ = lock;
	is_unlock_ = true;
	
	if (type_ == RWLockWriteLock)
	{
		lock_->WriteLock();
		is_unlock_ = false;
	}
	else if (type_ == RWLockReadLock)
	{
		lock_->ReadLock();
		is_unlock_ = false;
	}
}

RWLockGuard :: ~RWLockGuard()
{
	if (type_ == RWLockWriteLock)
	{
		if (!is_unlock_)
		{
			lock_->WriteUnLock();
			is_unlock_ = true;
		}
	}
	else if (type_ == RWLockReadLock)
	{
		if (!is_unlock_)
		{
			lock_->ReadUnLock();
			is_unlock_ = true;
		}
	}
}

void RWLockGuard :: UnLock()
{
	if (type_ == RWLockWriteLock)
	{
		if (!is_unlock_)
		{
			lock_->WriteUnLock();
			is_unlock_ = true;
		}
	}
	else if (type_ == RWLockReadLock)
	{
		if (!is_unlock_)
		{
			lock_->ReadUnLock();
			is_unlock_ = true;
		}
	}
}

}

