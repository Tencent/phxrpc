#pragma once

#include "client_monitor_sk.h"
#include "server_monitor_sk.h"
#include "phxrpc/rpc/monitor_factory.h"

#include <memory>
#include <string>
#include <map>

#include <pthread.h>

namespace phxrpc {

class MonitorFactory_SK : public MonitorFactory {
 public:
	MonitorFactory_SK();

	virtual ~MonitorFactory_SK();

	virtual std::shared_ptr<ClientMonitor> CreateClientMonitor(const int oss_id);
	virtual std::shared_ptr<ServerMonitor> CreateServerMonitor(const int oss_id);


 private:

	std::shared_ptr<ServerMonitor_SK> server_monitor_;
	//server monitor will be always the same, no need to create every time

	pthread_rwlock_t rwlock_;
};


}
