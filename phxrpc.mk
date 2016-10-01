#--------------------------------------------------------------------

where-am-i = $(abspath $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

#$(warning $(dir $(call where-am-i)))

PHXRPC_ROOT=$(dir $(call where-am-i))

OS := $(shell uname)

PROTOBUF_ROOT=$(PHXRPC_ROOT)/third_party/protobuf
BOOST_ROOT=$(PHXRPC_ROOT)/third_party/boost

ifeq ($(OS),Darwin)
	PLUGIN_BOOST_LDFLAGS = -L$(PHXRPC_ROOT)/lib/ -Wl,-force_load,libphxrpc_plugin_boost.a \
			-L$(BOOST_ROOT)/lib/ -lboost_context
	PLUGIN_ELPP_LDFLAGS = -L$(PHXRPC_ROOT)/lib/ -Wl,-force_load,libphxrpc_plugin_elpp.a
	PLUGIN_REDIS_CONFIG_LOADER_LDFLAGS = -L$(PHXRPC_ROOT)/lib/ -Wl,-force_load,libphxrpc_plugin_redis_config_loader.a
	PLUGIN_SK_MONITOR_LDFLAGS = -L$(PHXRPC_ROOT)/lib/ -Wl,-force_load,libphxrpc_plugin_monitor_sk.a
else
	PLUGIN_BOOST_LDFLAGS = -Wl,--whole-archive -L$(PHXRPC_ROOT)/lib/ -lphxrpc_plugin_boost \
			-Wl,--no-whole-archive -L$(BOOST_ROOT)/lib/ -lboost_context
	PLUGIN_ELPP_LDFLAGS = -Wl,--whole-archive -L$(PHXRPC_ROOT)/lib/ -lphxrpc_plugin_elpp \
			-Wl,--no-whole-archive
	PLUGIN_REDIS_CONFIG_LOADER_LDFLAGS = -Wl,--whole-archive -L$(PHXRPC_ROOT)/lib/ -lphxrpc_plugin_redis_config_loader \
			-Wl,--no-whole-archive
	PLUGIN_SK_MONITOR_LDFLAGS = -Wl,--whole-archive -L$(PHXRPC_ROOT)/lib/ -lphxrpc_plugin_monitor_sk \
			-Wl,--no-whole-archive
endif

#--------------------------------------------------------------------

ifeq ($(debug),y)
# (1) Debug
	OPT = -g2
else
# (2) Production
	OPT = -O2
endif

CC = gcc
AR = ar cru

SOFLAGS = -shared -Wl,-h,$@
LINKER = $(CC)
LINT = lint -c
RM = /bin/rm -f

CFLAGS = -std=c++11 -Wall -D_REENTRANT -D_GNU_SOURCE -D_XOPEN_SOURCE -fPIC -m64 $(OPT) \
		 -I$(PROTOBUF_ROOT)/include \
		 -I$(PHXRPC_ROOT) \

LDFLAGS = -L$(PROTOBUF_ROOT)/lib/ $(PROTOBUF_ROOT)/lib/libprotobuf.a \
		  -lstdc++ -lpthread -lm

PBFLAGS = -I $(PROTOBUF_ROOT)/include -I $(PHXRPC_ROOT)

#--------------------------------------------------------------------

# make rule
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@	

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@	

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@	
