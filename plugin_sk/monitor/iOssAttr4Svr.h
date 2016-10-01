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
 * �������� svrkit �� OssAttr ����
 *
 * 1. ID ��˵��
 *
 * �� svrkit ������ server/client �˿ڷ����� ( 11000, 13000 ) ���Ѷ˿�ӳ��Ϊ OssAttrID
 *
 * ( SVR/CLI �ۺ�ͳ���� )      -- ID ( 1000, 3000 )
 *      1) Key0 Ϊ������Key1 Ϊ���ӳɹ�����Key2 Ϊ����ʧ������
 *         Key3 ��������Key4 ��������Key5 Client �����ܺ�ʱ��
 *         Key6 Server ����������Key7 Server ��Ӧ����,
 *         Key8 Server ����ʱ��Key9 Server д��ʱ
 *      2) Key10 ͳ��������Key11 ͳ�� coredump, Key12 ͳ�� Accept ʧ�ܣ�
 *         Key13 ���������������Key14 ͳ�ƶ�������Key15 ��������
 *         Key16 ��������Key17 Server�����ܺ�ʱ��Key18 ���д����ܺ�ʱ��
 *         Key19 Accept ����
 *      3) [ Key20, Key30 ) ͳ�� Client ���нӿں�ʱ�ֲ�
 *      4) [ Key30, Key40 ) ͳ�� Server ���нӿں�ʱ�ֲ�
 *      5) [ Key40, Key50 ) ͳ�� ���� ��ʱ�ֲ�
 *      6) [ Key50, Key54 ) ͳ�� ����� ��С�ֲ�
 *      7) [ Key55, Key60 ) ͳ�� ��Ӧ�� ��С�ֲ�
 *      8) Key60 
 *         !!!! ע�� ������Ϊ  Client GetEndpoint ʧ�ܣ� Ų��( 0, 1000 ] �� ( 19000, 20000 ) ��key 35
 *         ��Ϊ������ȫ������
 *      9) Key61 ����������
 *      10) Key62 
 *         !!!! ע�� ������Ϊ ����21�ں�������
 *         Ų�� ��0?000] �� ��19000,20000)��key 36
 *         ��Ϊconnect��ʱͳ��
 *      11) Key63 ��ʧ���� Key64 ��ʧ����
 *      12) Key66 �ڴ�ﵽ����
 *      13) Key67 ����������̨ģ��Ĵ���
 *      14) Key68 mmclientcall�е�3�λ������Ծ�ʧ�ܣ��ϱ�Ϊ����ʧ��
 *      15) Key69 fast reject����ֵ
 *      16) Key70 dataticket -13 verify error
 *      17) Key71 dataticket -14 verify error
 *      18) Key72 IoThreadNum ����ֵ
 *      19) Key73 FastReject������ƽ����ʱ
 *      20) Key75 trafficmgr�ٷֱ�token��������
 *      21) Key76 MMClientCallҵ��������
 *      22) Key77 FastReject Type
 *      23) Key78 ȫ���ν���ʱ���Դ���
 *      24) Key79 ҵ��ʧ�ܱ���
 *
 * ( CLI �˽ӿڵ������� )      -- ID ( 3000, 5000 )
 * ( CLI �� CGI ������ )       -- ID ( 5000, 7000 )
 * ( CLI �˽ӿڵ��ö���ʱ )    -- ID ( 7000, 9000 )
 * 
 * ( SVR �˽ӿڵ������� )      -- ID ( 11000, 13000 )
 *
 * ( SVR �˽ӿڵ��÷��� 0 )    -- ID ( 13000, 15000 )  !!!����!!!
 *
 * ID ( 13000, 15000)��������ͳ��Svrkit�и�������׶εĺ�ʱ
 *      1) [ Key10, Key20 ) ͳ�� ��Accept Queue���յ�fd �� �������push��InQueue �ĺ�ʱ�ֲ�
 *      2) [ Key20, Key30 ) ͳ�� ������InQueue�еȴ� �ĺ�ʱ�ֲ�
 *      3) [ Key30, Key40 ) ͳ�� ������OutQueue�еȴ��ĺ�ʱ�ֲ�
 *      4) [ Key40, Key50 ) ͳ�� һ��epoll�ͳ�ʱ����ĺ�ʱ�ֲ�
 *      5) Key3             ͳ�� CheckBarrel failed ����, �����Ѿ�û��key,����ֻ�ܷŵ�����
 6) [key51, key60]   cli����Ч���ͳ��
 7) [key61, key70]   svr����Ч���ͳ��
 *
 *
 *
 * ( SVR �˽ӿڵ��÷��ط� 0 )  -- ID ( 15000, 17000 )
 * ( SVR �˽ӿڵ��ó��� 30MS ) -- ID ( 17000, 19000 )
 *
 * 2. ���ӿ�ͳ�� Key ��˵��
 *
 * 1) ���� ID �� Key �������£�Key0 ���� CmdID �ܺͣ�[ Key1 ~ Key62 ] �� CmdID �ֽӿ�ͳ�ơ�
 * 2) Key0 ���ۼ������½ӿ��Զ���ɣ��ⲿ�����߲���Ҫ��ʽ���á�
 *
 * 3. svr ������ coredump ͳ��
 * ʹ�� ID ( 1000, 3000 ) , Key10 ͳ������, Key11 ͳ�� coredump
 *
 *
 *
 * ��̨ģ��˿���ӡ[13000, 15000]����Σ���Ӧ��ossattr id�ֶ�Ϊ[80000,100000]
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
// �������ϣ����ű��� undefined symbol
void OssAttr4SvrClientSuccess(int port, int key, int count);


/* ID ( 7000, 9000 ) */
void OssAttr4SvrClientReadTimeout(int port, int key, int count);

/*********************************************************/


/*********************************************************/

/* ID ( 11000, 13000 ) */
void OssAttr4SvrCall(int port, int key, int count);

/* ID ( 13000, 15000 ) */
// �������ϣ����ű��� undefined symbol
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
 * key1  connect���svr�ɹ�
 * key2  connect���svrʧ��
 * key5  agent���ú���ܺ�ʱ
 * key6  agent���ú�˶�ʧ��
 * key7  agent���ú��дʧ��
 *
 * key10 ������
 * key11 ������
 * key12 ������
 * key13 req������
 * key14 resp������
 *
 * key21 ���svr����ʱ
 * key22 ��˳����Ӳ���Ծ��ʱ
 * key23 connect���svr��ʱ
 *
 *
 * key30 connect idc connagent �ɹ�
 * key31 connect idc connagent ʧ��
 * 
 * [key40, key50)  connagent���ú�˺�ʱ�ֲ�
 *
 *
 * key 30 connect idc connagent succ
 * key 31 connect idc connagent fail
 * key 32 fastreject after accept
 * key 33 fastreject after read
 * key 34 kill busy worker
 * key 35 client get endpoint failed 
 * key 36 ����21�ں�������
 *
 *
 * [key73, key93)  FastReject Cgi���ȼ��ܾ���
 * key 93          FastReject SEVERE State
 * [key94, key98)  FastReject Uin���ȼ��ܾ���
 * [key98, key103) FastReject CmdId���ȼ��ܾ���
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


//��Ч����ϱ�
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

