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

#include <limits.h>

#include "iOssAttr4Svr.h"

#include "iOssAttrApi.h"

#define LOGICSVR_CGI_BEGIN_ID 30000
#define WEBFRAME_CGI_BEGIN_ID 40000
#define CGI_COUNT_KEY         30

#define BUSINESS_FAILURE_BEGIN_RET_CODE -60000
#define BUSINESS_FAILURE_END_RET_CODE   -50000

/*
 * ( CLI 端发起连接数 )        -- ID ( 1000, 3000 ) || ( 80000, 82000 )，Key0 为总数，Key1 为连接成功数，Key2 为连接失败数
 * ( CLI 端接口调用总数 )      -- ID ( 3000, 5000 ) || ( 82000, 84000 )
 * ( CLI 端接口调用成功 )      -- ID ( 5000, 7000 ) || ( 84000, 86000 )
 * ( CLI 端接口调用读超时 )    -- ID ( 7000, 9000 ) || ( 86000, 88000 )
 * 
 * ( SVR 端接口调用总数 )      -- ID ( 11000, 13000 ) || ( 90000, 92000 )
 * ( Svrkit耗时统计  )         -- ID ( 13000, 15000 ) || ( 92000, 94000 )
 * ( SVR 端接口调用返回非 0 )  -- ID ( 15000, 17000 ) || ( 94000, 96000 )
 * ( SVR 端接口调用超过 30MS ) -- ID ( 17000, 19000 ) || ( 96000, 98000 )
 *
 */

static const int cOssAttr4SvrIdRange = 2000;

//===================================================================

static int GetOssAttr4SvrClientID(int port) {
    int baseID = 1000;

    if (port > 11000 && port < 13000) {
        baseID = port - 10000;
    }

    if (port > 13000 && port < 15000) {
        baseID = port + 67000;
    }

    return baseID;
}

static int GetOssAttr4SvrID(int port) {
    int baseID = 11000;

    if (port > 11000 && port < 13000) {
        baseID = port;
    }
    if (port > 13000 && port < 15000) {
        baseID = port + 77000;
    }

    return baseID;
}

static int Interval2Key(int msInterval) {
    int key = 0;

    if (msInterval < 30) {
        key = msInterval / 10;
    } else if (msInterval < 100) {
        key = 3;
    } else if (msInterval < 500) {
        key = 4;
    } else if (msInterval < 1000) {
        key = 5;
    } else if (msInterval < 3000) {
        key = 6;
    } else if (msInterval < 5000) {
        key = 7;
    } else if (msInterval < 10000) {
        key = 8;
    } else {
        key = 9;
    }

    return key;
}

static int Size2Key(int size) {
    int key = 0;
    if (size < 1024) {
        key = 0;
    } else if (size < 2 * 1024) {
        key = 1;
    } else if (size < 4 * 1024) {
        key = 2;
    } else if (size < 64 * 1024) {
        key = 3;
    } else {
        key = 4;
    }

    return key;
}

inline int OssAttr4SvrGetLongConnID(int port) {
    int iRealOssAttrID = 0;
    if (port > 11000 && port < 13000) {
        if (12000 < port) {
            iRealOssAttrID = port + 7000;
        } else {
            iRealOssAttrID = port - 11000;
        }
    }
    if (port > 13000 && port < 15000) {
        iRealOssAttrID = port + 85000;
    }
    return iRealOssAttrID;
}

//----------------------------------------------------


void OssAttr4SvrClientConnect(int port, int key, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);

}

void OssAttr4SvrClientSendBytes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 3, count);
}

void OssAttr4SvrClientRecvBytes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 4, count);
}

void OssAttr4SvrClientGetEndpointFail(int port, int count) {

    int iRealOssAttrID = OssAttr4SvrGetLongConnID(port);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 35, 1);
    }
}

void OssAttr4SvrClientMasterHostShield(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 61, count);

}

void OssAttr4SvrClientMaxInt(unsigned int uin, int port, int count) {
    if (INT_MAX == uin) {

        int iRealOssAttrID = OssAttr4SvrGetLongConnID(port);
        if (iRealOssAttrID) {
            OssAttrInc(iRealOssAttrID, 36, count);
        }
    }
}

void OssAttr4SvrRestart(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 10, count);
}

void OssAttr4SvrCoredump(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 11, count);
}

void OssAttr4SvrAcceptFail(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 12, count);
}

void OssAttr4SvrOutOfConn(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 13, count);
}

void OssAttr4SvrOutOfQueue(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 14, count);
}

void OssAttr4SvrRecvBytes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 15, count);

    OssAttrInc(baseID, 50 + Size2Key(count), 1);
}

void OssAttr4SvrRecvBytes_all(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 15, count);
}

void OssAttr4SvrRecvBytes_1K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 50, count);
}

void OssAttr4SvrRecvBytes_2K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 51, count);
}
void OssAttr4SvrRecvBytes_4K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 52, count);
}
void OssAttr4SvrRecvBytes_64K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 53, count);
}
void OssAttr4SvrRecvBytes_limitK(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 54, count);
}

void OssAttr4SvrSendBytes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 16, count);

    OssAttrInc(baseID, 55 + Size2Key(count), 1);
}
void OssAttr4SvrSendBytes_all(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 16, count);
}

void OssAttr4SvrSendBytes_1K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 55, count);
}

void OssAttr4SvrSendBytes_2K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 56, count);
}
void OssAttr4SvrSendBytes_4K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 57, count);
}
void OssAttr4SvrSendBytes_64K(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 58, count);
}
void OssAttr4SvrSendBytes_limitK(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 59, count);
}

void OssAttr4SvrClientRuntime(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 20 + Interval2Key(msInterval);

    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, key, 1);

    OssAttrInc(baseID, 5, msInterval);

}
void OssAttr4SvrClientRuntime_all(int port, int count) {
    if (0 > count) {
        count = 0;
    }
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 5, count);
}

void OssAttr4SvrClientRuntime_10MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 20, count);
}

void OssAttr4SvrClientRuntime_20MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 21, count);
}
void OssAttr4SvrClientRuntime_30MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 22, count);
}
void OssAttr4SvrClientRuntime_100MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 23, count);
}
void OssAttr4SvrClientRuntime_500MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 24, count);
}
void OssAttr4SvrClientRuntime_1000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 25, count);
}
void OssAttr4SvrClientRuntime_3000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 26, count);
}
void OssAttr4SvrClientRuntime_5000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 27, count);
}
void OssAttr4SvrClientRuntime_10000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 28, count);
}
void OssAttr4SvrClientRuntime_limitMS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 29, count);
}

void OssAttr4SvrRuntime(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 30 + Interval2Key(msInterval);

    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, key, 1);

    OssAttrInc(baseID, 17, msInterval);
}
void OssAttr4SvrRuntime_all(int port, int count) {
    if (0 > count) {
        count = 0;
    }
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 17, count);
}

void OssAttr4SvrRuntime_10MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 30, count);
}

void OssAttr4SvrRuntime_20MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 31, count);
}
void OssAttr4SvrRuntime_30MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 32, count);
}
void OssAttr4SvrRuntime_100MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 33, count);
}
void OssAttr4SvrRuntime_500MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 34, count);
}
void OssAttr4SvrRuntime_1000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 35, count);
}
void OssAttr4SvrRuntime_3000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 36, count);
}
void OssAttr4SvrRuntime_5000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 37, count);
}
void OssAttr4SvrRuntime_10000MS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 38, count);
}

void OssAttr4SvrRuntime_limitMS(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 39, count);
}

void OssAttr4SvrQueueDelay(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 40 + Interval2Key(msInterval);

    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, key, 1);

    OssAttrInc(baseID, 18, msInterval);
}

void OssAttr4SvrReadTimeout(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 8, count);
}

void OssAttr4SvrSendTimeout(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 9, count);
}

void OssAttr4SvrReqPacket(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 6, count);
}

void OssAttr4SvrRespPacket(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 7, count);
}

void OssAttr4SvrAccept(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 19, count);
}

void OssAttr4SvrClientCall(int port, int key, int count) {
    int baseID = GetOssAttr4SvrClientID(port) + cOssAttr4SvrIdRange;

    for (; key > 126;)
        key = key - 126;

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);
}

void OssAttr4SvrClientSuccess(int port, int key, int count) {
    // 作废，把这部分 id 用于统计 cgi 对这个 client 的调用次数
#if 0
    int baseID = GetOssAttr4SvrClientID( port ) + cOssAttr4SvrIdRange * 2;

    OssAttrInc( baseID, key, count );

    if( 0 != key ) OssAttrInc( baseID, 0, count );
#endif
}

void OssAttr4SvrClientReadTimeout(int port, int key, int count) {
    int baseID = GetOssAttr4SvrClientID(port) + cOssAttr4SvrIdRange * 3;

    for (; key > 126;)
        key = key - 126;

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);

}

void OssAttr4SvrCall(int port, int key, int count) {
    int baseID = GetOssAttr4SvrID(port);

    for (; key > 126;)
        key = key - 126;

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);
}

void OssAttr4SvrSuccess(int port, int key, int count) {

//作废，这部分id用于统计svrkit中各部分操作的耗时，包括从accept queue接收到fd到push进inqueue、在inquue的耗时、在outqueue的耗时这几部分

#if 0
    int baseID = GetOssAttr4SvrID( port ) + cOssAttr4SvrIdRange;

    OssAttrInc( baseID, key, count );

    if( 0 != key ) OssAttrInc( baseID, 0, count );
#endif

}

void OssAttr4SvrFail(int port, int key, int count) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange * 2;

    for (; key > 126;)
        key = key - 126;

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);
}

void OssAttr4SvrTimeout(int port, int key, int count) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange * 3;

    for (; key > 126;)
        key = key - 126;

    OssAttrInc(baseID, key, count);

    if (0 != key)
        OssAttrInc(baseID, 0, count);
}

void OssAttr4SvrAcceptToInqueue(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 10 + Interval2Key(msInterval);
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, key, 1);
}

void OssAttr4SvrWaitInInQueue(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 20 + Interval2Key(msInterval);
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, key, 1);
}

void OssAttr4SvrWaitInOutQueue(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 30 + Interval2Key(msInterval);
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, key, 1);
}

void OssAttr4SvrEpollAndTimeout(int port, int msInterval) {
    if (0 > msInterval) {
        msInterval = 0;
    }
    int key = 40 + Interval2Key(msInterval);
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, key, 1);
}

void OssAttr4SvrCheckBarrelFail(int port) {
    int key = 3;
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, key, 1);
}

void OssAttr4SvrConnectIDCConnAgentSucc(int iCliOssAttrID, int iAgentOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iCliOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 30, 1);
    }
    iRealOssAttrID = GetOssAttr4SvrClientID(iAgentOssAttrID);
    OssAttrInc(iRealOssAttrID, 0, 1);
    OssAttrInc(iRealOssAttrID, 1, 1);
}

void OssAttr4SvrConnectIDCConnAgentFail(int iCliOssAttrID, int iAgentOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iCliOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 31, 1);
    }
    iRealOssAttrID = GetOssAttr4SvrClientID(iAgentOssAttrID);
    OssAttrInc(iRealOssAttrID, 0, 1);
    OssAttrInc(iRealOssAttrID, 2, 1);
}

void OssAttr4SvrFastRejectAfterAccept2(int iOssAttrID, int cnt) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 32, cnt);
    }
}

void OssAttr4SvrFastRejectAfterAccept(int iOssAttrID) {
    return OssAttr4SvrFastRejectAfterAccept2(iOssAttrID, 1);
}
void OssAttr4SvrFastRejectAfterRead2(int iOssAttrID, int cnt) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 33, cnt);
    }
}

void OssAttr4SvrFastRejectAfterRead(int iOssAttrID) {
    return OssAttr4SvrFastRejectAfterRead2(iOssAttrID, 1);
}

void OssAttr4SvrKillBusyWorker(int iOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 34, 1);
    }
}

void OssAttr4SvrClientMasterHostShieldGlobal(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 60, count);
}

void OssAttr4SvrClientConnectRunTime(int port, int count) {
    if (0 > count) {
        count = 0;
    }
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 62, count);
}

void OssAttr4SvrHitMemoryLimit(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 66, count);
}

void OssAttr4SvrFinalErr(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 68, count);
}

void OssAttr4SvrFinalReq(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);

    OssAttrInc(baseID, 76, count);
}
void OssAttr4SvrLogicSvrCgiCallCount(int iCmdID) {
    int iOssAttrID = iCmdID + LOGICSVR_CGI_BEGIN_ID;
    if (30000 < iOssAttrID && 32000 >= iOssAttrID) {
        OssAttrInc(iOssAttrID, CGI_COUNT_KEY, 1);
    }
}

void OssAttr4SvrWebFrameCgiCallCount(int iCmdID) {
    int iOssAttrID = iCmdID + WEBFRAME_CGI_BEGIN_ID;
    if (40000 < iOssAttrID && 50000 >= iOssAttrID) {
        OssAttrInc(iOssAttrID, CGI_COUNT_KEY, 1);
    }
}

void OssAttr4SvrEnableFastReject(int port, int value) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrSetMax(baseID, 69, value);
}

void OssAttr4SvrDataticketVerify(int port, int ret) {
    int baseID;
    if (-13 == ret) {
        baseID = GetOssAttr4SvrClientID(port);
        OssAttrInc(baseID, 70, 1);
        OssAttrInc(52499, 25, 1);
    } else if (-14 == ret) {
        baseID = GetOssAttr4SvrClientID(port);
        OssAttrInc(baseID, 71, 1);
        OssAttrInc(52499, 26, 1);
    }
}

void OssAttr4SvrEnableIoThreadNum(int port, int value) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrSetMax(baseID, 72, value);
}

void OssAttr4SvrFastRejectAverageTime(int port, unsigned int value) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrSetMax(baseID, 73, value);
}

void OssAttr4SvrCallStepLimit(int port, unsigned int value) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrSetMax(baseID, 74, value);
}

void OssAttr4TrafficMgrPercentTokenTimes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 75, count);
}

void OssAttr4SvrFastRejectQoS(int iOssAttrID, int iPriority, unsigned int iUinBucket, unsigned int iUinBucketCnt,
                              int iCurrentPriority, unsigned int iCurrentUinBucket, int iState) {

    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 73 + (iPriority - 1 % 20), 1);

        if (2 == iState) {
            OssAttrInc(iRealOssAttrID, 93, 1);
            OssAttrSetMin(iRealOssAttrID, 98, 1);
        } else {
            OssAttrInc(iRealOssAttrID, 94 + iUinBucket / (iUinBucketCnt / 4), 1);
            OssAttrSetMin(iRealOssAttrID, 98, ((iCurrentPriority - 1) * (iUinBucketCnt + 1) + iCurrentUinBucket + 1));
        }
        //OssAttrInc( iRealOssAttrID, 98 + (iCmdId % 5), 1 );
    }
}

void OssAttr4SvrFastRejectCauseType(int iOssAttrID, int iCause) {

    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        if (!(iCause ^ (0x1 | 0x2))) {
            OssAttrInc(iRealOssAttrID, 101, 1);
        } else if (iCause & 0x1) {
            OssAttrInc(iRealOssAttrID, 99, 1);
        } else {
            OssAttrInc(iRealOssAttrID, 100, 1);
        }
    }
}

void OssAttr4SvrFastRejectType(int port, int value) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrSetMax(baseID, 77, value);
}

void OssAttr4AllBlockDecreaseTimes(int port, int count) {
    int baseID = GetOssAttr4SvrClientID(port);
    OssAttrInc(baseID, 78, count);
}

void OssAttr4SvrClientFastReject(int iOssAttrID, int cnt) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 103, cnt);
    }
}

