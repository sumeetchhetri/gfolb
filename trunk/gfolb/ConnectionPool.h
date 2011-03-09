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
#include "fstream"
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
	int onlineroute;
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
			instance->onlineroute = 0;
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
			instance->siz = (int)instance->ip.size();
			if(mode=="LB")
			{
				instance->tem = num/(int)instance->ip.size();
				instance->num = instance->tem*(int)instance->ip.size();
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
		cout << instance << endl;
	}
	static Connection* getConnection()
	{
		Connection* conn = NULL;
		if(!instance->persi)
		{
			Connection* con = new Connection;
			int tries = 0;
			string msg;
			if(instance->mode=="LB")
			{
				con->client.connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
				instance->fonum++;
				if(instance->fonum==instance->siz)
					instance->fonum = 0;
				msg = "no servers available";
			}
			else if(instance->mode=="FO")
			{
				con->client.connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
				msg = "no servers available to switch over to";
			}
			else if(instance->mode=="OR")
			{
				ifstream ifs("o_data");
				if(ifs.is_open())
				{
					string temp;
					getline(ifs,temp);
					try
					{
						int nwrt = boost::lexical_cast<int>(temp);
						instance->onlineroute = nwrt;
						cout << "got the changed route " << nwrt << endl;
					}
					catch(...)
					{
					}
					remove("o_data");
				}
				cout << instance->onlineroute << endl;
				con->client.connection(instance->ip.at(instance->onlineroute),instance->port.at(instance->onlineroute));
				return con;
			}
			while(!con->client.isConnected())
			{
				if(instance->mode=="FO")instance->fonum++;
				if(instance->fonum==instance->siz)
				{
					tries++;
					instance->fonum = 0;
					if(tries==3)
					{
						cout << msg << endl;
						exit(0);
					}
				}
				con->client.connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
				if(instance->mode=="LB")
				{
					instance->fonum++;
					if(instance->fonum==instance->siz)
						instance->fonum = 0;
				}
			}
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
						 //cout << "returned not null conn" << endl;
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
	static string validate(vector<string> cmd)
	{
		try
		{
			cout << instance << endl;
			int nwrt = boost::lexical_cast<int>(cmd.at(2));
			ofstream ofs("o_data");
			ofs.write(cmd.at(2).c_str(),cmd.at(2).length());
			ofs.close();
			cout << "changed route " << nwrt << endl;
		}
		catch(...)
		{
			return "INVALID ARGUMENT";
		}
		return "COMMAND SUCCESSFULL";
	}
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
			//conn->destroyed = true;
		}
	}
	static bool isPersistent()
	{
		return instance->persi;
	}
};

#endif /* CONNECTIONPOOL_H_ */
