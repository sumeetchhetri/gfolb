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
#include "vector"
#include <boost/thread/thread.hpp>
using namespace std;
class Connection
{
	bool busy;
	friend class ConnectionPool;
public:
	bool destroyed;
	Client client;
	void free()
	{
		busy = false;
	}
};



class ConnectionPool {
	ConnectionPool();
	static void keepspawiningConnections(ConnectionPool *pool);
	vector<Connection> conns;
	static ConnectionPool* instance;
	vector<string> ip;
	vector<int> port;
	int num;
public:
	static void createPool(int num,string ip,int port,bool persistent)
	{
		if(instance==NULL)
		{
			instance = new ConnectionPool();
			instance->num = num;
			instance->ip.push_back(ip);
			instance->port.push_back(port);
			for (int var = 0; var < num; ++var)
			{
				Connection conn;
				Client client;
				client.connection(ip,port);
				conn.client = client;
				conn.destroyed = false;
				conn.busy = false;
				instance->conns.push_back(conn);
			}
			if(!persistent)
			{
				boost::thread g_thread(boost::bind(&keepspawiningConnections,instance));
				cout << "pool not persistent" << flush;
			}
			cout << "\ninitialised connectikon pool\n" << flush;
		}
	}
	static void createPool(int num,vector<string> ipprts,bool persistent)
	{
		if(instance==NULL)
		{
			instance = new ConnectionPool();
			instance->num = num;
			for (int var = 0; var < (int)ipprts.size(); ++var)
			{
				string whl = ipprts.at(var);
				instance->ip.push_back(whl.substr(0,whl.find(":")));

				try
				{
					int por = boost::lexical_cast<int>(whl.substr(whl.find(":")+1));
					instance->port.push_back(por);
				}
				catch(...)
				{
					cout << "invalid port specified" << endl;
					exit(-1);
				}
			}
			int tem = num/(int)instance->ip.size();
			instance->num = tem*(int)instance->ip.size();
			for (int var1 = 0; var1 < (int)instance->ip.size(); ++var1)
			{
				for (int var = 0; var < tem; ++var)
				{
					Connection conn;
					Client client;
					client.connection(instance->ip.at(var1),instance->port.at(var1));
					conn.client = client;
					conn.destroyed = false;
					conn.busy = false;
					instance->conns.push_back(conn);
				}
			}
			if(!persistent)
			{
				boost::thread g_thread(boost::bind(&keepspawiningConnections,instance));
				cout << "pool not persistent" << flush;
			}
			cout << "\ninitialised connection pool\n" << flush;
		}
	}
	static Connection* getConnection()
	{
		Connection* conn = NULL;
		for (int var = 0; var < (int)instance->conns.size(); ++var)
		{
			if(!instance->conns[var].busy && !instance->conns[var].destroyed
					&& instance->conns[var].client.isConnected())
			{
				 instance->conns[var].busy = true;
				 conn = &(instance->conns[var]);
				 cout << "returned not null conn" << endl;
				 break;
			}
			else if(!instance->conns[var].client.isConnected())
			{
				instance->conns[var].destroyed = true;
			}
		}
		if(conn==NULL)
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		return conn;
	}
	virtual ~ConnectionPool();
};

#endif /* CONNECTIONPOOL_H_ */
