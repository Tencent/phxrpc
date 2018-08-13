include ../../phxrpc.mk

TEST_TARGETS = test_thread_queue test_hsha_server test_client

all: $(TEST_TARGETS)

test_thread_queue: test_thread_queue.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

test_hsha_server: test_hsha_server.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

test_client: test_client.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

clean:
	@( $(RM) $(TEST_TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )

