
include ../phxrpc.mk

LIB_OBJS = network/epoll-darwin.o

TARGETS = libphxrpc_plugin_darwin.a

all: $(TARGETS) 

libphxrpc_plugin_darwin.a: $(LIB_OBJS)
	$(AR) $@ $^
	mkdir -p $(PHXRPC_ROOT)/lib; cp $@ $(PHXRPC_ROOT)/lib 

clean:
	@( $(RM) $(TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )
