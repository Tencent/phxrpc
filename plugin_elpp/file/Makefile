
include ../../phxrpc.mk

LIB_OBJS = elpp_plugin.o

TARGETS = libphxrpc_plugin_elpp.a test_elpp_plugin

CFLAGS := -DELPP_NO_DEFAULT_LOG_FILE \
	-DELPP_DISABLE_DEFAULT_CRASH_HANDLING \
	-DELPP_THREAD_SAFE \
	-I$(PHXRPC_ROOT)/third_party $(CFLAGS)

LDFLAGS := -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS)

all: $(TARGETS) 

libphxrpc_plugin_elpp.a: $(LIB_OBJS)
	$(AR) $@ $^
	mkdir -p $(PHXRPC_ROOT)/lib; cp $@ $(PHXRPC_ROOT)/lib 

test_elpp_plugin: test_elpp_plugin.o
	$(LINKER) -o $@ $^ $(PLUGIN_ELPP_LDFLAGS) $(LDFLAGS)

clean:
	@( $(RM) $(TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )
