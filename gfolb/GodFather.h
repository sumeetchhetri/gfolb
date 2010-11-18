/*
 * GodFather.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef GODFATHER_H_
#define GODFATHER_H_
#include "IConnectionHandler.h"
#include "string"
#include "vector"
#include "map"
using namespace std;
#include <algorithm>
#include <cstdlib>
#include <dlfcn.h>
#include "sstream"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <boost/thread/recursive_mutex.hpp>
#include <queue>
#include <sys/uio.h>
#include <sys/un.h>
#include <stdexcept>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>
#include "string"
#include <sstream>
#include <typeinfo>
#define MAXEPOLLSIZE 100
#define BACKLOGM 500
#define MAXBUFLENM 32768
#define PORT "9992"
#define NUM_PROC 2
class GodFather {
public:
	GodFather();
	virtual ~GodFather();
};

#endif /* GODFATHER_H_ */
