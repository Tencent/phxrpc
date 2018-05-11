version = 0.8.0

all:
	@( cd phxrpc; make )
	@( cd codegen; make )
	@( cd sample; test -f Makefile || ./regen.sh; make )

boost:
	@( cd plugin_boost; make )

dist: clean phxrpc-$(version).src.tar.gz

phxrpc-$(version).src.tar.gz:
	@find . -type f | grep -v CVS | grep -v .svn | grep -v third_party | sed s:^./:phxrpc-$(version)/: > MANIFEST
	@(cd ..; ln -s phxrpc phxrpc-$(version))
	(cd ..; tar cvf - `cat phxrpc/MANIFEST` | gzip > phxrpc/phxrpc-$(version).src.tar.gz)
	@(cd ..; rm phxrpc-$(version))

clean:
	@( rm -rf lib/*; )
	@( cd phxrpc; make clean )
	@( cd codegen; make clean )
	@( cd sample; test -f Makefile && make clean )
	@( cd plugin_boost; make clean )
	@( cd plugin_darwin; make clean )

