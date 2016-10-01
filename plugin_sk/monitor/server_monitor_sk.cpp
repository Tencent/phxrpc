#include "server_monitor_sk.h"

#include "iOssAttr.h"

#include <stdint.h>
#include <stdio.h>

namespace phxrpc {


//ServerMonitor_SK begin
ServerMonitor_SK :: ServerMonitor_SK( uint32_t oss_id )
				 : oss_id_( oss_id ) {
}
	 
ServerMonitor_SK :: ~ServerMonitor_SK() {
}

void ServerMonitor_SK :: Accept( int count) {
	OssAttr4SvrAccept( oss_id_, count );
}

void ServerMonitor_SK :: AcceptFail( int count ) {
	OssAttr4SvrAcceptFail( oss_id_, count );
}

void ServerMonitor_SK :: RequestCount( int count ) {
	OssAttr4SvrReqPacket( oss_id_, count );
}

void ServerMonitor_SK :: ResponseCount( int count ) {
	OssAttr4SvrRespPacket( oss_id_, count );
}

void ServerMonitor_SK :: SendBytes( size_t bytes ) {
	OssAttr4SvrSendBytes( oss_id_, (int)bytes );
}

void ServerMonitor_SK :: RecvBytes( size_t bytes ) {
	OssAttr4SvrRecvBytes( oss_id_, (int)bytes );
}

void ServerMonitor_SK :: RequestCost( uint64_t cost ) {
	OssAttr4SvrRuntime( oss_id_, cost );
}

void ServerMonitor_SK :: ReadError( int count ) {
	OssAttr4SvrReadTimeout( oss_id_, count );
}

void ServerMonitor_SK :: SendError( int count ) {
	OssAttr4SvrSendTimeout( oss_id_, count );
}

void ServerMonitor_SK :: OutOfQueue( int count ) {
	OssAttr4SvrOutOfQueue( oss_id_, count );
}

void ServerMonitor_SK :: QueueDelay( uint64_t cost_ms ) {
	OssAttr4SvrQueueDelay( oss_id_, (int)cost_ms );
}

void ServerMonitor_SK :: FastRejectAfterAccept( int count ) {
	OssAttr4SvrFastRejectAfterAccept2( oss_id_, count );
}
	
void ServerMonitor_SK :: FastRejectAfterRead( int count ) {
	OssAttr4SvrFastRejectAfterRead2( oss_id_, count );
}

void ServerMonitor_SK :: WrokerInQueueTimeout( int count ) {
	OssAttr4SvrWrokerInQueueTimeout2( oss_id_, count ); 
}

void ServerMonitor_SK :: WaitInInQueue( uint64_t cost_ms ) {
	OssAttr4SvrWaitInInQueue( oss_id_, (int)cost_ms );
}

void ServerMonitor_SK :: WaitInOutQueue( uint64_t cost_ms ) {
	OssAttr4SvrWaitInOutQueue( oss_id_, (int)cost_ms );
}

void ServerMonitor_SK :: SvrCall( int cmdid, const char * method_name, int count ) {
	if ( cmdid > 0 ) {
		OssAttr4SvrCall( oss_id_, cmdid, count );
	}
}

//ServerMonitor_SK end

}


//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/phoenix_proj/trunk/phxrpc/plugin_sk/monitor/server_monitor_sk.cpp $ $Id: server_monitor_sk.cpp 1710334 2016-08-12 10:25:50Z mariohuang $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

