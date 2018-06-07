debug = y
include ../phxrpc.mk

COMM_OBJS = syntax_tree.o proto_utils.o name_render.o code_utils.o

TARGETS = phxrpc_pb2service phxrpc_pb2client phxrpc_pb2tool phxrpc_pb2server

TEST_TARGETS = test_proto_utils

all: $(TARGETS) $(TEST_TARGETS)

phxrpc_pb2service: $(COMM_OBJS) service_code_render.o phxrpc_pb2service.o
	$(LINKER) $^ $(LDFLAGS) -o $@

phxrpc_pb2client: $(COMM_OBJS) client_code_render.o client_template.o phxrpc_pb2client.o
	$(LINKER) $^ $(LDFLAGS) -o $@

phxrpc_pb2tool: $(COMM_OBJS) tool_code_render.o tool_template.o phxrpc_pb2tool.o
	$(LINKER) $^ $(LDFLAGS) -o $@

phxrpc_pb2server: $(COMM_OBJS) server_code_render.o server_template.o phxrpc_pb2server.o
	$(LINKER) $^ $(LDFLAGS) -o $@

test_proto_utils: test_proto_utils.o syntax_tree.o proto_utils.o code_utils.o
	$(LINKER) $^ $(LDFLAGS) -o $@


clean:
	@($(RM) $(TARGETS) $(TEST_TARGETS))
	@($(RM) *.o)
