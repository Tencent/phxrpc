include ../../phxrpc.mk

TEST_TARGETS = test_http_client

all: $(TEST_TARGETS)

test_http_client: test_http_client.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

clean:
	@( $(RM) $(TEST_TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )

