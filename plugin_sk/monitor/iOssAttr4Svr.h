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

/**
 * 定义用于 svrkit 的 OssAttr 函数
 *
 * 1. ID 的说明
 *
 * 用 svrkit 开发的 server/client 端口分配在 ( 11000, 13000 ) ，把端口映射为 OssAttrID
 *
 * ( SVR/CLI 综合统计项 )      -- ID ( 1000, 3000 )
 *      1) Key0 为总数，Key1 为连接成功数，Key2 为连接失败数，
 *         Key3 发流量，Key4 收流量，Key5 Client 调用总耗时，
 *         Key6 Server 请求总数，Key7 Server 响应总数,
 *         Key8 Server 读超时，Key9 Server 写超时
 *      2) Key10 统计重启，Key11 统计 coredump, Key12 统计 Accept 失败，
 *         Key13 超过最大连接数，Key14 统计队列满，Key15 入流量，
 *         Key16 出流量，Key17 Server处理总耗时，Key18 队列处理总耗时，
 *         Key19 Accept 总数
 *      3) [ Key20, Key30 ) 统计 Client 所有接口耗时分布
 *      4) [ Key30, Key40 ) 统计 Server 所有接口耗时分布
 *      5) [ Key40, Key50 ) 统计 队列 耗时分布
 *      6) [ Key50, Key54 ) 统计 请求包 大小分布
 *      7) [ Key55, Key60 ) 统计 响应包 大小分布
 *      8) Key60 
 *         !!!! 注意 不再作为  Client GetEndpoint 失败， 挪到( 0, 1000 ] 和 ( 19000, 20000 ) 的key 35
 *         改为主机被全局屏蔽
 *      9) Key61 主机被屏蔽
 *      10) Key62 
 *         !!!! 注意 不再作为 出现21亿号码问题
 *         挪到 （0?000] 和 （19000,20000)的key 36
 *         改为connect耗时统计
 *      11) Key63 总失败率 Key64 总失败率
 *      12) Key66 内存达到限制
 *      13) Key67 调用其他后台模块的次数
 *      14) Key68 mmclientcall中的3次换机重试均失败，上报为最终失败
 *      15) Key69 fast reject配置值
 *      16) Key70 dataticket -13 verify error
 *      17) Key71 dataticket -14 verify error
 *      18) Key72 IoThreadNum 配置值
 *      19) Key73 FastReject按分钟平均耗时
 *      20) Key75 trafficmgr百分比token调整次数
 *      21) Key76 MMClientCall业务请求数
 *      22) Key77 FastReject Type
 *      23) Key78 全屏蔽降超时策略次数
 *      24) Key79 业务失败报警
 *
 * ( CLI 端接口调用总数 )      -- ID ( 3000, 5000 )
 * ( CLI 端 CGI 调用数 )       -- ID ( 5000, 7000 )
 * ( CLI 端接口调用读超时 )    -- ID ( 7000, 9000 )
 * 
 * ( SVR 端接口调用总数 )      -- ID ( 11000, 13000 )
 *
 * ( SVR 端接口调用返回 0 )    -- ID ( 13000, 15000 )  !!!作废!!!
 *
 * ID ( 13000, 15000)现在用于统计Svrkit中各个处理阶段的耗时
 *      1) [ Key10, Key20 ) 统计 从Accept Queue接收到fd 到 将请求包push进InQueue 的耗时分布
 *      2) [ Key20, Key30 ) 统计 请求在InQueue中等待 的耗时分布
 *      3) [ Key30, Key40 ) 统计 请求在OutQueue中等待的耗时分布
 *      4) [ Key40, Key50 ) 统计 一轮epoll和超时处理的耗时分布
 *      5) Key3             统计 CheckBarrel failed 次数, 由于已经没有key,所以只能放到这里
 6) [key51, key60]   cli端有效输出统计
 7) [key61, key70]   svr端有效输出统计
 *
 *
 *
 * ( SVR 端接口调用返回非 0 )  -- ID ( 15000, 17000 )
 * ( SVR 端接口调用超过 30MS ) -- ID ( 17000, 19000 )
 *
 * 2. 按接口统计 Key 的说明
 *
 * 1) 以上 ID 的 Key 安排如下：Key0 所有 CmdID 总和，[ Key1 ~ Key62 ] 按 CmdID 分接口统计。
 * 2) Key0 的累加由以下接口自动完成，外部调用者不需要显式调用。
 *
 * 3. svr 重启和 coredump 统计
 * 使用 ID ( 1000, 3000 ) , Key10 统计重启, Key11 统计 coredump
 *
 *
 *
 * 后台模块端口添加13000, 15000]这个段，对应的ossattr id分段为[80000,100000]
 *
 */

/* ID ( 1000, 3000 ) */

void OssAttr4SvrClientConnect(int port, int key, int count);
void OssAttr4SvrClientSendBytes(int port, int count);
void OssAttr4SvrClientRecvBytes(int port, int count);

void OssAttr4SvrClientMasterHostShieldGlobal(int port, int count);
void OssAttr4SvrClientMasterHostShield(int port, int count);

void OssAttr4SvrRestart(int port, int count);
void OssAttr4SvrCoredump(int port, int count);
void OssAttr4SvrAcceptFail(int port, int count);
void OssAttr4SvrOutOfConn(int port, int count);
void OssAttr4SvrOutOfQueue(int port, int count);
void OssAttr4SvrRecvBytes(int port, int count);
void OssAttr4SvrRecvBytes_all(int port, int count);
void OssAttr4SvrRecvBytes_1K(int port, int count);
void OssAttr4SvrRecvBytes_2K(int port, int count);
void OssAttr4SvrRecvBytes_4K(int port, int count);
void OssAttr4SvrRecvBytes_64K(int port, int count);
void OssAttr4SvrRecvBytes_limitK(int port, int count);
void OssAttr4SvrSendBytes(int port, int count);
void OssAttr4SvrSendBytes_all(int port, int count);
void OssAttr4SvrSendBytes_1K(int port, int count);
void OssAttr4SvrSendBytes_2K(int port, int count);
void OssAttr4SvrSendBytes_4K(int port, int count);
void OssAttr4SvrSendBytes_64K(int port, int count);
void OssAttr4SvrSendBytes_limitK(int port, int count);

void OssAttr4SvrClientRuntime(int port, int msInterval);
void OssAttr4SvrClientRuntime_all(int port, int msInterval);
void OssAttr4SvrClientRuntime_10MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_20MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_30MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_100MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_500MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_1000MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_3000MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_5000MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_10000MS(int port, int msInterval);
void OssAttr4SvrClientRuntime_limitMS(int port, int msInterval);
void OssAttr4SvrRuntime(int port, int msInterval);
void OssAttr4SvrRuntime_all(int port, int msInterval);
void OssAttr4SvrRuntime_10MS(int port, int msInterval);
void OssAttr4SvrRuntime_20MS(int port, int msInterval);
void OssAttr4SvrRuntime_30MS(int port, int msInterval);
void OssAttr4SvrRuntime_100MS(int port, int msInterval);
void OssAttr4SvrRuntime_500MS(int port, int msInterval);
void OssAttr4SvrRuntime_1000MS(int port, int msInterval);
void OssAttr4SvrRuntime_3000MS(int port, int msInterval);
void OssAttr4SvrRuntime_5000MS(int port, int msInterval);
void OssAttr4SvrRuntime_10000MS(int port, int msInterval);
void OssAttr4SvrRuntime_limitMS(int port, int msInterval);
void OssAttr4SvrQueueDelay(int port, int msInterval);

void OssAttr4SvrReadTimeout(int port, int count);
void OssAttr4SvrSendTimeout(int port, int count);
void OssAttr4SvrReqPacket(int port, int count);
void OssAttr4SvrRespPacket(int port, int count);
void OssAttr4SvrAccept(int port, int count);

void OssAttr4SvrClientConnectRunTime(int port, int count);

void OssAttr4SvrHitMemoryLimit(int port, int count);

void OssAttr4SvrFinalErr(int port, int count);

void OssAttr4SvrFinalReq(int port, int count);

void OssAttr4SvrEnableFastReject(int port, int value);

void OssAttr4SvrDataticketVerify(int port, int ret);

void OssAttr4SvrEnableIoThreadNum(int port, int value);

void OssAttr4SvrFastRejectAverageTime(int port, unsigned int value);

void OssAttr4TrafficMgrPercentTokenTimes(int port, int count);

void OssAttr4SvrFastRejectType(int port, int value);

void OssAttr4AllBlockDecreaseTimes(int port, int count);

void OssAttr4SvrBusinessFailure(int ret_code, int port, int count);

/* ID ( 3000, 5000 ) */
void OssAttr4SvrClientCall(int port, int key, int count);

/* ID ( 5000, 7000 ) */
// 函数作废，留着避免 undefined symbol
void OssAttr4SvrClientSuccess(int port, int key, int count);


/* ID ( 7000, 9000 ) */
void OssAttr4SvrClientReadTimeout(int port, int key, int count);

/*********************************************************/


/*********************************************************/

/* ID ( 11000, 13000 ) */
void OssAttr4SvrCall(int port, int key, int count);

/* ID ( 13000, 15000 ) */
// 函数作废，留着避免 undefined symbol
void OssAttr4SvrSuccess(int port, int key, int count);

void OssAttr4SvrAcceptToInqueue(int port, int msInterval);
void OssAttr4SvrWaitInInQueue(int port, int msInterval);
void OssAttr4SvrWaitInOutQueue(int port, int msInterval);
void OssAttr4SvrEpollAndTimeout(int port, int msInterval);

void OssAttr4SvrCheckBarrelFail(int port);

/* ID ( 15000, 17000 ) */
void OssAttr4SvrFail(int port, int key, int count);

/* ID ( 17000, 19000 ) */
void OssAttr4SvrTimeout(int port, int key, int count);

/* ID ( 0, 1000 ] for port (11000, 12000] */
/* ID ( 19000, 20000 ) for port (12000, 13000)  */
/*
 *
 * key1  connect后端svr成功
 * key2  connect后端svr失败
 * key5  agent调用后端总耗时
 * key6  agent调用后端读失败
 * key7  agent调用后端写失败
 *
 * key10 连接数
 * key11 收流量
 * key12 发流量
 * key13 req请求数
 * key14 resp请求数
 *
 * key21 后端svr处理超时
 * key22 后端长连接不活跃超时
 * key23 connect后端svr超时
 *
 *
 * key30 connect idc connagent 成功
 * key31 connect idc connagent 失败
 * 
 * [key40, key50)  connagent调用后端耗时分布
 *
 *
 * key 30 connect idc connagent succ
 * key 31 connect idc connagent fail
 * key 32 fastreject after accept
 * key 33 fastreject after read
 * key 34 kill busy worker
 * key 35 client get endpoint failed 
 * key 36 出现21亿号码问题
 *
 *
 * [key73, key93)  FastReject Cgi优先级拒绝数
 * key 93          FastReject SEVERE State
 * [key94, key98)  FastReject Uin优先级拒绝数
 * [key98, key103) FastReject CmdId优先级拒绝数
 *
 *
 * key 103 client fastreject 
 * key 104 req uncompress error 
 *
 * key 105 sys/resource fail 
 * key 106 sys/resource fail rate
 *
 * key 107 client percent block
 *
 * key 108 init cgroup fail
 *
 */

void OssAttr4SvrConnectIDCConnAgentSucc(int iCliOssAttrID, int iAgentOssAttrID);
void OssAttr4SvrConnectIDCConnAgentFail(int iCliOssAttrID, int iAgentOssAttrID);

void OssAttr4SvrFastRejectAfterAccept(int iOssAttrID);
void OssAttr4SvrFastRejectAfterRead(int iOssAttrID);

void OssAttr4SvrFastRejectAfterAccept2(int iOssAttrID, int cnt);
void OssAttr4SvrFastRejectAfterRead2(int iOssAttrID, int cnt);

void OssAttr4SvrKillBusyWorker(int iOssAttrID);
void OssAttr4SvrClientGetEndpointFail(int port, int count);

void OssAttr4SvrClientMaxInt(unsigned int uin, int port, int count);
void OssAttr4SvrCallStepLimit(int port, unsigned int value);

void OssAttr4SvrFastRejectQoS(int iOssAttrID, int iPriority, unsigned int iUinBucket, unsigned int iUinBucketCnt,
                              int iCurrentPriority, unsigned int iCurrentUinBucket, int iState);

void OssAttr4SvrClientFastReject(int iOssAttrID, int cnt);
void OssAttr4SvrFastRejectCauseType(int iOssAttrID, int iCause);

void OssAttr4SvrClientPercentBlock(int iOssAttrID, int cnt);

void OssAttr4SvrClientQoSState(int iOssAttrID, unsigned char iApiVersion, unsigned int iUinBucketCnt, int iCgiPriority,
                               unsigned int iUinPriority);
void OssAttr4SvrReqUnCompressError(int iOssAttrID);

void OssAttr4SvrSysOrResourceFail(int iOssAttrID);
void OssAttr4SvrSysOrResourceFailRate(int iOssAttrID, int iFailRate);
void OssAttr4SvrInitCgroupFail(int iOssAttrID);

//------------------------
//cgi call count
void OssAttr4SvrLogicSvrCgiCallCount(int iCmdID);
void OssAttr4SvrWebFrameCgiCallCount(int iCmdID);


//有效输出上报
void OssAttr4CliReadTimeOut(int port, int cnt = 1);
void OssAttr4CliWriteTimeOut(int port, int cnt = 1);
void OssAttr4CliRecvResp(int port, int cnt = 1);
void OssAttr4CliParsePacketFail(int port);
void OssAttr4CliFastReject(int port, int cnt = 1);
void OssAttr4CliRespSuccess(int port, int cnt = 1);
void OssAttr4CliNoResp(int port);

void OssAttr4SvrTransferTimeout(int port);
void OssAttr4SvrTransferTimeoutWithCnt(int port, int cnt);

void OssAttr4SvrWrokerInQueueTimeout(int port);
void OssAttr4SvrWrokerInQueueTimeout2(int port, int count );
void OssAttr4SvrNoResp(int port);

void OssAttr4SvrMaybeSuccess(int port);
void OssAttr4SvrMaybeSuccessWithCnt(int port, int cnt);

void OssAttr4SvrNoTransferTimeout(int port);
void OssAttr4SvrNoTransferTimeoutWithCnt(int port, int cnt);

int OssAttr4GetTotalSvrRuntime(int port, unsigned int & run_time);
int OssAttr4GetSvrCallCnt(int port, unsigned int & cnt);


#ifdef   __cplusplus 
}
#endif 

