
#pragma once

#include <string>

namespace r3c {
    class CRedisClient;
};

namespace phxrpc {

class RedisClientFactory {
public:

    static RedisClientFactory * GetDefault();

    RedisClientFactory();
    ~RedisClientFactory();

    int Init(const char * config_file);

    r3c::CRedisClient * Get();

private:
    bool is_init_;
    std::string nodes_;
};

}

