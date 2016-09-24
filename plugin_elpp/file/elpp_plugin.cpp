/*
Tencent is pleased to support the open source community by making 
PhxRPC available.
Copyright (C) 2016 THL A29 Limited, a Tencent company. 
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may 
not use this file except in compliance with the License. You may 
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
implied. See the License for the specific language governing 
permissions and limitations under the License.

See the AUTHORS file for names of contributors.
*/

#include "elpp_plugin.h"

#include "easyloggingpp/easylogging++.h"  
#include "phxrpc/file.h"

INITIALIZE_EASYLOGGINGPP  

namespace phxrpc {

static char global_logid_[ 128 ] = { "default" };
static time_t global_last_log_rotate_time_ = 0;

class ElppPlugingRegister {
public:
    ElppPlugingRegister() {
        phxrpc::setlog( elpp_openlog, elpp_closelog, elpp_vlog );
    }
};

static ElppPlugingRegister global_elpp_pluging_register_;

void elpp_openlog( const char * argv0, const char * log_dir, int priority )
{
    std::string filename( log_dir );
    filename += "/%datetime{%Y%M%d%H}.log";

    const char * pos = strrchr( argv0, '/' );
    if( NULL != pos ) {
        strncpy( global_logid_, pos + 1, sizeof( global_logid_ ) - 1 );
    } else {
        strncpy( global_logid_, argv0, sizeof( global_logid_ ) - 1 );
    }
    el::Logger * logger = el::Loggers::getLogger( global_logid_ );

    logger->configurations()->setGlobally( el::ConfigurationType::Filename, filename );
    logger->configurations()->setGlobally( el::ConfigurationType::ToStandardOutput, std::string("false") );
    logger->configurations()->setGlobally( el::ConfigurationType::MaxLogFileSize, std::string("0") );
    logger->configurations()->setGlobally( el::ConfigurationType::Format, std::string("%datetime %level [%logger] %msg") );

    logger->reconfigure();
    global_last_log_rotate_time_ = time( NULL ) / 3600 * 3600;
}

void elpp_closelog()
{
}

void elpp_vlog(int priority, const char * format, va_list args )
{
    time_t now_time = time( NULL );
    if( ( now_time - global_last_log_rotate_time_ ) >= 3600 ) {
        el::Logger * logger = el::Loggers::getLogger( global_logid_ );
        logger->reconfigure();
        global_last_log_rotate_time_ = now_time / 3600 * 3600;
    }

    char text[ 1024 ];
    vsnprintf( text, sizeof( text ), format, args );

    if( LOG_EMERG   == priority ) CLOG(FATAL, global_logid_) << text;
    if( LOG_ALERT   == priority || LOG_CRIT == priority || LOG_ERR == priority ) {
        CLOG(ERROR, global_logid_ ) << text;
    }
    if( LOG_WARNING == priority || LOG_NOTICE == priority ) {
        CLOG(WARNING, global_logid_) << text;
    }
    if( LOG_INFO    == priority ) CLOG(INFO, global_logid_) << text;
    if( LOG_DEBUG   == priority ) CLOG(DEBUG, global_logid_) << text;
}

void elpp_log(int priority, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    elpp_vlog(priority, format, args);
    va_end(args);
}

}

