#pragma once

#include "phxrpc/rpc/server_monitor.h"

#include <stdint.h>
#include <sys/types.h>

namespace phxrpc {


class ServerMonitor_SK: public ServerMonitor {
 public:
	ServerMonitor_SK( uint32_t oss_id );
	 
	virtual ~ServerMonitor_SK();

	virtual void Accept( int count);

	virtual void AcceptFail( int count );

	virtual void RequestCount( int count );

	virtual void ResponseCount( int count );

	virtual void SendBytes( size_t bytes );

	virtual void RecvBytes( size_t bytes );

	virtual void RequestCost( uint64_t cost );

	virtual void ReadError( int count );

	virtual void SendError( int count );

	virtual void OutOfQueue( int count );

	virtual void QueueDelay( uint64_t cost_ms );

	virtual void FastRejectAfterAccept( int count );
	
	virtual void FastRejectAfterRead( int count );

	virtual void WrokerInQueueTimeout( int count );

	virtual void WaitInInQueue( uint64_t cost_ms );

	virtual void WaitInOutQueue( uint64_t cost_ms );

	virtual void SvrCall( int cmdid, const char * method_name, int count );
 private:
	uint32_t oss_id_;
};

}

