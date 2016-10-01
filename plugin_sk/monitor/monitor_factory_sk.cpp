#include "monitor_factory_sk.h"

#include <memory>
#include <stdlib.h>

namespace phxrpc {

static MonitorFactory_SK g_monitor_factory_sk;

MonitorFactory_SK :: MonitorFactory_SK() {
	pthread_rwlock_init( &rwlock_, NULL );

    MonitorFactory::SetFactory(this);
}

MonitorFactory_SK :: ~MonitorFactory_SK() {
	pthread_rwlock_destroy( &rwlock_ );
}

std::shared_ptr<ClientMonitor> MonitorFactory_SK :: CreateClientMonitor(const int oss_id) {
	return std::shared_ptr<ClientMonitor_SK>( new ClientMonitor_SK( oss_id ) );
}

std::shared_ptr<ServerMonitor> MonitorFactory_SK :: CreateServerMonitor(const int oss_id) {
	if ( server_monitor_.get() == NULL ) {
		pthread_rwlock_wrlock( &rwlock_ );
		if ( server_monitor_.get() == NULL ) {
			server_monitor_ = std::shared_ptr<ServerMonitor_SK>( new ServerMonitor_SK( oss_id ) );
		}
		pthread_rwlock_unlock( &rwlock_ );
	}
	return server_monitor_;
}

}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/phoenix_proj/trunk/phxrpc/plugin_sk/monitor/monitor_factory_sk.cpp $ $Id: monitor_factory_sk.cpp 1716024 2016-08-17 10:09:55Z mariohuang $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

