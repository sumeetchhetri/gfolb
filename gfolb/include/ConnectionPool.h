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
 * ConnectionPool.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_
#include "Client.h"
#include "SSLClient.h"
#include "vector"
#include "set"
#include "Mutex.h"
#include "Thread.h"
#include "fstream"
#include "CastUtil.h"
#include "Logger.h"
using namespace std;
class Connection
{
	Connection(bool);
	bool busy;
	friend class ConnectionPool;
public:
	virtual ~Connection();
	string host;
	bool destroyed;
	ClientInterface* client;
	void free()
	{
		busy = false;
	}
};

class ConnectionPool {
	Logger logger;
	ConnectionPool();
	static void* keepspawiningConnections(void* arg);
	static void cleanUP();
	vector<Connection> conns;
	static ConnectionPool* instance;
	vector<string> ip;
	vector<int> port;
	vector<bool> cssl;
	string mode;
	int siz;
	int num,tem;
	int fonum;
	bool persi,sock_blkg;
	int onlineroute, numretries;
	Mutex cpmutex;
public:
	static void createPool(int num,vector<string> ipprts,bool persistent,string mode,int numretries);
	static Connection* getConnection();
	virtual ~ConnectionPool();
	static string validate(vector<string> cmd);
	static void release(Connection *conn);
	static bool isPersistent();
};

#endif /* CONNECTIONPOOL_H_ */
