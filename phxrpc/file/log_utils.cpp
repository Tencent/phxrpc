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

#include "log_utils.h"

#include <cstdio>
#include <cstdlib>


namespace phxrpc {


static openlog_t global_openlog_ = NULL;
static closelog_t global_closelog_ = ::closelog;
static vlog_t global_vlog_ = vsyslog;
static int global_priority_ = LOG_ERR;

void openlog(const char *argv0, const char *log_dir, int priority) {

    char new_path[ 1024 ] = { 0 };
    if( '~' == log_dir[0] ) {
        snprintf( new_path, sizeof( new_path ), "%s%s", getenv( "HOME" ), log_dir + 1 );
    } else {
        snprintf( new_path, sizeof( new_path ), "%s", log_dir );
    }

    global_priority_ = priority;
    if( NULL != global_openlog_ ) {
        global_openlog_( argv0, new_path, priority );
    } else {
        ::openlog( argv0, LOG_CONS | LOG_PID, priority );
    }
}

void closelog() {
    global_closelog_();
}

void log(int priority, const char *format, ...) {

    if( priority > global_priority_ ) return;

    va_list args;
    va_start(args, format);
    global_vlog_(priority, format, args);
    va_end(args);
}

void setvlog(vlog_t vlog) {
    global_vlog_ = vlog;
}

void setlog(openlog_t open_log, closelog_t close_log, vlog_t vlog) {
    global_openlog_ = open_log;
    global_closelog_ = close_log;
    global_vlog_ = vlog;
}


}  // namespace phxrpc