void OssAttr4SvrBusinessFailure(int ret_code, int port, int count) {
    if (ret_code >= BUSINESS_FAILURE_BEGIN_RET_CODE && ret_code <= BUSINESS_FAILURE_END_RET_CODE) {
        int baseID = GetOssAttr4SvrClientID(port);
        OssAttrInc(baseID, 79, count);
    }
}

void OssAttr4SvrClientPercentBlock(int iOssAttrID, int cnt) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 107, cnt);
    }
}

void OssAttr4CliReadTimeOut(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 51, cnt);
}
void OssAttr4CliWriteTimeOut(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 52, cnt);
}
void OssAttr4CliRecvResp(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 53, cnt);
}
void OssAttr4CliParsePacketFail(int port) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 54, 1);
}
void OssAttr4CliFastReject(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 55, cnt);
}
void OssAttr4CliRespSuccess(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 56, cnt);
}
void OssAttr4CliNoResp(int port) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 57, 1);
}

void OssAttr4SvrTransferTimeoutWithCnt(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 61, cnt);
}

void OssAttr4SvrTransferTimeout(int port) {
    OssAttr4SvrTransferTimeoutWithCnt(port, 1);
}

void OssAttr4SvrWrokerInQueueTimeout(int port) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 62, 1);
}

void OssAttr4SvrWrokerInQueueTimeout2(int port, int count )
{
	int baseID = GetOssAttr4SvrID( port ) + cOssAttr4SvrIdRange;
	OssAttrInc( baseID, 62, count );
}

void OssAttr4SvrNoResp(int port) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 63, 1);
}

void OssAttr4SvrMaybeSuccessWithCnt(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 64, cnt);
}

void OssAttr4SvrMaybeSuccess(int port) {
    OssAttr4SvrMaybeSuccessWithCnt(port, 1);
}

void OssAttr4SvrNoTransferTimeoutWithCnt(int port, int cnt) {
    int baseID = GetOssAttr4SvrID(port) + cOssAttr4SvrIdRange;
    OssAttrInc(baseID, 65, cnt);
}

void OssAttr4SvrNoTransferTimeout(int port) {
    OssAttr4SvrNoTransferTimeoutWithCnt(port, 1);
}

int OssAttr4GetTotalSvrRuntime(int port, unsigned int & run_time) {
    int baseID = GetOssAttr4SvrClientID(port);
    return OssAttrIncGet(baseID, 17, run_time);
}

int OssAttr4GetSvrCallCnt(int port, unsigned int & cnt) {
    int baseID = GetOssAttr4SvrID(port);
    return OssAttrIncGet(baseID, 0, cnt);
}

void OssAttr4SvrClientQoSState(int iOssAttrID, unsigned char /* iApiVersion */, unsigned int iUinBucketCnt,
                               int iCgiPriority, unsigned int iUinPriority) {

    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        if (0 == iCgiPriority) {
            OssAttrSetMin(iRealOssAttrID, 102, 0);
        } else {
            OssAttrSetMin(iRealOssAttrID, 102, ((iCgiPriority - 1) * (iUinBucketCnt + 1) + iUinPriority + 1));
        }
    }
}

void OssAttr4SvrReqUnCompressError(int iOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 104, 1);
    }
}

void OssAttr4SvrSysOrResourceFail(int iOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 105, 1);
    }
}

void OssAttr4SvrSysOrResourceFailRate(int iOssAttrID, int iFailRate) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrSetMax(iRealOssAttrID, 106, iFailRate);
    }
}

void OssAttr4SvrInitCgroupFail(int iOssAttrID) {
    int iRealOssAttrID = OssAttr4SvrGetLongConnID(iOssAttrID);
    if (iRealOssAttrID) {
        OssAttrInc(iRealOssAttrID, 108, 1);
    }
}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))
        ="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/phoenix_proj/trunk/phxrpc/plugin_sk/phxskoss/iOssAttr4Svr.cpp $ $Id: iOssAttr4Svr.cpp 1708014 2016-08-11 07:31:02Z junechen $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

