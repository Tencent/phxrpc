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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <vector>

namespace phxrpc {

#ifdef __APPLE__
	#define MAP_ANONYMOUS MAP_ANON
#endif

/*
	slot: pos
	item: hash,key
	optional:raw_value
*/

inline void* malloc_mmap( int size )
{
	return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0);
}
template < class key_type,class key_mng_type,int use_mmap=0 >
class buffer_hash
{
	// n * idx 
	// + m * { next,hash } 
public:
	typedef key_mng_type mng_type;
	typedef uint32_t slot_pos_t;

	struct header_t
	{
		int slot_cnt;
		int key_cnt;

		int slot_offset;
		int key_offset;

	}__attribute__((packed));


	char *m_buf;
	int m_buflen;

	header_t *m_head;
	slot_pos_t *m_slot;

	int m_key_total;
	bool m_ref;
	
	unsigned long long m_search_cnt;
public:

	void clear()
	{
		memset( m_slot,0,sizeof(slot_pos_t) * m_head->slot_cnt );
		m_head->key_cnt = 1;
		m_search_cnt = 0;
	}
	static void *alloc_buffer( int size )
	{
		if( use_mmap )
		{
			return malloc_mmap( size );
		}
		return malloc( size );
	}
	static void free_buffer( void *ptr,int size )
	{
		if( use_mmap )
		{
			munmap( ptr,size );
			return ;
		}
		free( ptr );
	}
	buffer_hash( int slot_cnt,int key_total )
			:m_buf(NULL),m_buflen(0),
			 m_head(NULL),m_slot(NULL),
			 m_key_total(key_total),m_ref(false),m_search_cnt(0)
	{
		m_buflen = sizeof(header_t) 
					+ slot_cnt * sizeof(slot_pos_t) 
					+ m_key_total * sizeof(key_type);

        //printf("slot_cnt %d sizeof(slot_pos_t) %zu m_key_total %d sizeof(key_type) %zu buf_len %d\n", 
                //slot_cnt, sizeof(slot_pos_t), m_key_total, sizeof(key_type), m_buflen);

		m_buf = (char*)alloc_buffer( m_buflen );

		m_head = (header_t*)m_buf;
		memset( m_head,0,sizeof(*m_head) );
		
		m_head->slot_offset = sizeof(header_t);
		m_head->slot_cnt = slot_cnt;

		m_slot = (slot_pos_t *)( m_buf + sizeof(header_t) );
		memset( m_slot,0,sizeof(slot_pos_t) * slot_cnt );

		m_head->key_offset = m_head->slot_offset + key_total * sizeof(slot_pos_t);
		m_head->key_cnt = 1;


	}

	buffer_hash( char *ptr,int len,bool copy ) //from buffer
			:m_buf(0),m_buflen(len),
			 m_head(NULL),m_slot(NULL),
			 m_key_total(0),m_ref(false),m_search_cnt(0)

	{
		if( copy )
		{
			m_buf = (char*)alloc_buffer( len );
			memcpy( m_buf,ptr,len );
		}
		else
		{
			m_buf = ptr;
			m_ref = true;
		}

		m_head = (header_t*)m_buf;
		m_slot = (slot_pos_t *)( m_buf + sizeof(header_t) );

		m_key_total = m_head->key_cnt;
	}
	~buffer_hash()
	{
		if( !m_ref )
		{
			free_buffer( m_buf,m_buflen );
		}
		m_buf = 0;
	}
	char *get_buf_ptr(int *plen )
	{
		char *ret = m_buf;
		*plen = sizeof(header_t) 
				+ m_head->slot_cnt * sizeof(slot_pos_t) 
				+ m_head->key_cnt * sizeof(key_type);
		return ret;
	}
	unsigned long long get_search_cnt()
	{
		return m_search_cnt;
	}
	inline header_t *get_head()
	{
		return m_head;
	}
	inline slot_pos_t *get_slot()
	{
		return (slot_pos_t*)( m_buf + m_head->slot_offset );
	}
	inline key_type *get_keys()
	{
		return (key_type*)( m_buf + m_head->key_offset );
	}
	inline int get_key_cnt()
	{
		return m_head->key_cnt ;
	}
	inline key_type *get_key( int i )
	{
		if( i < 0 || i >= m_head->key_cnt )
		{
			return 0;
		}
		return (key_type*)( m_buf + m_head->key_offset ) + i ;
	}
	inline int get_key_pos( key_type *k )
	{
		return k - get_keys();
	}
	inline key_type *find( const typename key_type::key_t newk,key_mng_type *mng )
	{
		typename key_type::hash_t hash = mng->hash_func( newk );
		return find( hash,newk,mng );
	}
	inline key_type *find( const typename key_type::hash_t hash,
						   const typename key_type::key_t newk,key_mng_type *mng )
	{
		int idx = mng->mod( hash,m_head->slot_cnt );
		int pos = get_slot()[ idx ];

		int cnt = 0;
		for(;;)
		{
			if ( cnt++ > m_key_total ) return NULL;
			++m_search_cnt;
			if( !pos ) return NULL;
			key_type *k = get_keys() + pos;
			if( hash == k->hash && mng->is_equal( k,newk ) )
			{
				return k;
			}
			pos = k->next;
		}
		return 0;

	}
	inline int at( const typename key_type::hash_t hash,
						const typename key_type::key_t &key,
						key_mng_type *mng,
		   				key_type **ret	)
	{
		*ret = 0;
		key_type *k = find( hash,key,mng );
		if( k )
		{
			*ret = k;
			return 1;//exists
		}
		if( m_head->key_cnt >= m_key_total ) 		
		{
			return 2;//full

		}

		k = get_keys() + m_head->key_cnt;
		int pos = m_head->key_cnt;

		int r = mng->set( k,hash,key );
		if( r ) return -__LINE__;

		int idx = mng->mod( hash,m_head->slot_cnt );

		k->next = m_slot[ idx ];
		m_slot[ idx ] = pos;
		m_head->key_cnt++;

		*ret = k;
		return 0;
	}
	inline int at( const typename key_type::key_t &k,key_mng_type *mng,key_type **ret )
	{
		const typename key_type::hash_t hash = mng->hash_func( k );
		return at( hash,k,mng,ret );
	}
};
struct dict_key_t 
{
	uint16_t next;
	uint16_t pos_link;
	uint32_t hash;

