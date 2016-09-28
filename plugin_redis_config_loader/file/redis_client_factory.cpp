
#include "redis_client_factory.h"

#include "phxrpc/rpc.h"
#include "phxrpc/file.h"

#include "r3c/r3c.h"


namespace phxrpc {

RedisClientFactory * RedisClientFactory::GetDefault() {
    static RedisClientFactory factory;
    return &factory;
}

RedisClientFactory :: RedisClientFactory()
{
    is_init_ = false;
    config_path_ = std::string("/home/qspace/etc/minichat/client/redis_client.conf");
}

RedisClientFactory :: ~RedisClientFactory()
{

}


int RedisClientFactory :: Init() 
{
    if(is_init_) {
        return 0;
    }

    phxrpc::ClientConfig config;

    if( config.Read( config_path_.c_str() ) ) {
        char buff[ 128 ] = { 0 };

        for( size_t i = 0; ; i++ ) {
            const phxrpc::Endpoint_t * ep = config.GetByIndex( i );

            if( NULL == ep ) break;
            snprintf( buff, sizeof( buff ), "%s:%d", ep->ip, ep->port );

            if( i > 0 ) nodes_.append( "," );
            nodes_.append( buff );
        }
        is_init_ = true;
        return 0;
    } else {
        return -1;
    }
}

r3c::CRedisClient * RedisClientFactory :: Get()
{
    if(0 != Init()) {
        return NULL;
    }

    static __thread r3c::CRedisClient * client = NULL;
    
    if( NULL == client ) client = new r3c::CRedisClient( nodes_ );

    return client;
}

}

