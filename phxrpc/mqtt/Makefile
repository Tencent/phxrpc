include ../../phxrpc.mk

TEST_TARGETS = test_mqtt_protocol test_mqtt_client

all: $(TEST_TARGETS)

test_mqtt_protocol: test_mqtt_protocol.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

test_mqtt_client: test_mqtt_client.o
	$(LINKER) $^ -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS) -o $@

clean:
	@( $(RM) $(TEST_TARGETS) )
	@( $(RM) *.o core.* $(LIB_OBJS) )

