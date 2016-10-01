/*
	Copyright (c) 2016 Tencent.  See the AUTHORS file for names 
	of contributors.
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
	
*/

#pragma once

#ifdef   __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

namespace phxrpc {

//old shm define begin
#define MAX_ATTR_ID 65535
#define KEY_PER_ID 64
#define MAX_KEY_ID 63
#define ERROR_KEY_ID 63 
#define KEY_VAL_LEN (MAX_ATTR_ID*KEY_PER_ID)

#define OSS_ATTT_SHM_KEY 0x212346

typedef struct oss_attr_key_val_s {
    uint32_t val[2];
}__attribute__ ((__packed__)) oss_attr_kv_t;

typedef struct aid_s {
    uint32_t res[2];
    oss_attr_kv_t kv[KEY_PER_ID];
}__attribute__ ((__packed__)) oss_attr_aid_t;

typedef struct oss_attr_s {

    uint32_t magic1;
    uint32_t res1[100];
    uint32_t mtime;                     //修改时间
    uint16_t vindex;                	//用来标识使用val[0] 还是 val[1] 
                                        //取值只能是0或1
    uint16_t reserve;

    oss_attr_aid_t aid[MAX_ATTR_ID];
    uint32_t res2[100];
    uint32_t magic2;

}__attribute__ ((__packed__)) oss_attr_t;

typedef struct tag_protected_addr_holder {
    char * protected_addr;
    oss_attr_t * oss_addr;
}__attribute__ ((__packed__)) protected_addr_holder_t;

//old shm define end

//new shm define begin ----by mariohuang/2013121715

#define NEW_IDKEY_ARRAY_SIZE 2
#define NEW_IDKEY_KEY_SIZE 128
#define NEW_IDKEY_ID_SIZE (128*1024)
#define NEW_IDKEY_SIZE (NEW_IDKEY_ID_SIZE*NEW_IDKEY_KEY_SIZE)
#define NEW_IDKEY_SHM_KEY 0xab9145cd
#define OSS_ATTR_MAGIC1	0x123abced
#define OSS_ATTR_MAGIC2	0x123abced

typedef struct tag_oss_value_struct {
    uint32_t val;
    uint32_t res;
}__attribute__ ((__packed__)) oss_value_struct_t;

typedef struct tag_oss_key_struct {
    oss_value_struct_t value_array[NEW_IDKEY_KEY_SIZE];
}__attribute__ ((__packed__)) oss_key_struct_t;

typedef struct tag_oss_id_struct {
    oss_key_struct_t key_array[NEW_IDKEY_ID_SIZE];
}__attribute__ ((__packed__)) oss_id_struct_t;

typedef struct tag_oss_attr_struct {
    uint32_t magic1;
    int32_t idc_room_id;
    uint32_t mem_timestamp;
    int32_t mem_pct;
    uint32_t graph_id;
    uint32_t graph_node_id;
    uint32_t cpu_timestamp;
    int32_t cpu_pct;
    uint32_t inner_traffic_timestamp;
    int32_t in_traffic_pct;
    int32_t out_traffic_pct;
    uint32_t eth_speed_in_Mbps;
    uint32_t res1[89];
    uint32_t mtime;                     //修改时间
    uint16_t vindex;                	//用来标识使用val[0] 还是 val[1] 
                                        //取值只能是0或1
    uint16_t reserve;

    oss_id_struct_t id_array[NEW_IDKEY_ARRAY_SIZE];
    uint32_t res2[100];
    uint32_t magic2;
}__attribute__ ((__packed__)) new_oss_attr_struct_t;

typedef struct tag_new_protected_addr_holder {
    char * head_protected_addr;
    new_oss_attr_struct_t * oss_addr;
    char * tail_protected_addr;
}__attribute__ ((__packed__)) new_protected_addr_holder_t;

typedef enum {
    op_inc = 0,
    op_set = 1,
    op_setmax = 2,
    op_setmin = 3,
    op_max
} oss_attr_op_type;
//new shm define end

typedef struct tag_oss_attr_id_key_value {
    uint32_t id;
    uint32_t key;
    uint32_t value1;
    uint32_t value2;
}__attribute__ ((__packed__)) oss_attr_id_key_value;

}
;
#ifdef   __cplusplus 
}
#endif
