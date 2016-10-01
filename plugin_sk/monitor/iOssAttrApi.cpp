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

#include <string>
#include <vector>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "iOssAttrDefine.h"
#include "iOssAttrApi.h"

#define LOGERR(fmt, args...)\
{\
}

using namespace phxrpc;

static protected_addr_holder_t *g_ptProtectedAddrHolder = NULL;
static new_protected_addr_holder_t * g_ptNewProtectedAddrHolder = NULL;

typedef std::vector<phxrpc::oss_attr_id_key_value> IDKeyValueVec;

int GenTraceID(uint32_t * pllWriteID, uint32_t & llDapperID);

void* GetShm(int key, size_t size, int flag) {
    int shmid = 0;
    void* sshm = NULL;
    //char errmsg[50] = {0};

    if ((shmid = shmget(key, size, flag)) < 0) {
        LOGERR("shmget %d %zu %s", key, size, strerror(errno));
        perror("shmget1");
        return NULL;
    }
    if ((sshm = shmat(shmid, (const void *) NULL, 0)) == (void *) -1) {
        LOGERR("shmat %s", strerror(errno));
        perror("shmat2");
        return NULL;
    }
    return sshm;
}

int GetShm2(void **pshm, int shmid, size_t size, int flag) {
    void* sshm = NULL;

    if (!(sshm = GetShm(shmid, size, flag & (~IPC_CREAT)))) {
        if (!(flag & IPC_CREAT))
            return -1;
        if (!(sshm = GetShm(shmid, size, flag))) {
            LOGERR("GetShm IPC_CREAT Error");
            usleep(10);
            if (!(sshm = GetShm(shmid, size, flag & (~IPC_CREAT))))
                return -1;
        } else
            memset(sshm, 0, size);
    }
    *pshm = sshm;
    return 0;
}

int InitNewOssAddr() {
    if (!g_ptNewProtectedAddrHolder) {
        int iPageSize = getpagesize();
        size_t szStructSize = sizeof(new_oss_attr_struct_t);
        int iAllocSize = iPageSize + ((szStructSize + iPageSize - 1) / iPageSize) * iPageSize + iPageSize;
        //size = head_protect_size + pagesize_aligned_struct_size + tail_protect_size;

        int shmid = shmget( NEW_IDKEY_SHM_KEY, szStructSize, 0666);
        if (shmid < 0) {
            shmid = shmget( NEW_IDKEY_SHM_KEY, szStructSize, 0666 | IPC_CREAT);
        }
        if (shmid < 0) {
            return -2;
        }

        g_ptNewProtectedAddrHolder = new new_protected_addr_holder_t();
        if (true) {
            g_ptNewProtectedAddrHolder->head_protected_addr = (char *) (valloc(iAllocSize));
            g_ptNewProtectedAddrHolder->tail_protected_addr = g_ptNewProtectedAddrHolder->head_protected_addr
                    + iAllocSize - iPageSize;
            if (mprotect((caddr_t) g_ptNewProtectedAddrHolder->head_protected_addr, iPageSize, PROT_NONE) < 0) {
                LOGERR("%s mprotect errno %d, errno str %s\n", __func__, errno, strerror(errno));
                return -4;
            }

            if (mprotect((caddr_t) g_ptNewProtectedAddrHolder->tail_protected_addr, iPageSize, PROT_NONE) < 0) {
                LOGERR("%s mprotect errno %d, errno str %s\n", __func__, errno, strerror(errno));
                return -5;
            }
            g_ptNewProtectedAddrHolder->oss_addr = (new_oss_attr_struct_t *) (shmat(
                    shmid, g_ptNewProtectedAddrHolder->head_protected_addr + iPageSize, 0 | SHM_REMAP));
            if (g_ptNewProtectedAddrHolder->oss_addr == (void *) -1) {
                LOGERR("%s shmat failed, errno %d str %s", __func__, errno, strerror( errno ));
                return -3;
            }
        } else {
            g_ptNewProtectedAddrHolder->oss_addr = (new_oss_attr_struct_t *) (shmat(shmid, NULL, 0));
            if (g_ptNewProtectedAddrHolder->oss_addr == (void *) -1) {
                LOGERR("%s shmat failed, errno %d str %s", __func__, errno, strerror( errno ));
                return -3;
            }
        }

        g_ptNewProtectedAddrHolder->oss_addr->magic1 = OSS_ATTR_MAGIC1;
        g_ptNewProtectedAddrHolder->oss_addr->magic2 = OSS_ATTR_MAGIC2;
    }

    if (g_ptNewProtectedAddrHolder == NULL || g_ptNewProtectedAddrHolder->oss_addr == NULL
            || g_ptNewProtectedAddrHolder->oss_addr == (void *) -1) {
        //之前的Init失败就失败了，不能释放，否则多线程下重复init会导致内存无限泄漏。
        return -5;
    }

    /*内存被写乱了*/
    if (g_ptNewProtectedAddrHolder->oss_addr->magic1 != OSS_ATTR_MAGIC1
            || g_ptNewProtectedAddrHolder->oss_addr->magic2 != OSS_ATTR_MAGIC2) {
        //这个日志中webmail打印得太多了。。。日志都打爆了。
        //LOGERR("magic1 :%x != OSS_ATTR_MAGIC1 || magic2 :%x != OSS_ATTR_MAGIC2 ");
        return -6;
    }
    return 0;

}

