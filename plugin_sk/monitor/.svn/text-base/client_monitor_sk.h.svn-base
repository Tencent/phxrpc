#pragma once

#include "phxrpc/rpc/client_monitor.h"

#include <stdint.h>
#include <sys/types.h>

namespace phxrpc {

class ClientMonitor_SK: public ClientMonitor {
 public:
	 ClientMonitor_SK( uint32_t oss_id );
	 
	 virtual ~ClientMonitor_SK();

	 void SetOssID( int32_t oss_id );

	 int32_t GetOssID();
 public:
	virtual void ClientConnect( bool result );

	virtual void SendBytes( size_t bytes );

	virtual void SendError();

	virtual void SendCount();

	virtual void RecvBytes( size_t bytes );

	virtual void RecvCount();

	virtual void RecvError();

	virtual void RequestCost( uint64_t begin_time, uint64_t end_time );

	virtual void GetEndpointFail();

	virtual void ClientCall( int cmdid, const char * method_name );
 private:
	int32_t oss_id_;
};

}

