
include ../phxrpc.mk

LIB_OBJS = network/uthread_context_boost.o 

TARGETS = libphxrpc_plugin_boost.a

CFLAGS := -I$(BOOST_ROOT) $(CFLAGS)

all: $(TARGETS) 

libphxrpc_plugin_boost.a: $(LIB_OBJS)
	$(AR) $@ $^
	mkdir -p $(PHXRPC_ROOT)/lib; cp $@ $(PHXRPC_ROOT)/lib 

clean:
	@( $(RM) $(TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )
