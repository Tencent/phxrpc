
include ../../phxrpc.mk

LIB_OBJS = iOssAttrApi.o iOssAttr4Svr.o client_monitor_sk.o server_monitor_sk.o monitor_factory_sk.o

TARGETS = libphxrpc_plugin_monitor_sk.a 

CFLAGS := $(CFLAGS)

LDFLAGS := -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS)

all: $(TARGETS) 

libphxrpc_plugin_monitor_sk.a: $(LIB_OBJS)
	$(AR) $@ $^
	mkdir -p $(PHXRPC_ROOT)/lib; cp $@ $(PHXRPC_ROOT)/lib 


clean:
	@( $(RM) $(TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )
