#include "client_monitor_sk.h"

#include "iOssAttr.h"

#include <stdint.h>
#include <stdio.h>
  
namespace phxrpc {

//ClientMonitor_SK begin
ClientMonitor_SK :: ClientMonitor_SK( uint32_t oss_id ) 
				: oss_id_(oss_id ){
}
	 
ClientMonitor_SK :: ~ClientMonitor_SK() {
}

void ClientMonitor_SK :: ClientConnect( bool result ) {
	int key = result ? 1 : 2;
	OssAttr4SvrClientConnect( oss_id_, key, 1 );
}

void ClientMonitor_SK :: SendBytes( size_t bytes ) {
	OssAttr4SvrClientSendBytes( oss_id_, (int)bytes );
}

void ClientMonitor_SK :: SendError() {
	OssAttr4CliReadTimeOut( oss_id_, 1 );
}

void ClientMonitor_SK :: SendCount() {
}

void ClientMonitor_SK :: RecvBytes( size_t bytes ) {
	OssAttr4SvrClientRecvBytes( oss_id_, (int)bytes );
}

void ClientMonitor_SK :: RecvCount() {
	OssAttr4CliRecvResp( oss_id_, 1 );
}

void ClientMonitor_SK :: RecvError() {
	OssAttr4CliWriteTimeOut( oss_id_, 1 );
}

void ClientMonitor_SK :: RequestCost( uint64_t begin_time, uint64_t end_time ) {
	if ( end_time < begin_time ) return ;
	int cost_ms = int(end_time - begin_time);
	if ( cost_ms >= 0 ) {
		OssAttr4SvrClientRuntime( oss_id_, cost_ms );
	}
}

void ClientMonitor_SK :: GetEndpointFail() {
	OssAttr4SvrClientGetEndpointFail( oss_id_, 1 );
}


void ClientMonitor_SK :: ClientCall( int cmdid, const char * method_name ) {
	OssAttr4SvrClientCall( oss_id_, cmdid, 1 );
}

void ClientMonitor_SK :: SetOssID( int32_t oss_id ) {
	oss_id_ = oss_id;
}

int ClientMonitor_SK :: GetOssID() {
	return oss_id_;
}

//ClientMonitor_SK end

}


//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/phoenix_proj/trunk/phxrpc/plugin_sk/monitor/client_monitor_sk.cpp $ $Id: client_monitor_sk.cpp 1710334 2016-08-12 10:25:50Z mariohuang $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

