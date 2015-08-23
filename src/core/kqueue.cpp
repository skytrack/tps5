//******************************************************************************
//
// File Name : kqueue.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifdef _MSC_VER

#include <stdlib.h>
#include <map>
#include <time.h>
#include <ws2tcpip.h>
#include "kqueue.h"
#include "log.h"

std::map<uintptr_t, struct kevent> timers;
std::map<uintptr_t, struct kevent> socks;
std::map<uintptr_t, struct kevent> writes;

static time_t timer;

int kqueue(void)
{
	timer = time(NULL);
	return 0;
}

int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout)
{
	for (int i = 0; i < nchanges; i++) {
		if (changelist[i].flags == EV_ADD)
		{
			if (changelist[i].filter == EVFILT_READ) {
				socks[changelist[i].ident] = changelist[i];
			}
			if (changelist[i].filter == EVFILT_WRITE) {
				writes[changelist[i].ident] = changelist[i];
			}
			if (changelist[i].filter == EVFILT_TIMER) {
				timers[changelist[i].ident] = changelist[i];
			}
		}
		if (changelist[i].flags == EV_DELETE)
		{
			if (changelist[i].filter == EVFILT_READ) {
				socks.erase(changelist[i].ident);
			}
			if (changelist[i].filter == EVFILT_WRITE) {
				writes.erase(changelist[i].ident);
			}
			if (changelist[i].filter == EVFILT_TIMER) {
				timers.erase(changelist[i].ident);
			}
		}
	}

	if (nevents == 0)
		return 0;

	fd_set 			fdReadSet;
	fd_set 			fdWriteSet;
	int 			nfds, status;
	struct timeval 		tv;

	for (;;) {

		nfds = 0;

		FD_ZERO(&fdReadSet);

		for (std::map<uintptr_t, struct kevent>::iterator it=socks.begin(); it!=socks.end(); ++it) {
			FD_SET(it->second.ident, &fdReadSet);
			if (it->second.ident > nfds)
				nfds = it->second.ident;
		}	

		FD_ZERO(&fdWriteSet);

		for (std::map<uintptr_t, struct kevent>::iterator it=writes.begin(); it!=writes.end(); ++it) {
			FD_SET(it->second.ident, &fdWriteSet);
			if (it->second.ident > nfds)
				nfds = it->second.ident;
		}	

		tv.tv_sec	= 1;
		tv.tv_usec	= 0;

		int c = 0;
		status = select(nfds + 1, &fdReadSet, &fdWriteSet, NULL, &tv);

		if (status == -1) {
			errno = WSAGetLastError();
			return -1;
		}

		if (status > 0) {

			for (std::map<uintptr_t, struct kevent>::iterator it=socks.begin(); it!=socks.end(); ++it) {
				if (FD_ISSET(it->second.ident, &fdReadSet)) {
					*eventlist = it->second;
					eventlist->data = 1;
					eventlist++;
					c++;
				}
			}
			for (std::map<uintptr_t, struct kevent>::iterator it=writes.begin(); it!=writes.end(); ++it) {
				if (FD_ISSET(it->second.ident, &fdWriteSet)) {
					*eventlist = it->second;
					eventlist->data = 1;
					eventlist++;
					c++;
				}
			}
		}

		for (std::map<uintptr_t, struct kevent>::iterator it=timers.begin(); it!=timers.end(); ++it) {
			if ((it->second.data == 999) && (timer != time(NULL))) {
				*eventlist = it->second;
				eventlist->data = 1;
				eventlist++;
				c++;
				timer = time(NULL);
			}
		}	

		if (c > 0)
			return c;
	}

	return 0;
}

#endif