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

#include "buffer_hash_mmap.h"

namespace phxrpc {

struct lb_key_hash_t
{
	unsigned long long hash:32;
	unsigned long long remote_ip:32;

	bool operator==( const lb_key_hash_t &other ) const
	{
		return ( *(unsigned long long *)this 
			   == *(unsigned long long *)&other);
	}

}__attribute__((packed));

struct lb_key_t
{
	uint32_t remote_ip;			
	uint16_t remote_port;
}__attribute__((packed));

struct lb_key_item_t 
{
	uint16_t next;
	lb_key_hash_t hash;
	lb_key_t key;

	int32_t business_priority;
	int32_t user_priority;

	typedef lb_key_t key_t;	
	typedef lb_key_hash_t hash_t;	

}__attribute__((packed));

int combine_lb_item( lb_key_item_t *dst,lb_key_item_t *src );

class lb_key_mng
{
public:
	typedef buffer_hash< lb_key_item_t,lb_key_mng,1> hash_type;
private:
	hash_type m_hash;
public:
	lb_key_mng( int slot_cnt,int key_cnt )
			:m_hash( slot_cnt,key_cnt)
	{}
	lb_key_mng( char *ptr,int len,bool copy )
			:m_hash( ptr,len,copy )
	{}
	~lb_key_mng() {}
public:
	hash_type &get_hash()
	{
		return m_hash;
	}
	inline void clear()
	{
		m_hash.clear();
	}
	inline int get_key_cnt()
	{
		return m_hash.get_key_cnt();
	}
	inline lb_key_item_t *get_key( int i )
	{
		return m_hash.get_key( i );
	}
	inline lb_key_item_t *at( const lb_key_t &k )
	{
		lb_key_item_t *ret = 0;
		m_hash.at( k,this,&ret );
		return ret;
	}
	inline lb_key_item_t *at( const lb_key_hash_t &h,const lb_key_t &k )
	{
		lb_key_item_t *ret = 0;
		m_hash.at( h,k,this,&ret );
		return ret;
	}
	inline int mod( const lb_key_hash_t hash,int x )
	{
        unsigned long long * tmp_hash = (unsigned long long*)&hash;
		return (*tmp_hash) % x;
	}
	static inline const lb_key_hash_t hash_func( const lb_key_t &k )  
	{  
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..  
		lb_key_hash_t hash = { 0 };
		char *p = (char*)&k + 4;//name_idx
		int len = sizeof(k) - 4;
		for(int i=0;i<len;i++)
		{  
			hash.hash = (hash.hash * seed) + p[i];
		}  
		hash.remote_ip = k.remote_ip;
		return hash;  
	} 
	inline bool is_equal( const lb_key_item_t *k1,const lb_key_t &k )
	{
		return !memcmp( &k1->key,&k,sizeof(k) );
	}
	inline int set( lb_key_item_t *k1,const lb_key_hash_t &hash,const lb_key_t &k )
	{
		memset( k1,0,sizeof(*k1) );
		memcpy( &k1->key,&k,sizeof(k) );
		memcpy( &k1->hash,&hash,sizeof(hash) );
		return 0;
	}
	static void new_array( std::vector<lb_key_mng*> &v,int array_size,
							int slot_cnt,int key_total )
	{
		v.clear();

		char *p = (char*)hash_type::alloc_buffer( sizeof(lb_key_mng) * array_size );
		for(int i=0;i<array_size;i++)
		{
			v.push_back( new(p) lb_key_mng( slot_cnt,key_total ) );
			p += sizeof( lb_key_mng );
		}
		return ;
	}
};

//-----------------------------------------------------
inline int combine( lb_key_mng *dst,lb_key_mng *src )
{
	lb_key_item_t *from = 0;
	lb_key_item_t *to = 0;

	for(int i=1;i<src->get_key_cnt();i++)
	{
		from = src->get_key(i);
		to = dst->at( from->key );
		if( to )
		{
			combine_lb_item( to,from );
		}
	}
	return 0;
}

inline int combine_lb_item( lb_key_item_t *dst,lb_key_item_t *src )
{
	if (dst->business_priority > src->business_priority) {
		dst->business_priority = src->business_priority;
		dst->user_priority = src->user_priority;
	} else if (dst->business_priority == src->business_priority
			&& dst->user_priority > src->user_priority) {
		dst->user_priority = src->user_priority;
	}
	return 0;
}

}



