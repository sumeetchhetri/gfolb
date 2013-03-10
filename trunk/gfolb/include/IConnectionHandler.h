/*
	Copyright 2010, Sumeet Chhetri

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
 * IConnectionHandler.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef ICONNECTIONHANDLER_H_
#define ICONNECTIONHANDLER_H_
#include "BufferedReader.h"
#include "ConnectionPool.h"
#include "Logger.h"
#include "Task.h"
#include <fcntl.h>
#include <dlfcn.h>
class IConnectionHandler {
	Logger logger;
	propMap props;
	map<int,bool> fds;
	static void* handleRequests(void* arg);
	static void* handleReleaseConnections(void* arg);
	static void* service(void* arg);
	string mode;
	Mutex qmutex;
	void* dlib;
public:
	BufferedReader *reader;
	IConnectionHandler(vector<string> ipps,bool persistent,int poolsize,propMap props,bool isSSL,SSL_CTX *ctx);
	virtual ~IConnectionHandler();
	void add(int);
	bool bind(int);
	bool unbind(int);
};


class RequestProp :public Task{
public:
	int fd;
	IConnectionHandler *handler;
	string data;
	void run();
};

#endif /* ICONNECTIONHANDLER_H_ */
