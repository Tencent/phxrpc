test -f Makefile && make clean

rm -f *.h *.cc *.cpp Makefile *.conf

../codegen/phxrpc_pb2server -I ../ -I ../third_party/protobuf/include -f search.proto -d .


