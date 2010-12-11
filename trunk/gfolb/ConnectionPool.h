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
#include "set"
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
	static void keepspawiningConnections();
	static void cleanUP();
	vector<Connection> conns;
	static ConnectionPool* instance;
	vector<string> ip;
	vector<int> port;
	string mode;
	int siz;
	int num,tem;
	int fonum;
	bool persi;
public:
	/*static void createPool(int num,string ip,int port,bool persistent)
	{
		if(instance==NULL)
		{
			instance = new ConnectionPool();
			instance->fonum = 1;
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
				boost::thread g_thread(boost::bind(&keepspawiningConnections));
				cout << "pool not persistent" << flush;
			}
			cout << "\ninitialised connectikon pool\n" << flush;
		}
	}*/
	static void createPool(int num,vector<string> ipprts,bool persistent,string mode)
	{

		if(instance==NULL)
		{
			instance = new ConnectionPool();
			instance->mode = mode;
			instance->persi = persistent;
			instance->num = num;
			instance->fonum = 0;
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
			instance->tem = num;
			instance->num = num;
			instance->siz = 1;
			if(mode=="LB")
			{
				instance->tem = num/(int)instance->ip.size();
				instance->num = instance->tem*(int)instance->ip.size();
				instance->siz = (int)instance->ip.size();
			}
			if(persistent)
			{
				for (int var1 = 0; var1 < (int)instance->siz; ++var1)
				{
					for (int var = 0; var < instance->tem; ++var)
					{
						Connection conn;
						Client client;
						client.connection(instance->ip.at(var1),instance->port.at(var1));
						if(!client.isConnected())
							break;
						conn.client = client;
						conn.destroyed = false;
						conn.busy = false;
						instance->conns.push_back(conn);
					}
				}
				if(instance->conns.size()==0)
				{
					cout << "No servers available" << endl;
					exit(0);
				}
				cout << "pool persistent" << flush;
				boost::thread g_thread(boost::bind(&keepspawiningConnections));
			}
			else
			{
				cout << "pool not persistent" << flush;
			}
			cout << "\ninitialised connection pool\n" << flush;
		}
	}
	static Connection* getConnection()
	{
		Connection* conn = NULL;
		if(!instance->persi)
		{
			Connection* con = new Connection;
			Client client;
			client.connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
			con->client = client;
			instance->fonum++;
			if(instance->fonum==instance->siz)
				instance->fonum = 0;
			return con;
		}
		else
		{
			while(conn==NULL)
			{
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
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}
		return conn;
	}
	virtual ~ConnectionPool();
	static void release(Connection *conn)
	{
		if(!instance->persi)
		{
			conn->~Connection();
			delete conn;
		}
		else
		{
			conn->free();
			conn->destroyed = true;
		}
	}
};

#endif /* CONNECTIONPOOL_H_ */