int InitOldOssAddr() {
    if (!g_ptProtectedAddrHolder) {
        int shmid = shmget( OSS_ATTT_SHM_KEY, sizeof(oss_attr_t), 0666);
        if (shmid < 0) {
            shmid = shmget( OSS_ATTT_SHM_KEY, sizeof(oss_attr_t), 0666 | IPC_CREAT);
        }
        if (shmid < 0) {
            return -2;
        }

        int iPageSize = getpagesize();
        g_ptProtectedAddrHolder = new protected_addr_holder_t();

        if (true) {
            g_ptProtectedAddrHolder->protected_addr = (char *) (valloc(iPageSize + sizeof(oss_attr_t)));
            if (mprotect((caddr_t) g_ptProtectedAddrHolder->protected_addr, iPageSize, PROT_NONE) < 0) {
                LOGERR("%s mprotect errno %d, errno str %s\n", __func__, errno, strerror(errno));
                return -4;
            }
            g_ptProtectedAddrHolder->oss_addr = (oss_attr_t *) (shmat(
                    shmid, g_ptProtectedAddrHolder->protected_addr + iPageSize, 0 | SHM_REMAP));
            if (g_ptProtectedAddrHolder->oss_addr == (void *) -1) {
                LOGERR("%s shmat failed, errno %d str %s", __func__, errno, strerror( errno ));
                return -3;
            }
        } else {
            g_ptProtectedAddrHolder->oss_addr = (oss_attr_t *) (shmat(shmid, NULL, 0 | SHM_REMAP));
            if (g_ptProtectedAddrHolder->oss_addr == (void *) -1) {
                LOGERR("%s shmat failed, errno %d str %s", __func__, errno, strerror( errno ));
                return -3;
            }
        }

        g_ptProtectedAddrHolder->oss_addr->magic1 = OSS_ATTR_MAGIC1;
        g_ptProtectedAddrHolder->oss_addr->magic2 = OSS_ATTR_MAGIC2;
    }

    if (g_ptProtectedAddrHolder == NULL || g_ptProtectedAddrHolder->oss_addr == NULL
            || g_ptProtectedAddrHolder->oss_addr == (void *) -1) {
        //之前的Init失败就失败了，不能释放，否则多线程下重复init会导致内存无限泄漏。
        return -5;
    }

    /*内存被写乱了*/
    if (g_ptProtectedAddrHolder->oss_addr->magic1 != OSS_ATTR_MAGIC1
            || g_ptProtectedAddrHolder->oss_addr->magic2 != OSS_ATTR_MAGIC2) {
        //这个日志中webmail打印得太多了。。。日志都打爆了。
        //LOGERR("magic1 :%x != OSS_ATTR_MAGIC1 || magic2 :%x != OSS_ATTR_MAGIC2 ");
        return -3;
    }
    return 0;
}