	int data;

	typedef const char * key_t;
	typedef unsigned int hash_t;

}__attribute__((packed));

class dict_key_mng
{
public:
	struct header_t
	{
		int len;
	}__attribute__((packed));

private:
	char *m_buf;
	int m_buflen;

	header_t *m_head;
	bool m_ref;

public:

	dict_key_mng( int buflen )
			:m_buf(0),m_buflen(0),m_head( 0 ),m_ref( false )
	{
		m_buflen = buflen;
		m_buf = (char*)malloc( m_buflen + sizeof(header_t) );
		m_head = (header_t*)m_buf;
		m_head->len = 0;
		m_buf += sizeof(header_t);
	}
	dict_key_mng( char *ptr,int len,bool copy )
			:m_buf(0),m_buflen(len),m_head( 0 ),m_ref( false )
	{
		if( copy )
		{
			m_buf = (char*)malloc( len );
			memcpy( m_buf,ptr,len );
		}
		else
		{
			m_ref = true;
			m_buf = ptr;
		}
		m_head = (header_t*)m_buf;
		if ( !copy ) m_head->len = 0;
		m_buf += sizeof(header_t);
	}
	~dict_key_mng()
	{
		if( !m_ref )
		{
			free( m_buf - sizeof(header_t) );
		}
		m_buf = 0;
		m_buflen = 0;
	}
	void clear()
	{
		m_head->len = 0;
	}
	char *get_buf_ptr( int *plen )
	{
		char *ret = m_buf - sizeof(header_t);
		*plen = sizeof(header_t) + m_head->len;
		return ret;
	}
	inline int mod( unsigned int hash,int x )
	{
		return hash % x;
	}
	inline const char *get_str( dict_key_t *k )
	{
		if( k->data >= m_head->len ) return 0;
		return m_buf + k->data ;
	}
	
	static inline unsigned int hash_func( const char *str )  
	{  
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..  
		unsigned int hash = 0;  
		while( *str )
		{  
			hash = (hash * seed) + (*str++);
		}  
		return hash;  
	} 
	inline bool is_equal( dict_key_t *k1,dict_key_t::key_t k )
	{
		return !strcmp( m_buf + k1->data,k);
	}
	inline int set( dict_key_t *k1,const dict_key_t::hash_t &hash,const dict_key_t::key_t &k )
	{
		int len = strlen( k ) + 1;
		if( len > 4096 ) return __LINE__;
		if( m_buflen - m_head->len < len + 2 )
		{
			return __LINE__;
		}

		//1.len
		*(uint16_t*)( m_buf+ m_head->len ) = len;
		m_head->len += 2;

		//2.str
		memcpy( m_buf + m_head->len,k,len );
		k1->data = m_head->len;
		m_head->len += len;
		
		//-------
		k1->hash = hash;
		return 0;
	}
};
class dict
{
	dict_key_mng m_mng;
	buffer_hash<dict_key_t,dict_key_mng> m_hash;
public:
	typedef buffer_hash<dict_key_t,dict_key_mng> hash_type;

	dict( int string_buf_len,int key_cnt )
			:m_mng( string_buf_len ),
			 m_hash( key_cnt,key_cnt )
	{}
	dict( char *hash_buf,int hash_len,char *mng_buf,int mng_len,bool copy )
		:m_mng(mng_buf,mng_len,copy),
		 m_hash( hash_buf,hash_len,copy )
	{}
	~dict(){}
	hash_type &get_hash()
	{
		return m_hash;
	}
	dict_key_mng &get_mng()
	{
		return m_mng;
	}
	inline int get_key_pos( dict_key_t *k )
	{
		return m_hash.get_key_pos( k );
	}

	inline int get_str_pos( const char *s )
	{
		dict_key_t *x = 0;
		m_hash.at( s,&m_mng,&x );
		if( x )
		{
			return get_key_pos ( x );
		}
		return -1;
	}
	inline int get_key_cnt()
	{
		return m_hash.get_key_cnt();
	}
	inline dict_key_t *get_key( int i )
	{
		return m_hash.get_key( i );
	}
	inline int get_str_cnt()
	{
		return m_hash.get_key_cnt();
	}
	inline const char *get_str( int pos )
	{
		dict_key_t *x = m_hash.get_key( pos );
		if( x ) return m_mng.get_str( x );
		return 0;
	}
	inline char *get_mng_buf_ptr( int *plen )
	{
		return m_mng.get_buf_ptr( plen );
	}
	inline char *get_hash_buf_ptr( int *plen )
	{
		return m_hash.get_buf_ptr( plen );
	}
	inline dict_key_t *at( const char *s )
	{
		dict_key_t *ret = 0;
		m_hash.at( s,&m_mng,&ret );
		
		return ret;
	}
	inline dict_key_t *at( uint32_t hash,const char *s )
	{
		dict_key_t *ret = 0;
		m_hash.at( hash,s,&m_mng,&ret );
		return ret;
	}

	inline dict_key_t *find( const char *s )
	{
		return m_hash.find( s,&m_mng );
	}

};

}
