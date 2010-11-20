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
#include "Client.h"
#include <fcntl.h>
class IConnectionHandler {
	BufferedReader *reader;
	propMap props;
	map<int,bool> fds;
	static void handle(IConnectionHandler* handler);
	void service(int fd,string data);
	string mode;
public:
	//IConnectionHandler(string ip,int port,bool persistent,int poolsize);
	IConnectionHandler(vector<string> ipps,bool persistent,int poolsize,propMap props);
	virtual ~IConnectionHandler();
	void add(int);
	bool bind(int);
	bool unbind(int);
};

#endif /* ICONNECTIONHANDLER_H_ */
