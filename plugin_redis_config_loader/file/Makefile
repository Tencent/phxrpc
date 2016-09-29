
include ../../phxrpc.mk

LIB_OBJS = redis_config_loader.o 

TARGETS = libphxrpc_plugin_redis_config_loader.a add_conf_tools

REDIS_CLIENT_INCLUDE = -I$(PHXRPC_ROOT)/third_party/hiredis \
  	-I$(PHXRPC_ROOT)/third_party/

REDIS_CLIENT_LIBRARY = -L$(PHXRPC_ROOT)/third_party/r3c/ -lr3c \
  	-L$(PHXRPC_ROOT)/third_party/hiredis -lhiredis

CFLAGS := -I$(PHXRPC_ROOT)/third_party $(REDIS_CLIENT_INCLUDE) $(CFLAGS)

LDFLAGS := -L$(PHXRPC_ROOT)/lib $(REDIS_CLIENT_LIBRARY) -lphxrpc $(LDFLAGS)

all: $(TARGETS) 

libphxrpc_plugin_redis_config_loader.a: $(LIB_OBJS)
	$(AR) $@ $^
	mkdir -p $(PHXRPC_ROOT)/lib; cp $@ $(PHXRPC_ROOT)/lib 

add_conf_tools: add_conf_tools.o 
	$(LINKER) $^  $(LDFLAGS) -o $@

clean:
	@( $(RM) $(TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )
