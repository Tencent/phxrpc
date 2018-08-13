/*
Tencent is pleased to support the open source community by making
PhxRPC available.
Copyright (C) 2016 THL A29 Limited, a Tencent company.
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may
not use this file except in compliance with the License. You may
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.

See the AUTHORS file for names of contributors.
*/

const char *PHXRPC_EPOLL_SERVER_MAIN_TEMPLATE =
        R"(

#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "phxrpc/comm.h"
#include "phxrpc/file.h"
#include "phxrpc/http.h"
#include "phxrpc/msg.h"
#include "phxrpc/rpc.h"

#include "$DispatcherFile$.h"
#include "$ServiceImplFile$.h"
#include "$ServerConfigFile$.h"


using namespace std;


void Dispatch(const phxrpc::BaseRequest &req,
              phxrpc::BaseResponse *const resp,
              phxrpc::DispatcherArgs_t *const args) {
    ServiceArgs_t *service_args{(ServiceArgs_t *)(args->service_args)};

    $ServiceImplClass$ service(*service_args);
    $DispatcherClass$ dispatcher(service, args);

    phxrpc::BaseDispatcher<$DispatcherClass$> base_dispatcher(
            dispatcher, $DispatcherClass$::GetURIFuncMap());
    if (!base_dispatcher.Dispatch(req, resp)) {
        resp->SetFake(phxrpc::BaseResponse::FakeReason::DISPATCH_ERROR);
    }
}

void ShowUsage(const char *program) {
    printf("\n");
    printf("Usage: %s [-c <config>] [-d] [-l <log level>] [-v]\n", program);
    printf("\n");

    exit(0);
}

