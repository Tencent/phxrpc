
set -e  # exit immediately on error
set -x  # display all commands

mkdir -p etc
mkdir -p sbin
mkdir -p third_party
cp ./Makefile.third_party third_party/Makefile

cd third_party;

if [ ! -f ../sbin/redis-server ]; then

    if [ ! -f 3.2.3.tar.gz ]; then
        wget https://github.com/antirez/redis/archive/3.2.3.tar.gz
    fi

    tar zxvf 3.2.3.tar.gz
    cd redis-3.2.3

    make
    cp src/redis-server ../../sbin/
    cp redis.conf ../../etc/

    cd ..
fi

if [ ! -f hiredis/libhiredis.a ]; then

    if [ ! -f hiredis/Makefile ]; then
        git clone https://github.com/redis/hiredis.git
    fi

    cd hiredis

    make static

    cd ..
fi

if [ ! -f r3c/libr3c.a ]; then
    if [ ! -f r3c/r3c.cpp ]; then
        git clone https://github.com/eyjian/r3c.git
    fi

    make
fi

if [ ! -f protobuf/bin/protoc ]; then
	if [ ! -f protobuf-cpp-3.0.0.tar.gz ]; then
		wget https://github.com/google/protobuf/releases/download/v3.0.0/protobuf-cpp-3.0.0.tar.gz
	fi	

	tar zxvf protobuf-cpp-3.0.0.tar.gz
	cd protobuf-3.0.0

	./configure --prefix=`pwd`/../protobuf
	make -j2
	make install

	cd ../
fi

if [ ! -f easyloggingpp/easylogging++.h ]; then
    if [ ! -f 9.84.tar.gz ]; then
        wget https://github.com/easylogging/easyloggingpp/archive/9.84.tar.gz
    fi

    tar zxvf 9.84.tar.gz

    mkdir -p easyloggingpp

    cp easyloggingpp-9.84/src/easylogging++.h  easyloggingpp
fi

cd ..

make

exit $?