int OssAttrBase(uint32_t id, uint32_t key, uint32_t val, phxrpc::oss_attr_op_type type) {
    if (InitOldOssAddr() != 0) {
        return -1;
    }

    if (InitNewOssAddr() != 0) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -4;
    }

    if (id >= NEW_IDKEY_ID_SIZE || key >= NEW_IDKEY_KEY_SIZE) {
        if (id >= NEW_IDKEY_ID_SIZE) {
            OssAttrInc(23312, 1, 1);
        }
        if (key >= NEW_IDKEY_KEY_SIZE) {
            OssAttrInc(23312, 2, 1);
        }
        return -1;
    }

    //开搞
    //agent每次上报完后都会将mtime = 0
    if (g_ptNewProtectedAddrHolder->oss_addr->mtime == 0)
        g_ptNewProtectedAddrHolder->oss_addr->mtime = time(NULL);

    oss_value_struct_t * write_struct_ptr = &(g_ptNewProtectedAddrHolder->oss_addr->id_array[vindex].key_array[id]
            .value_array[key]);

    if (write_struct_ptr->res != 0 && write_struct_ptr->res != (unsigned int) (type + 1)) {
        LOGERR("id %d key %d use 2 different api: %d and %d, reject", id, key, write_struct_ptr->res, type + 1);
        return -2;
    }
    write_struct_ptr->res = type + 1;

    if (type == op_inc) {
        __sync_fetch_and_add(&(write_struct_ptr->val), val);
    } else if (type == op_set) {
        //set old
        if (id < MAX_ATTR_ID && key < KEY_PER_ID) {
            g_ptProtectedAddrHolder->oss_addr->aid[id].kv[key].val[vindex] = val;
        }
        //set new 
        write_struct_ptr->val = val;
    } else if (type == op_setmax) {
        for (int i = 0; i < 10; i++) {
            int tmp = write_struct_ptr->val;
            if (val > (unsigned int) tmp) {
                if (__sync_bool_compare_and_swap(&(write_struct_ptr->val), tmp, val)) {
                    break;
                }
            } else {
                break;
            }
        }
    } else if (type == op_setmin) {
        for (int i = 0; i < 10; i++) {
            int tmp = write_struct_ptr->val;
            if (val < (unsigned int) tmp || (0 == tmp && 0 != val)) {
                if (__sync_bool_compare_and_swap(&(write_struct_ptr->val), tmp, val)) {
                    break;
                }
            } else {
                break;
            }
        }
    }
    return 0;
}

int OssAttrInc0(uint32_t id, uint32_t val) {
    return OssAttrBase(id, 0, val, op_inc);
}

int OssAttrInc(uint32_t id, uint32_t key, uint32_t val) {
    return OssAttrBase(id, key, val, op_inc);
}

int OssAttrSet0(uint32_t id, uint32_t val) {
    return OssAttrBase(id, 0, val, op_set);
}
int OssAttrSet(uint32_t id, uint32_t key, uint32_t val) {
    return OssAttrBase(id, key, val, op_set);
}

int OssAttrSetMax0(uint32_t id, uint32_t val) {
    return OssAttrBase(id, 0, val, op_setmax);
}
int OssAttrSetMax(uint32_t id, uint32_t key, uint32_t val) {
    return OssAttrBase(id, key, val, op_setmax);
}

int OssAttrSetMin0(uint32_t id, uint32_t val) {
    return OssAttrBase(id, 0, val, op_setmin);
}
int OssAttrSetMin(uint32_t id, uint32_t key, uint32_t val) {
    return OssAttrBase(id, key, val, op_setmin);
}

void Show_All_Attr() {
    int i = 0;
    int j = 0;

    if (InitOldOssAddr() != 0) {
        return;
    }

    if (InitNewOssAddr() != 0) {
        return;
    }
    printf("mtime :%u\n", g_ptNewProtectedAddrHolder->oss_addr->mtime);
    printf("val_id :%d\n", g_ptNewProtectedAddrHolder->oss_addr->vindex);
    printf("reserve :%d\n", g_ptNewProtectedAddrHolder->oss_addr->reserve);

    for (i = 0; i < NEW_IDKEY_ID_SIZE; i++) {
        for (j = 0; j < NEW_IDKEY_KEY_SIZE; j++) {
            if (g_ptNewProtectedAddrHolder->oss_addr->id_array[1].key_array[i].value_array[j].val != 0
                    || g_ptNewProtectedAddrHolder->oss_addr->id_array[0].key_array[i].value_array[j].val != 0) {
                printf("id:%d key:%d val0:%u val1:%u op0:%d op1:%d\n", i, j,
                       g_ptNewProtectedAddrHolder->oss_addr->id_array[0].key_array[i].value_array[j].val,
                       g_ptNewProtectedAddrHolder->oss_addr->id_array[1].key_array[i].value_array[j].val,
                       g_ptNewProtectedAddrHolder->oss_addr->id_array[0].key_array[i].value_array[j].res,
                       g_ptNewProtectedAddrHolder->oss_addr->id_array[1].key_array[i].value_array[j].res);
            }
        }
    }
}

int GetIDCRoomID() {
    if (InitNewOssAddr() != 0) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    return g_ptNewProtectedAddrHolder->oss_addr->idc_room_id;
}

int GetMemUsedInfo(unsigned int * timestamp, int * pct) {
    if (InitNewOssAddr() != 0 || !timestamp || !pct) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    (*timestamp) = g_ptNewProtectedAddrHolder->oss_addr->mem_timestamp;
    (*pct) = g_ptNewProtectedAddrHolder->oss_addr->mem_pct;
    return 0;
}

int GetCpuUsedInfo(unsigned int * timestamp, int * pct) {
    if (InitNewOssAddr() != 0 || !timestamp || !pct) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    (*timestamp) = g_ptNewProtectedAddrHolder->oss_addr->cpu_timestamp;
    (*pct) = g_ptNewProtectedAddrHolder->oss_addr->cpu_pct;
    return 0;
}

int GetTrafficUsedInfo(unsigned int * timestamp, int * in_traffic_pct, int * out_traffic_pct) {
    if (InitNewOssAddr() != 0 || !timestamp || !in_traffic_pct || !out_traffic_pct) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    (*timestamp) = g_ptNewProtectedAddrHolder->oss_addr->inner_traffic_timestamp;
    (*in_traffic_pct) = g_ptNewProtectedAddrHolder->oss_addr->in_traffic_pct;
    (*out_traffic_pct) = g_ptNewProtectedAddrHolder->oss_addr->out_traffic_pct;
    return 0;
}

int GetEthSpeedInMbps(unsigned int * eth_speed_in_Mbps) {
    if (InitNewOssAddr() != 0 || !eth_speed_in_Mbps) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    (*eth_speed_in_Mbps) = g_ptNewProtectedAddrHolder->oss_addr->eth_speed_in_Mbps;
    return 0;
}

int GetAllAttrID(void ** vec) {
    int i = 0;
    int j = 0;

    if (InitOldOssAddr() != 0) {
        return -1;
    }

    if (InitNewOssAddr() != 0) {
        return -1;
    }

    if (NULL != (*vec)) {
        return -2;
    }

    IDKeyValueVec * id_key_value_vec = new IDKeyValueVec;
    assert(NULL != id_key_value_vec);
    (*vec) = id_key_value_vec;

#if 0
    for (i = 0; i < NEW_IDKEY_ID_SIZE; i++)
    {
        for (j = 0; j < NEW_IDKEY_KEY_SIZE; j++)
        {
            if ( g_ptNewProtectedAddrHolder->oss_addr->id_array[1].key_array[i].value_array[j].val != 0 ||
                    g_ptNewProtectedAddrHolder->oss_addr->id_array[0].key_array[i].value_array[j].val != 0 )
            {
                oss_attr_id_key_value id_key_value;
                id_key_value.id = i;
                id_key_value.key = j;
                id_key_value.value1 = g_ptNewProtectedAddrHolder->oss_addr->id_array[0].key_array[i].value_array[j].val;
                id_key_value.value2 = g_ptNewProtectedAddrHolder->oss_addr->id_array[1].key_array[i].value_array[j].val;
                id_key_value_vec->push_back(id_key_value);
            }
        }
    }
#endif

    oss_id_struct_t * a_ptIdArray = g_ptNewProtectedAddrHolder->oss_addr->id_array;
    oss_attr_aid_t *a_ptAid = g_ptProtectedAddrHolder->oss_addr->aid;
    for (i = 0; i < NEW_IDKEY_ID_SIZE; i++) {
        for (j = 0; j < NEW_IDKEY_KEY_SIZE; j++) {
            uint32_t iOldShmValue_0 = 0;
            uint32_t iOldShmValue_1 = 0;
            if (i < MAX_ATTR_ID && j < KEY_PER_ID) {
                iOldShmValue_0 = a_ptAid[i].kv[j].val[0];
                iOldShmValue_1 = a_ptAid[i].kv[j].val[1];
            }

            uint32_t iNewShmValue_0 = a_ptIdArray[0].key_array[i].value_array[j].val;
            uint32_t iNewShmValue_1 = a_ptIdArray[1].key_array[i].value_array[j].val;
            uint32_t iOpType_0 = a_ptIdArray[0].key_array[i].value_array[j].res - 1;
            uint32_t iOpType_1 = a_ptIdArray[1].key_array[i].value_array[j].res - 1;

            int iVal_0 = 0;
            int iVal_1 = 0;
            if (iOldShmValue_0 != 0 || iNewShmValue_0 != 0) {

                switch (iOpType_0) {
                    case op_inc:
                        iVal_0 = iOldShmValue_0 + iNewShmValue_0;
                        break;
                    case op_set:
                        iVal_0 = (iNewShmValue_0 == iOldShmValue_0 ? iNewShmValue_0 : iOldShmValue_0);
                        break;
                    case op_setmax:
                        iVal_0 = (iNewShmValue_0 > iOldShmValue_0 ? iNewShmValue_0 : iOldShmValue_0);
                        break;
                    case op_setmin:
                        iVal_0 = (iNewShmValue_0 < iOldShmValue_0 ? iNewShmValue_0 : iOldShmValue_0);
                        break;
                    default:
                        iVal_0 = iOldShmValue_0;
                        break;
                }
            }

            if (iOldShmValue_1 != 0 || iNewShmValue_1 != 0) {

                switch (iOpType_1) {
                    case op_inc:
                        iVal_1 = iOldShmValue_1 + iNewShmValue_1;
                        break;
                    case op_set:
                        iVal_1 = (iNewShmValue_1 == iOldShmValue_1 ? iNewShmValue_1 : iOldShmValue_1);
                        break;
                    case op_setmax:
                        iVal_1 = (iNewShmValue_1 > iOldShmValue_1 ? iNewShmValue_1 : iOldShmValue_1);
                        break;
                    case op_setmin:
                        iVal_1 = (iNewShmValue_1 < iOldShmValue_1 ? iNewShmValue_1 : iOldShmValue_1);
                        break;
                    default:
                        iVal_1 = iOldShmValue_1;
                        break;
                }
            }

            if (0 != iVal_0 || 0 != iVal_1) {
                oss_attr_id_key_value id_key_value;
                id_key_value.id = i;
                id_key_value.key = j;
                id_key_value.value1 = iVal_0;
                id_key_value.value2 = iVal_1;
                id_key_value_vec->push_back(id_key_value);
            }
        }
    }

    return 0;
}

