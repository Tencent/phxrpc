<img align="right" src="http://mmbiz.qpic.cn/mmbiz/UqFrHRLeCAkOcYOjaX3oxIxWicXVJY0ODsbAyPybxk4DkPAaibgdm7trm1MNiatqJYRpF034J7PlfwCz33mbNUkew/640?wx_fmt=jpeg&wxfrom=5&wx_lazy=1" hspace="15" width="150px" style="float: right">

**PhxRPC是微信后台团队推出的一个非常简洁小巧的RPC框架，编译生成的库只有450K。**

作者： Sifan Liu, Haochuan Cui 和 Duokai Huang

联系我们：phxteam@tencent.com

想了解更多, 以及更详细的编译手册，请进入[中文WIKI](https://github.com/Tencent/phxrpc/wiki)，和扫描右侧二维码关注我们的公众号

PhxRPC[![Build Status](https://travis-ci.org/Tencent/phxrpc.png)](https://travis-ci.org/Tencent/phxrpc)

## 总览

- 使用Protobuf作为IDL用于描述RPC接口以及通信数据结构。
- 基于Protobuf文件自动生成Client以及Server接口，用于Client的构建，以及Server的实现。
- 半同步半异步模式，采用独立多IO线程，通过Epoll管理请求的接入以及读写，工作线程采用固定线程池。IO线程与工作线程通过内存队列进行交互。
- 支持协程Worker，可配置多个线程，每个线程多个协程。
- 提供完善的过载保护，无需配置阈值，支持动态自适应拒绝请求。
- 提供简易的Client/Server配置读入方式。
- 基于lambda函数实现并发访问Server，可以非常方便地实现Google提出的 [Backup Requests](http://static.googleusercontent.com/media/research.google.com/zh-CN//people/jeff/Berkeley-Latency-Mar2012.pdf) 模式。

## 局限

- 不支持多进程模式。

## 性能

>使用Sample目录下的Search RPC C/S进行Echo RPC调用的压测，相当于Worker空转情况。

### 运行环境

CPU：24 x Intel(R) Xeon(R) CPU E5-2620 v3 @ 2.40GHz
内存：32 GB
网卡：千兆网卡
Client/Server机器之间PING值： 0.05ms
请求写入并发：1000个线程
业务数据大小：除去HTTP协议部分20b
Worker线程数：20

### 性能测试结果(qps)

#### 短连接

| ucontext类型/IO线程 | 1 | 3 | 8 | 20 |
| ----- | ---- | ---- | ---- | ---- |
| system  | 41k | 85k | 90k   | 92k |
| boost | 45k | 95k | 95k | 95k |

#### 长连接

| ucontext类型/IO线程 | 1 | 3 | 8 | 20 |
| ----- | ---- | ----- | --- | --- |
| system  | 55k | 160k   | 360k | 500k |
| boost | 62k | 175k | 410k | 500k |

## 如何编译

### Protobuf准备

PhxRPC必须依赖的第三方库只有Protobuf。在编译前，在`third_party`目录放置好`protobuf`目录，或者通过软链的形式。

### boost优化

PhxRPC在ServerIO以及Client并发连接管理上使用了ucontext，而boost的ucontext实现要比system默认的更为高效，推荐使用boost。如果需要使用boost的话需要在third_party目录放置好boost目录，或者通过软链的形式。

### 编译环境

- Linux
- GCC-4.8及以上版本
- boost 1.56及以上版本（可选）

### 编译安装方法

进入PhxRPC根目录。

```sh
make (默认是-O2编译，如需编译debug版，执行 make debug=y)
make boost (可选，编译PhxRPC的boost优化插件，编译之前先准备好boost库)
```

## 如何使用

### 编写proto文件

下面是sample目录下的proto文件样例。

```c++
syntax = "proto3";
package search;
import "google/protobuf/wrappers.proto";
import "google/protobuf/empty.proto";
import "phxrpc/phxrpc.proto";

enum SiteType {
    BLOG = 0;
    NEWS = 1;
    VIDEO = 2;
    UNKNOWN = 3;
}

message Site {
    string url = 1;
    string title = 2;
    SiteType type = 3;
    tring summary = 4;
}

message SearchRequest {
    string query = 1;
}

message SearchResult {
    repeated Site sites = 1;
}

service Search {
    rpc Search(SearchRequest) returns (SearchResult) {
        option(phxrpc.CmdID) = 1;
        option(phxrpc.OptString) = "q:";
        option(phxrpc.Usage) = "-q <query>";
    }
    rpc Notify(google.protobuf.StringValue) returns (google.protobuf.Empty) {
        option(phxrpc.CmdID) = 2;
        option(phxrpc.OptString) = "m:";
        option(phxrpc.Usage) = "-m <msg>";
    }
}

```

### 生成代码

```bash
(PhxRPC根目录)/codegen/phxrpc_pb2server -I (PhxRPC根目录) -I (Protobuf include目录) -f (proto文件路径) -d (生成代码放置路径)

# sample
../codegen/phxrpc_pb2server -I ../ -I ../third_party/protobuf/include -f search.proto -d .
../codegen/phxrpc_pb2server -I ../ -I ../third_party/protobuf/include -f search.proto -d . -u
```

两种生成模式，区别在于`-u`参数。

第一种生成默认的线程池worker模型。

第二种`-u`参数指定生成uthread worker模型，也就是工作线程池里面每个线程里面运行着多个协程。

调用完工具后，在生成代码放置目录下执行`make`，即可生成全部的RPC相关代码。

### 选择是否启用Boost优化

打开生成代码放置目录下的`Makefile`文件。

```bash
# choose to use boost for network
#LDFLAGS := $(PLUGIN_BOOST_LDFLAGS) $(LDFLAGS)
```

可以看到以上两行，取消注释掉第二行，重新`make clean && make`即可开启Boost对PhxRPC的优化。开启前记得编译好PhxRPC的Boost插件。

### 补充自己的代码

#### Server(xxx_service_impl.cpp)

```c++
int SearchServiceImpl::PHXEcho(const google::protobuf::StringValue &req,
        google::protobuf::StringValue *resp) {
    resp->set_value(req.value());
    return 0;
}

int SearchServiceImpl::Search(const search::SearchRequest &req,
        search::SearchResult *resp) {
    // 这里补充这个RPC调用的Server端代码
    return -1;
}

int SearchServiceImpl::Notify(const google::protobuf::StringValue &req,
        google::protobuf::Empty *resp) {
    // 这里补充这个RPC调用的Server端代码
    return -1;
}
```

#### Client (xxx_client.cpp)

```c++
// 这个是默认生成的代码，可自行修改，或利用我们提供的stub API自定义封装Client

int SearchClient::PHXEcho(const google::protobuf::StringValue &req,
        google::protobuf::StringValue *resp) {
    const phxrpc::Endpoint_t *ep = global_searchclient_config_.GetRandom();

    if (ep != nullptr) {
        phxrpc::BlockTcpStream socket;
        bool open_ret = phxrpc::PhxrpcTcpUtils::Open(&socket, ep->ip, ep->port,
                    global_searchclient_config_.GetConnectTimeoutMS(), nullptr, 0,
                    *(global_searchclient_monitor_.get()));
        if (open_ret) {
            socket.SetTimeout(global_searchclient_config_.GetSocketTimeoutMS());

            SearchStub stub(socket, *(global_searchclient_monitor_.get()));
            return stub.PHXEcho(req, resp);
        }
    }

    return -1;
}
```

#### UThread Client (xxx_client_uthread.cpp)

```c++
// 这个是默认生成的代码，可自行修改，或利用我们提供的stub API自定义封装Client
// UThread Client只能在采用PhxRPC uthread worker模型的server中调用。
// UThread Client构造函数需要传入UThreadEpollScheduler*类型参数，
// 这个参数来源可以在xxx_service_impl.h的私有变量中获得。

int SearchClientUThread::PHXEcho(const google::protobuf::StringValue &req,
        google::protobuf::StringValue *resp) {
    const phxrpc::Endpoint_t *ep = global_searchclientuthread_config_.GetRandom();

    if (uthread_scheduler_ != nullptr && ep != nullptr) {
        phxrpc::UThreadTcpStream socket;
        bool open_ret = phxrpc::PhxrpcTcpUtils::Open(uthread_scheduler_, &socket, ep->ip, ep->port,
                    global_searchclientuthread_config_.GetConnectTimeoutMS(),
                    *(global_searchclientuthread_monitor_.get()));
        if (open_ret) {
            socket.SetTimeout(global_searchclientuthread_config_.GetSocketTimeoutMS());

            SearchStub stub(socket, *(global_searchclientuthread_monitor_.get()));
            return stub.PHXEcho(req, resp);
        }
    }

    return -1;
}
```

#### Client并发调用样例

```c++
int SearchClient::PHXBatchEcho(const google::protobuf::StringValue &req,
        google::protobuf::StringValue *resp) {
    int ret = -1;
    size_t echo_server_count = 2;
    uthread_begin;
    for (size_t i = 0; i < echo_server_count; i++) {
        uthread_t [=, &uthread_s, &ret](void *) {
            const phxrpc::Endpoint_t *ep = global_searchclient_config_.GetByIndex(i);
            if (ep != nullptr) {
                phxrpc::UThreadTcpStream socket;
                if(phxrpc::PhxrpcTcpUtils::Open(&uthread_s, &socket, ep->ip, ep->port,
                            global_searchclient_config_.GetConnectTimeoutMS(),
                            *(global_searchclient_monitor_.get()))) {
                    socket.SetTimeout(global_searchclient_config_.GetSocketTimeoutMS());
                    SearchStub stub(socket, *(global_searchclient_monitor_.get()));
                    int this_ret = stub.PHXEcho(req, resp);
                    if (this_ret == 0) {
                        ret = this_ret;
                        uthread_s.Close();
                    }
                }
            }
        };
    }
    uthread_end;
    return ret;
}
```

`uthread_begin`, `uthread_end`, `uthread_s`, `uthread_t`这几个关键字是PhxRPC自定义的宏，分别表示协程的准备、结束，协程调度器以及协程的创建。

上面的代码实现了Google提出的 [Backup Requests](http://static.googleusercontent.com/media/research.google.com/zh-CN//people/jeff/Berkeley-Latency-Mar2012.pdf) 模式。实现的功能是分别对两个Server同时发起Echo调用，当有一个Server响应的时候，则整个函数结束。在这段代码里面，我们提供了一种异步IO的同步写法，并给予了一些方便使用的宏定义。首先使用`uthread_begin`进行准备，然后使用`uthread_t`以lambda的形式创建一个协程，而在任意一个协程里面都可使用我们PhxRPC生成的Client API进行RPC调用，并可使用`uthread_s`随时结束所有RPC调用。最后的`uthread_end`真正通过协程调度发起这些lambda内的RPC调用，并等待结束。

当然你可以借用这4个宏定义，以同步代码的写法，进行更自定义的并发访问。

#### Server配置说明 (xxx_server.conf)

```ini
[Server]
BindIP = 127.0.0.1              // Server IP
Port = 16161                    // Server Port
MaxThreads = 16                 // Worker 线程数
WorkerUThreadCount = 50         // 每个线程开启的协程数，采用-u生成的Server必须配置这一项
WorkerUThreadStackSize = 65536  // UThread worker的栈大小
IOThreadCount = 3               // IO线程数，针对业务请自行调节
PackageName = search            // Server 名字，用于自行实现的监控统计上报
MaxConnections = 800000         // 最大并发连接数
MaxQueueLength = 20480          // IO队列最大长度
FastRejectThresholdMS = 20      // 快速拒绝自适应调节阀值，建议保持默认20ms，不做修改

[ServerTimeout]
SocketTimeoutMS = 5000          // Server读写超时，Worker处理超时
```

