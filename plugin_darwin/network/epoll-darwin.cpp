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

#include "epoll-darwin.h"

#include <cstdlib>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/event.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>

int epoll_create(int size)
{
	return kqueue();
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	struct kevent kev;
	if (op == EPOLL_CTL_ADD) {
		if (event->events == EPOLLIN) {
			EV_SET(&kev, fd, EVFILT_READ, EV_ADD, 0, 0, event->data.ptr);
		} else {
			EV_SET(&kev, fd, EVFILT_WRITE, EV_ADD, 0, 0, event->data.ptr);
		}
	} else if (op == EPOLL_CTL_DEL) {
		if (event->events == EPOLLIN) {
			EV_SET(&kev, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
		} else {
			EV_SET(&kev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
		}
	} else {
		errno = EINVAL;
		return -1;
	}

	return kevent(epfd, &kev, 1, nullptr, 0, nullptr);
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	struct kevent * evlist = (struct kevent*)malloc(sizeof(struct kevent)*maxevents);

	struct timespec to = {0, 0};
	if (timeout > 0) {
		to.tv_sec = timeout / 1000;
		to.tv_nsec = (timeout % 1000) * 1000 * 1000;
	}

	int ret = kevent(epfd, nullptr, 0, evlist, maxevents, timeout == -1 ? nullptr : &to);
	if (ret > 0) {
		for (int i = 0; i < ret; ++i) {
			events[i].events = ( evlist[i].filter == EVFILT_READ ) ? EPOLLIN : EPOLLOUT;
			events[i].data.ptr = evlist[i].udata;
		}
	}

	free( evlist );

	return ret;
}

