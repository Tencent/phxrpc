
cd third_party;

test -f protobuf-cpp-3.0.0.tar.gz || wget https://github.com/google/protobuf/releases/download/v3.0.2/protobuf-cpp-3.0.0.tar.gz

tar zxvf protobuf-cpp-3.0.0.tar.gz

cd protobuf-3.0.0

./configure --prefix=`pwd`/../protobuf
make
make install

cd ../..

make

