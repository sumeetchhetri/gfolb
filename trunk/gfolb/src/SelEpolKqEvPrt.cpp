/*
	Copyright 2009-2013, Sumeet Chhetri

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */
/*
 * SelEpolKqEvPrt.cpp
 *
 *  Created on: 30-Dec-2012
 *      Author: sumeetc
 */

#include "SelEpolKqEvPrt.h"

SelEpolKqEvPrt::SelEpolKqEvPrt() {
	logger = Logger::getLogger("SelEpolKqEvPrt");
}

SelEpolKqEvPrt::~SelEpolKqEvPrt() {
}

void SelEpolKqEvPrt::initialize(int sockfd)
{
	this->sockfd = sockfd;
	#ifdef USE_SELECT
		fdmax = sockfd;        // maximum file descriptor number

		FD_ZERO(&master);    // clear the master and temp sets
		FD_ZERO(&read_fds);

		FD_SET(this->sockfd, &master);
	#endif
	#ifdef USE_EPOLL
		curfds = 1;
		epoll_handle = epoll_create(MAXDESCRIPTORS);
		ev.events = EPOLLIN | EPOLLPRI;
		ev.data.fd = this->sockfd;

		if (epoll_ctl(epoll_handle, EPOLL_CTL_ADD, sockfd, &ev) < 0)
		{
			perror("epoll set insertion error");
			return;
		}
	#endif
	#ifdef USE_KQUEUE
		curfds = 1;
		kq = kqueue();
		if (kq == -1)
			perror("kqueue");

		EV_SET(&change, sockfd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, 0);
		kevent(kq, &change, 1, NULL, 0, NULL);
	#endif
	#ifdef USE_EVPORT
		if ((port = port_create()) < 0) {
			perror("port_create");
		}
		if (port_associate(port, PORT_SOURCE_FD, sockfd, POLLIN, NULL) < 0) {
			perror("port_associate");
		}
	#endif
}


int SelEpolKqEvPrt::getEvents()
{
	int numEvents = -1;
	#ifdef USE_SELECT
		read_fds = master;
		numEvents = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		if(numEvents==-1)
		{
			perror("select()");
		}
		else
		{
			return fdmax+1;
		}
	#endif
	#ifdef USE_EPOLL
		numEvents = epoll_wait(epoll_handle, events, curfds,-1);
	#endif
	#ifdef USE_KQUEUE
		numEvents = kevent(kq, NULL, 0, evlist, curfds, NULL);
	#endif
	#ifdef USE_EVPORT
		uint_t num = 1;
		port_getn(port, evlist, (uint_t) MAXDESCRIPTORS, (uint_t *) &num, NULL);
		numEvents = (int)num;
	#endif
	return numEvents;
}

int SelEpolKqEvPrt::getDescriptor(int index)
{
	#ifdef USE_SELECT
		if(FD_ISSET(index, &read_fds))
		{
			return index;
		}
	#endif
	#ifdef USE_EPOLL
		if(index>-1 && index<sizeof events)
		{
			return events[index].data.fd;
		}
	#endif
	#ifdef USE_KQUEUE
		if(index>-1 && index<sizeof evlist)
		{
			return evlist[index].ident;
		}
	#endif
	#ifdef USE_EVPORT
		if(evlist[index].portev_source == PORT_SOURCE_FD)
		{
			reassociateListener = true;
			return (int)evlist[index].portev_object;
		}
	#endif
	return -1;
}

bool SelEpolKqEvPrt::isListeningDescriptor(int descriptor)
{
	if(descriptor==sockfd)
	{
		return true;
	}
	return false;
}

bool SelEpolKqEvPrt::registerForEvent(int descriptor)
{
	fcntl(descriptor, F_SETFL, fcntl(descriptor, F_GETFD, 0) | O_NONBLOCK);
	#ifdef USE_SELECT
		FD_SET(descriptor, &master); // add to master set
		if (descriptor > fdmax) {    // keep track of the max
			fdmax = descriptor;
		}
	#endif
	#ifdef USE_EPOLL
		curfds++;
		ev.events = EPOLLIN | EPOLLPRI;
		ev.data.fd = descriptor;
		if (epoll_ctl(epoll_handle, EPOLL_CTL_ADD, descriptor, &ev) < 0)
		{
			perror("epoll");
			logger << "\nerror adding to epoll cntl list" << flush;
			return false;
		}
	#endif
	#ifdef USE_KQUEUE
		curfds++;
		EV_SET(&change, descriptor, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, 0);
		kevent(kq, &change, 1, NULL, 0, NULL);
	#endif
	#ifdef USE_EVPORT
		if (port_associate(port, PORT_SOURCE_FD, descriptor, POLLIN, NULL) < 0) {
			perror("port_associate");
		}
		if(reassociateListener)
		{
			if (port_associate(port, PORT_SOURCE_FD, sockfd, POLLIN, NULL) < 0) {
				perror("port_associate");
			}
			reassociateListener = false;
		}
	#endif
	return true;
}

bool SelEpolKqEvPrt::unRegisterForEvent(int descriptor)
{
	#ifdef USE_SELECT
		FD_CLR(descriptor, &master);
	#endif
	#ifdef USE_EPOLL
		curfds--;
		epoll_ctl(epoll_handle, EPOLL_CTL_DEL, descriptor, &ev);
	#endif
	#ifdef USE_KQUEUE
		curfds--;
		EV_SET(&change, descriptor, EVFILT_READ, EV_DELETE, 0, 0, 0);
		kevent(kq, &change, 1, NULL, 0, NULL);
	#endif
	#ifdef USE_EVPORT
		if (port_dissociate(port, PORT_SOURCE_FD, descriptor) < 0) {
			perror("port_dissociate");
		}
	#endif
	return true;
}
