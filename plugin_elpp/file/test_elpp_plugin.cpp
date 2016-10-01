
#include <stdio.h>
#include <unistd.h>

#include "phxrpc/file.h"

#include "easyloggingpp/easylogging++.h"  

#include "elpp_plugin.h"

void test0( const char * argv0 )
{
    phxrpc::openlog( argv0, "~/log", 7 );

    phxrpc::log( LOG_INFO, "hello test0" );

    for( int i = 0; i < 20; i++ ) {
        phxrpc::log( LOG_DEBUG, "%d", i );
        sleep( 4 );
    }

    phxrpc::closelog();
}

void test1( const char * argv0 )
{
    char log_id[ 128 ];

    const char * pos = strrchr( argv0, '/' );
    if( NULL != pos ) {
        strncpy( log_id, pos + 1, sizeof( log_id ) );
    } else {
        strncpy( log_id, argv0, sizeof( log_id ) );
    }

    std::string filename( "log" );
    filename += "/%datetime{%Y%M%d%H}.log";

    el::Logger * logger = el::Loggers::getLogger( log_id );

    logger->configurations()->setGlobally( el::ConfigurationType::Filename, filename );
    logger->configurations()->setGlobally( el::ConfigurationType::ToStandardOutput, std::string("false") );
    logger->reconfigure();
    
    CLOG(INFO, log_id) << "hello test1";
}

void test2( const char * argv0 )
{
    phxrpc::elpp_openlog( argv0, "log", 7 );

    phxrpc::elpp_log( LOG_INFO, "hello test2" );

    phxrpc::elpp_closelog();
}

int main(int argc, const char** argv)   
{
    if( argc < 2 ) {
        printf( "Usage: %s <type>\n", argv[0] );
        return -1;
    }

    int type = atoi( argv[1] );

    if( 0 == type ) test0( argv[0] );
    if( 1 == type ) test1( argv[0] );
    if( 2 == type ) test2( argv[0] );

    return 0;  
} 

