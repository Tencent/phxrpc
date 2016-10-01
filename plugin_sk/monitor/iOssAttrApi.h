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

/* @OssAttrInc0: id[0].val += val
 * 
 * @id: Attr ID
 * @val: val
 * 
 */
int OssAttrInc0(uint32_t id, uint32_t val);

/* @OssAttrInc id[key].val += val
 *
 * @id: Attr ID
 * @key: key
 * @val: val
 *
 */
int OssAttrInc(uint32_t id, uint32_t key, uint32_t val);

/* @OssAttrSet0: id[0].val = val
 * 
 * @id: Attr ID
 * @val: val
 *  
 */
int OssAttrSet0(uint32_t id, uint32_t val);

/* @OssAttrSet id[key].val = val
 *
 * @id: Attr ID
 * @key: key
 * @val: val
 * 
 */
int OssAttrSet(uint32_t id, uint32_t key, uint32_t val);

/* @OssAttrSetMax0: id[0].val = val
 * 
 * @id: Attr ID
 * @val: val
 *  
 */
int OssAttrSetMax0(uint32_t id, uint32_t val);

/* @OssAttrSetMax id[key].val = val
 *
 * @id: Attr ID
 * @key: key
 * @val: val
 * 
 */
int OssAttrSetMax(uint32_t id, uint32_t key, uint32_t val);

/* @OssAttrSetMin0: id[0].val = val
 * 
 * @id: Attr ID
 * @val: val
 *  
 */
int OssAttrSetMin0(uint32_t id, uint32_t val);

/* @OssAttrSetMin id[key].val = val
 *
 * @id: Attr ID
 * @key: key
 * @val: val
 * 
 */
int OssAttrSetMin(uint32_t id, uint32_t key, uint32_t val);

/*for test or debug*/
void Show_All_Attr();

/*for ossattragent */
void* GetShm(int key, size_t size, int flag);
int GetShm2(void **pshm, int shmid, size_t size, int flag);

int GetIDCRoomID();

int GetMemUsedInfo(unsigned int * timestamp, int * pct);

int GetCpuUsedInfo(unsigned int * timestamp, int * pct);

int GetTrafficUsedInfo(unsigned int * timestamp, int * in_traffic_pct, int * out_traffic_pct);

int GetAllAttrID(void ** vec);

int GenGraphID(uint32_t & llGraphID);

int GenGraphNodeID(uint32_t & llGraphNodeID);

int OssAttrIncGet(uint32_t id, uint32_t key, uint32_t & val);

int GetEthSpeedInMbps(unsigned int * eth_speed_in_Mbps);

#ifdef   __cplusplus 
}
#endif 
