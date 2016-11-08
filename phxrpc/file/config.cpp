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

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"
#include "file_utils.h"

namespace phxrpc {

Config :: Config() {
}

Config :: ~Config() {
}

bool Config::InitConfig(const char * path) {
    bool ret = FileUtils::ReadFile(path, &content_);
    if( ret ) content_.insert( 0, "\n" );
    return ret;
}

void Config::SetContent(const std::string & content) {
    content_.clear();
    content_.append( "\n" ).append( content );
}

bool Config::ReadItem(const char * section, const char * key, int * value) {
    char tmp[128] = { 0 };
    bool ret = ReadItem(section, key, tmp, sizeof(tmp));
    if (ret) {
        *value = atoi(tmp);
    }
    return ret;
}

bool Config::ReadItem(const char * section, const char * key, int * value, const int default_value) {
    char tmp[128] = { 0 };
    bool ret = ReadItem(section, key, tmp, sizeof(tmp));
    if (ret) {
        *value = atoi(tmp);
    } else {
        *value = default_value;
    }
    return ret;
}

bool Config::ReadItem(const char * section, const char * key, 
        char * value, size_t size, const char * default_value) {
    bool ret = ReadItem(section, key, value, size);
    if (!ret) {
        snprintf(value, size, "%s", default_value != nullptr ? default_value : "");
    }

    return ret;
}

bool Config::ReadItem(const char * section, const char * key, char * value, size_t size) {
    bool ret = false;

    char tmp_section[128] = { 0 };
    snprintf(tmp_section, sizeof(tmp_section), "\n[%s]", section);

    char tmp_key[128] = { 0 };
    snprintf(tmp_key, sizeof(tmp_key), "\n%s", key);

    const char * end_pos = NULL;
    const char * pos = strstr(content_.c_str(), tmp_section);
    if (NULL != pos) {
        pos = strchr(pos + 1, '\n');
        if (NULL == pos)
            pos = strchr(content_.c_str(), '\0');

        end_pos = strstr(pos, "\n[");
        if (NULL == end_pos)
            end_pos = strchr(pos, '\0');
    }

    for (; NULL != pos && pos < end_pos;) {
        pos = strstr(pos, tmp_key);

        if (NULL == pos || pos > end_pos)
            break;

        const char * tmp_pos = pos + strlen(tmp_key);
        if ((!isspace(*tmp_pos)) && ('=' != *tmp_pos))
            continue;

        pos++;

        const char * eol = strchr(pos, '\n');
        if (NULL == eol)
            eol = strchr(pos, '\0');

        tmp_pos = strchr(pos, '=');
        if (NULL != tmp_pos && tmp_pos < eol) {
            ret = true;

            for (tmp_pos++; tmp_pos <= eol && isspace(*tmp_pos);)
                tmp_pos++;

            for (size_t i = 0; tmp_pos <= eol && i < (size - 1); i++) {
                if (isspace(*tmp_pos))
                    break;
                *(value++) = *(tmp_pos++);
            }

            *value = '\0';

            break;
        }
    }

    return ret;
}

int Config::TrimCStr( char * src_str )
{
    int len = 0;
    char *pos = 0;

    len = strlen ( src_str ) ;
    while ( len > 0 && isspace( src_str [len - 1] ) )
        len--;
    src_str [len] = '\0' ;
    for ( pos = src_str; isspace(*pos); pos ++ )
        len--;
    if ( pos != src_str )
        memmove ( src_str, pos, len + 1 ) ;
    return 0;
}

bool Config::GetSection(const char * name,
        std::vector<std::string> * section) {

    char tmp_section[ 128 ] = { 0 };
    snprintf(tmp_section, sizeof( tmp_section ), "\n[%s]", name);

    char line[ 1024 ] = { 0 };

    const char * pos = strstr( content_.c_str(), tmp_section );
    if(!pos) {
        return false;
    } else {
        ++pos;
    }

    for( ; NULL != pos; )
    {
        pos = strchr( pos, '\n' );

        if( NULL == pos ) break;
        pos++;

        if( '[' == *pos ) break;

        if( ';' == *pos || '#' == *pos ) continue;

        strncpy( line, pos, sizeof( line ) - 1 );

        char * tmp_pos = strchr( line, '\n' );
        if( NULL != tmp_pos ) *tmp_pos = '\0';

        TrimCStr( line );

        if( '\0' != line[0] ) section->push_back( line );
    }
    return true;
}


}