int main(int argc, char **argv) {
    const char *config_file{nullptr};
    bool daemonize{false};
    int log_level{-1};
    extern char *optarg;
    int c;
    while (EOF != (c = getopt(argc, argv, "c:vl:d"))) {
        switch (c) {
            case 'c': config_file = optarg; break;
            case 'd': daemonize = true; break;
            case 'l': log_level = atoi(optarg); break;

            case 'v':
            default: ShowUsage(argv[0]); break;
        }
    }

    if (daemonize) phxrpc::ServerUtils::Daemonize();

    PHXRPC_ASSERT(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    // set customize log / monitor
    //phxrpc::setlog(openlog, closelog, vlog);
    //phxrpc::MonitorFactory::SetFactory(new YourMonitorFactory());

    if (nullptr == config_file) ShowUsage(argv[0]);

    $ServerConfigClass$ config;
    if (!config.Read(config_file)) ShowUsage(argv[0]);

    if (log_level > 0) config.GetHshaServerConfig().SetLogLevel(log_level);

    phxrpc::openlog(argv[0], config.GetHshaServerConfig().GetLogDir(),
                    config.GetHshaServerConfig().GetLogLevel());

    ServiceArgs_t service_args;
    service_args.config = &config;

    phxrpc::HshaServer server(config.GetHshaServerConfig(), Dispatch, &service_args);
    server.RunForever();

    phxrpc::closelog();

    return 0;
}

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_EPOLL_UTHREAD_SERVER_MAIN_TEMPLATE =
        R"(

#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "phxrpc/comm.h"
#include "phxrpc/file.h"
#include "phxrpc/http.h"
#include "phxrpc/msg.h"
#include "phxrpc/rpc.h"

#include "$DispatcherFile$.h"
#include "$ServiceImplFile$.h"
#include "$ServerConfigFile$.h"


using namespace std;


void Dispatch(const phxrpc::BaseRequest &req,
              phxrpc::BaseResponse *const resp,
              phxrpc::DispatcherArgs_t *const args) {
    ServiceArgs_t *service_args{(ServiceArgs_t *)(args->service_args)};

    $ServiceImplClass$ service(*service_args, args->server_worker_uthread_scheduler);
    $DispatcherClass$ dispatcher(service, args);

    phxrpc::BaseDispatcher<$DispatcherClass$> base_dispatcher(
            dispatcher, $DispatcherClass$::GetURIFuncMap());
    if (!base_dispatcher.Dispatch(req, resp)) {
        resp->SetFake(phxrpc::BaseResponse::FakeReason::DISPATCH_ERROR);
    }
}

void ShowUsage(const char *program) {
    printf("\n");
    printf("Usage: %s [-c <config>] [-d] [-l <log level>] [-v]\n", program);
    printf("\n");

    exit(0);
}

int main(int argc, char **argv) {
    const char *config_file{nullptr};
    bool daemonize{false};
    int log_level{-1};
    extern char *optarg;
    int c;
    while (EOF != (c = getopt(argc, argv, "c:vl:d"))) {
        switch (c) {
            case 'c': config_file = optarg; break;
            case 'd': daemonize = true; break;
            case 'l': log_level = atoi(optarg); break;

            case 'v':
            default: ShowUsage(argv[0]); break;
        }
    }

    if (daemonize) phxrpc::ServerUtils::Daemonize();

    PHXRPC_ASSERT(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    // set customize log / monitor
    //phxrpc::setlog(openlog, closelog, vlog);
    //phxrpc::MonitorFactory::SetFactory(new YourMonitorFactory());

    if (nullptr == config_file) ShowUsage(argv[0]);

    $ServerConfigClass$ config;
    if (!config.Read(config_file)) ShowUsage(argv[0]);

    if (log_level > 0) config.GetHshaServerConfig().SetLogLevel(log_level);

    phxrpc::openlog(argv[0], config.GetHshaServerConfig().GetLogDir(),
                    config.GetHshaServerConfig().GetLogLevel());

    ServiceArgs_t service_args;
    service_args.config = &config;

    phxrpc::HshaServer server(config.GetHshaServerConfig(), Dispatch, &service_args);
    server.RunForever();

    phxrpc::closelog();

    return 0;
}

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_EPOLL_SERVER_CONFIG_HPP_TEMPLATE =
        R"(

#include "phxrpc/rpc.h"


class $ServerConfigClass$ {
  public:
    $ServerConfigClass$();

    virtual ~$ServerConfigClass$();

    bool Read(const char *config_file);

    phxrpc::HshaServerConfig &GetHshaServerConfig();

  private:
    phxrpc::HshaServerConfig ep_server_config_;
};

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_EPOLL_SERVER_CONFIG_CPP_TEMPLATE =
        R"(

#include "$ServerConfigFile$.h"

#include "$MessageFile$.h"


$ServerConfigClass$::$ServerConfigClass$() {
}

$ServerConfigClass$::~$ServerConfigClass$() {
}

bool $ServerConfigClass$::Read(const char *config_file) {
    bool ret{ep_server_config_.Read(config_file)};

    if (0 == strlen(ep_server_config_.GetPackageName())) {
        ep_server_config_.SetPackageName($PackageNameExpression$);
    }

    return ret;
}

phxrpc::HshaServerConfig &$ServerConfigClass$::GetHshaServerConfig() {
    return ep_server_config_;
}

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_EPOLL_SERVER_ETC_TEMPLATE =
        R"(

[Server]
BindIP = 127.0.0.1
Port = 16161
MaxThreads = 16
IOThreadCount = 3
PackageName = $PbPackageName$
MaxConnections = 800000
MaxQueueLength = 20480
FastRejectThresholdMS = 20
FastRejectAdjustRate = 5

[Log]
LogDir = ~/log
LogLevel = 3

[ServerTimeout]
SocketTimeoutMS = 5000

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_EPOLL_UTHREAD_SERVER_ETC_TEMPLATE =
        R"(

[Server]
BindIP = 127.0.0.1
Port = 16161
MaxThreads = 16
WorkerUThreadCount = 50
WorkerUThreadStackSize = 65536
IOThreadCount = 3
PackageName = $PbPackageName$
MaxConnections = 800000
MaxQueueLength = 20480
FastRejectThresholdMS = 20
FastRejectAdjustRate = 5

[Log]
LogDir = ~/log
LogLevel = 3

[ServerTimeout]
SocketTimeoutMS = 5000

)";

//////////////////////////////////////////////////////////////////////

const char *PHXRPC_SERVER_MAKEFILE_TEMPLATE =
        R"(

include $PhxRPCMKDir$/phxrpc.mk

LDFLAGS := -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS)

# choose to use boost for network
#LDFLAGS := $(PLUGIN_BOOST_LDFLAGS) $(LDFLAGS)

SVR_OBJS = $MessageFile$.o \
		$ServiceImplFile$.o \
		$ServiceFile$.o \
		$DispatcherFile$.o \
		$ServerConfigFile$.o \
		$ServerMainFile$.o

CLI_OBJS = $MessageFile$.o \
		$ClientFile$.o \
		$StubFile$.o

TARGETS = lib$ClientFile$.a $ServerMainFile$ $ToolMainFile$

all: $(TARGETS)

$ServerMainFile$: $(SVR_OBJS)
	$(LINKER) $^ $(LDFLAGS) -o $@

lib$ClientFile$.a: $(CLI_OBJS)
	$(AR) $@ $^

$ToolMainFile$: $ToolFile$.o $ToolImplFile$.o $ToolMainFile$.o
	$(LINKER) $^ -L. -l$ClientFile$ $(LDFLAGS) -o $@

########## message ##########

$MessageFile$.cc: $MessageFile$.h

