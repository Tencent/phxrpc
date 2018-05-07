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

#include "file_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "log_utils.h"


namespace phxrpc {


using namespace std;


bool FileUtils::ReadFile(const char *path, string *content) {
    char newpath[1024]{0};
    if ('~' == path[0]) {
        snprintf(newpath, sizeof(newpath), "%s%s", getenv("HOME"), path + 1);
    } else {
        snprintf(newpath, sizeof(newpath), "%s", path);
    }

    bool ret = false;

    int fd = ::open(newpath, O_RDONLY);
    if (fd >= 0) {
        struct stat file_stat;
        if (0 == fstat(fd, &file_stat)) {
            content->resize(file_stat.st_size);

            if (read(fd, (char *) content->data(), file_stat.st_size) == file_stat.st_size) {
                ret = true;
            } else {
                phxrpc::log(LOG_ERR, "WARN: read( ..., %llu ) fail, errno %d, %s",
                            (unsigned long long) file_stat.st_size, errno, strerror(errno));
            }
        } else {
            phxrpc::log(LOG_ERR, "WARN: stat %s fail, errno %d, %s", newpath, errno, strerror(errno));
        }

        close(fd);
    } else {
        phxrpc::log(LOG_ERR, "WARN: open %s fail, errno %d, %s", newpath, errno, strerror(errno));
    }

    return ret;
}

void FileUtils::StrSplitList(const string &str, const string &delimiters,
                             vector<string> &results) {
    results.clear();
    auto last(0);
    auto found(str.find_first_of(delimiters));
    while (string::npos != found) {
        auto r(str.substr(last, found - last));
        last = found + 1;
        found = str.find_first_of(delimiters, last);
        if (!r.empty()) results.push_back(r);
    }
    auto r(str.substr(last));
    if (!r.empty()) results.push_back(r);
}


}