int GenTraceID(uint32_t * pllWriteID, uint32_t & llDapperID) {
    uint32_t llOldID = *pllWriteID;
    llOldID = __sync_add_and_fetch(pllWriteID, 1);
    llDapperID = llOldID & 0xffff;
    return 0;
}

int GenGraphID(uint32_t & uGraphID) {
    if (InitNewOssAddr() != 0) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    return GenTraceID(&(g_ptNewProtectedAddrHolder->oss_addr->graph_id), uGraphID);
}

int GenGraphNodeID(uint32_t & uGraphNodeID) {
    if (InitNewOssAddr() != 0) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -1;
    }
    return GenTraceID(&(g_ptNewProtectedAddrHolder->oss_addr->graph_node_id), uGraphNodeID);
}

//------------------------------------------------------------------

int OssAttrBaseGet(uint32_t id, uint32_t key, uint32_t & val, phxrpc::oss_attr_op_type type) {
    if (InitNewOssAddr() != 0) {
        return -1;
    }

    int vindex = g_ptNewProtectedAddrHolder->oss_addr->vindex;
    /*只能为0或1超出这个值说明内存被写乱了*/
    if (vindex != 0 && vindex != 1) {
        LOGERR("vindex :%d != 0 && vindex :%d != 1", vindex, vindex);
        return -4;
    }

    oss_value_struct_t * write_struct_ptr_0 = &(g_ptNewProtectedAddrHolder->oss_addr->id_array[vindex].key_array[id]
            .value_array[key]);
    if (write_struct_ptr_0->res != 0 && write_struct_ptr_0->res != (unsigned int) (type + 1)) {
        LOGERR("id %d key %d use 2 different api: %d and %d, reject", id, key, write_struct_ptr_0->res, type + 1);
        return -2;
    }

    oss_value_struct_t * write_struct_ptr_1 = &(g_ptNewProtectedAddrHolder->oss_addr->id_array[(0x1 & ~vindex)]
            .key_array[id].value_array[key]);
    if (write_struct_ptr_1->res != 0 && write_struct_ptr_1->res != (unsigned int) (type + 1)) {
        LOGERR("id %d key %d use 2 different api: %d and %d, reject", id, key, write_struct_ptr_1->res, type + 1);
        return -2;
    }

    if (type == op_inc) {
        val = write_struct_ptr_0->val + write_struct_ptr_1->val;

    } else if (type == op_set) {
        val = write_struct_ptr_0->val;

    } else if (type == op_setmax) {
        if (write_struct_ptr_0->val > write_struct_ptr_1->val) {
            val = write_struct_ptr_0->val;
        } else {
            val = write_struct_ptr_1->val;
        }
    } else if (type == op_setmin) {
        if (write_struct_ptr_0->val < write_struct_ptr_1->val) {
            val = write_struct_ptr_0->val;
        } else {
            val = write_struct_ptr_1->val;
        }
    }
    return 0;
}

int OssAttrIncGet(uint32_t id, uint32_t key, uint32_t & val) {
    return OssAttrBaseGet(id, key, val, op_inc);
}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))
        ="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/phoenix_proj/trunk/phxrpc/plugin_sk/phxskoss/iOssAttrApi.cpp $ $Id: iOssAttrApi.cpp 1708014 2016-08-11 07:31:02Z junechen $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