$MessageFile$.h: $ProtoFile$
	$(PROTOBUF_ROOT)/bin/protoc -I$(PROTOBUF_ROOT)/include --cpp_out=. -I$(PHXRPC_ROOT) -I. $^

########## client ##########

$StubFile$.cpp: $StubFile$.h
$StubFile$.o: $StubFile$.h
$ClientFile$.cpp: $StubFile$.h
$ClientFile$.o: $StubFile$.h

$StubFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2client $(PBFLAGS) -f $^ -d .

########## service ##########

$ServiceFile$.cpp: $ServiceFile$.h
$ServiceFile$.o: $ServiceFile$.h
$ServiceImplFile$.cpp: $ServiceFile$.h
$ServiceImplFile$.o: $ServiceFile$.h
$DispatcherFile$.cpp: $ServiceFile$.h
$DispatcherFile$.o: $ServiceFile$.h

$ServiceFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2service $(PBFLAGS) -f $^ -d .

########## tool ##########

$ToolFile$.cpp: $ToolFile$.h
$ToolFile$.o: $ToolFile$.h
$ToolImplFile$.cpp: $ToolFile$.h
$ToolImplFile$.o: $ToolFile$.h
$ToolMainFile$.cpp: $ToolFile$.h
$ToolMainFile$.o: $ToolFile$.h

$ToolFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2tool $(PBFLAGS) -f $^ -d .

clean:
	@($(RM) $(TARGETS))
	@($(RM) *.o)
	@($(RM) phxrpc_*)
	@($(RM) *.pb.*)

)";

/////////////////////////////////////////////////////////////////////

const char *PHXRPC_UTHREAD_SERVER_MAKEFILE_TEMPLATE =
        R"(

include $PhxRPCMKDir$/phxrpc.mk

LDFLAGS := -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS)

# choose to use boost for network
#LDFLAGS := $(PLUGIN_BOOST_LDFLAGS) $(LDFLAGS)

SVR_OBJS = $MessageFile$.o \
		$ServiceImplFile$.o \
		$ServiceFile$.o \
		$DispatcherFile$.o \
		$ServerConfigFile$.o \
		$ServerMainFile$.o

CLI_OBJS = $MessageFile$.o \
		$ClientFile$.o \
		$ClientFile$_uthread.o \
		$StubFile$.o

TARGETS = lib$ClientFile$.a $ServerMainFile$ $ToolMainFile$

all: $(TARGETS)

$ServerMainFile$: $(SVR_OBJS)
	$(LINKER) $^ $(LDFLAGS) -o $@

lib$ClientFile$.a: $(CLI_OBJS)
	$(AR) $@ $^

$ToolMainFile$: $ToolFile$.o $ToolImplFile$.o $ToolMainFile$.o
	$(LINKER) $^ -L. -l$ClientFile$ $(LDFLAGS) -o $@

########## message ##########

$MessageFile$.cc: $MessageFile$.h

$MessageFile$.h: $ProtoFile$
	$(PROTOBUF_ROOT)/bin/protoc -I$(PROTOBUF_ROOT)/include --cpp_out=. -I$(PHXRPC_ROOT) -I. $^

########## client ##########

$StubFile$.cpp: $StubFile$.h
$StubFile$.o: $StubFile$.h
$ClientFile$.cpp: $StubFile$.h
$ClientFile$.o: $StubFile$.h
$ClientFile$_uthread.cpp: $StubFile$.h
$ClientFile$_uthread.o: $StubFile$.h

$StubFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2client $(PBFLAGS) -f $^ -d . -u

########## service ##########

$ServiceFile$.cpp: $ServiceFile$.h
$ServiceFile$.o: $ServiceFile$.h
$ServiceImplFile$.cpp: $ServiceFile$.h
$ServiceImplFile$.o: $ServiceFile$.h
$DispatcherFile$.cpp: $ServiceFile$.h
$DispatcherFile$.o: $ServiceFile$.h

$ServiceFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2service $(PBFLAGS) -f $^ -d . -u

########## tool ##########

$ToolFile$.cpp: $ToolFile$.h
$ToolFile$.o: $ToolFile$.h
$ToolImplFile$.cpp: $ToolFile$.h
$ToolImplFile$.o: $ToolFile$.h
$ToolMainFile$.cpp: $ToolFile$.h
$ToolMainFile$.o: $ToolFile$.h

$ToolFile$.h: $ProtoFile$
	$(PHXRPC_ROOT)/codegen/phxrpc_pb2tool $(PBFLAGS) -f $^ -d .

clean:
	@($(RM) $(TARGETS))
	@($(RM) *.o)
	@($(RM) phxrpc_*)
	@($(RM) *.pb.*)

)";

