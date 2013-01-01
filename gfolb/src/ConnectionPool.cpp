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
 * ConnectionPool.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "ConnectionPool.h"

ConnectionPool* ConnectionPool::instance = NULL;

ConnectionPool::ConnectionPool() {
	logger = Logger::getLogger("ConnectionPool");
}

ConnectionPool::~ConnectionPool() {
}

void ConnectionPool::cleanUP()
{
	instance->cpmutex.lock();
	for (int var = 0; var < (int)instance->conns.size(); ++var)
	{
		instance->conns.at(var).client->closeConnection();
	}
	instance->cpmutex.unlock();
}

void* ConnectionPool::keepspawiningConnections(void* arg)
{
	try{
		bool secfld = false;
		while(true)
		{
			Thread::mSleep(10);
			if(instance->mode=="LB")
			{
				vector<int> backlog;
				set<int> backwhch;
				for (int var1 = 0; var1 < instance->siz; ++var1)
				{
					for (int var = var1*instance->tem;var < (int)(var1*instance->tem + instance->tem); ++var)
					{
						if(instance->conns.at(var).destroyed)
						{
							instance->logger << "respawned destroyed connection" << flush;

							instance->cpmutex.lock();
							instance->conns.at(var).client->closeConnection();
							instance->cpmutex.unlock();

							Connection conn(instance->cssl.at(var1));
							bool flag = conn.client->connection(instance->ip.at(var1),instance->port.at(var1));
							if(!flag)
							{
								backlog.push_back(var);
							}
							else
							{
								conn.host = instance->ip.at(var1) + (instance->port.at(var1)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(var1)):"");

								instance->cpmutex.lock();
								instance->conns[var] = conn;
								instance->cpmutex.unlock();
							}
						}
						else if(!instance->conns.at(var).client->isConnected())
						{
							Connection conn(instance->cssl.at(var));
							bool flag = conn.client->connection(instance->ip.at(var1),instance->port.at(var1));
							if(!flag)
							{
								backlog.push_back(var);
							}
							else
							{
								conn.host = instance->ip.at(var1) + (instance->port.at(var1)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(var1)):"");

								instance->cpmutex.lock();
								instance->conns[var] = conn;
								instance->cpmutex.unlock();
							}
						}
						else
						{
							backwhch.insert(var);
						}
					}
				}
				if(backwhch.size()>0 && backlog.size()>0)
				{
					set<int>::iterator it;
					int backeach = backlog.size()/backwhch.size();
					int backeachrem = backlog.size()%backwhch.size();
					int cnt = 0;
					for (it=backwhch.begin();it!=backwhch.end();++it,cnt++)
					{
						for (int var2 = cnt*backeach; var2 < cnt*backeach+backeach; ++var2)
						{
							Connection conn(instance->cssl.at(*it));
							bool flag = conn.client->connection(instance->ip.at(*it),instance->port.at(*it));
							if(flag)
							{
								conn.host = instance->ip.at(*it) + (instance->port.at(*it)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(*it)):"");

								instance->cpmutex.lock();
								instance->conns[var2] = conn;
								instance->cpmutex.unlock();
							}
							else
								break;
						}
					}
					for (it=backwhch.begin();it!=backwhch.end();++it,cnt++)
					{
						while(backeachrem>0)
						{
							Connection conn(instance->cssl.at(*it));
							bool flag = conn.client->connection(instance->ip.at(*it),instance->port.at(*it));
							if(flag)
							{
								conn.host = instance->ip.at(*it) + (instance->port.at(*it)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(*it)):"");

								instance->cpmutex.lock();
								instance->conns[cnt*backeach+backeachrem] = conn;
								instance->cpmutex.unlock();
							}
							else
								break;
							backeachrem--;
						}
					}
				}
			}
			else if(instance->mode=="FO")
			{
				if(instance->fonum==0)
				{
					instance->logger << "Now trying Primary again..";
					instance->logger << "Will quit if this fails..";
				}
				for (int var = 0; var < instance->num; ++var)
				{
					if(!instance->conns.at(var).client->isConnected())
					{
						Connection conn(instance->cssl.at(instance->fonum));
						bool flag = conn.client->connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
						if(!flag)
						{
							instance->logger << "Failover Secondary link failed...";

							instance->cpmutex.lock();
							instance->fonum = 0;
							instance->cpmutex.unlock();

							secfld = conn.client->connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
							if(secfld)
							{
								instance->logger << "Both links down will quit now...";
								cleanUP();
								exit(-1);
							}
							else
								break;
						}
						conn.host = instance->ip.at(instance->fonum) + (instance->port.at(instance->fonum)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(instance->fonum)):"");

						instance->cpmutex.lock();
						instance->conns[var] = conn;
						instance->cpmutex.unlock();
					}
				}
			}
		}
	}
	catch(...)
	{
		instance->logger << "exception occurred ";
	}
	return NULL;
}

void ConnectionPool::createPool(int num,vector<string> ipprts,bool persistent,string mode,int numretries)
{
	if(instance==NULL)
	{
		instance = new ConnectionPool();
		instance->mode = mode;
		instance->persi = persistent;
		instance->num = num;
		instance->fonum = 0;
		instance->onlineroute = 0;
		instance->numretries = numretries;
		for (int var = 0; var < (int)ipprts.size(); ++var)
		{
			string whl = ipprts.at(var);
			vector<string> ipdetails;
			StringUtil::split(ipdetails, whl, ":");
			if(ipdetails.size()!=3)
			{
				instance->logger << "Invalid endpoint configured....";
				continue;
			}
			bool ssl = false;
			if(ipdetails.at(0)=="ssl" || ipdetails.at(0)=="SSL")
				ssl = true;
			instance->cssl.push_back(ssl);
			instance->ip.push_back(ipdetails.at(1));
			try
			{
				int por = CastUtil::lexical_cast<int>(ipdetails.at(2));
				instance->port.push_back(por);
			}
			catch(...)
			{
				instance->logger << "invalid port specified";
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
					Connection conn(instance->cssl.at(var1));
					conn.client->connection(instance->ip.at(var1),instance->port.at(var1));
					if(!conn.client->isConnected())
						break;
					conn.destroyed = false;
					conn.busy = false;
					conn.host = instance->ip.at(var1) + (instance->port.at(var1)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(var1)):"");
					instance->conns.push_back(conn);
				}
			}
			if(instance->conns.size()==0)
			{
				instance->logger << "No servers available";
				exit(0);
			}
			instance->logger << "pool persistent" << flush;

			Thread keepspawiningConnections_thread(&keepspawiningConnections, NULL);
			keepspawiningConnections_thread.execute();
		}
		else
		{
			instance->logger << "pool not persistent" << flush;
		}
		instance->logger << "\ninitialised connection pool\n" << flush;
	}
	instance->logger << instance;
}

Connection* ConnectionPool::getConnection()
{
	Connection* conn = NULL;
	if(!instance->persi)
	{
		Connection* con = NULL;
		int tries = 0;
		string msg;
		if(instance->mode=="LB")
		{
			if(instance->fonum==instance->siz)
			{
				instance->cpmutex.lock();
				instance->fonum = 0;
				instance->cpmutex.unlock();
			}
			instance->logger << instance->fonum;
			con = new Connection(instance->cssl.at(instance->fonum));
			con->host = instance->ip.at(instance->fonum) + (instance->port.at(instance->fonum)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(instance->fonum)):"");
			con->client->connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));

			instance->cpmutex.lock();
			instance->fonum++;
			instance->cpmutex.unlock();

			msg = "no servers available";
		}
		else if(instance->mode=="FO")
		{
			con = new Connection(instance->cssl.at(instance->fonum));
			con->client->connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
			con->host = instance->ip.at(instance->fonum) + (instance->port.at(instance->fonum)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(instance->fonum)):"");
			msg = "no servers available to switch over to";
		}
		else if(instance->mode=="OR")
		{
			instance->logger << instance->onlineroute;
			if(instance->onlineroute>=instance->siz)
			{
				instance->cpmutex.lock();
				instance->onlineroute = 0;
				instance->cpmutex.unlock();
			}
			con = new Connection(instance->cssl.at(instance->onlineroute));
			con->client->connection(instance->ip.at(instance->onlineroute),instance->port.at(instance->onlineroute));
			con->host = instance->ip.at(instance->onlineroute) + (instance->port.at(instance->onlineroute)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(instance->onlineroute)):"");
			return con;
		}
		while(con!=NULL && !con->client->isConnected())
		{
			delete con;
			if(instance->mode=="FO")
			{
				instance->cpmutex.lock();
				instance->fonum++;
				instance->cpmutex.unlock();
			}
			if(instance->fonum==instance->siz)
			{
				tries++;
				instance->cpmutex.lock();
				instance->fonum = 0;
				instance->cpmutex.unlock();
				if(tries==instance->numretries)
				{
					instance->logger << msg;
					exit(0);
				}
			}
			con = new Connection(instance->cssl.at(instance->fonum));
			con->client->connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
			con->host = instance->ip.at(instance->fonum) + (instance->port.at(instance->fonum)!=80?":"+CastUtil::lexical_cast<string>(instance->port.at(instance->fonum)):"");
			if(instance->mode=="LB")
			{
				instance->cpmutex.lock();
				instance->fonum++;
				instance->cpmutex.unlock();
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
						&& instance->conns[var].client->isConnected())
				{
					instance->cpmutex.lock();
					instance->conns[var].busy = true;
					instance->cpmutex.unlock();
					conn = &(instance->conns[var]);
					break;
				}
				else if(!instance->conns[var].client->isConnected())
				{
					instance->cpmutex.lock();
					instance->conns[var].destroyed = true;
					instance->cpmutex.unlock();
				}
			}
			Thread::mSleep(1);
		}
	}
	return conn;
}

string ConnectionPool::validate(vector<string> cmd)
{
	if(cmd.at(1)=="CR")
	{
		try
		{
			int nwrt = CastUtil::lexical_cast<int>(cmd.at(2));
			if(instance->siz==1)
			{
				string t =  "INVALID ROUTE - ONLY A SINGLE ROUTE EXISTS";			
				return t;
			}
			if(instance->siz==0)
			{
				string t =  "NO ROUTE EXISTS";	
				return t;
			}
			if(nwrt>instance->siz-1)
			{
				string t =  "INVALID ROUTE - SELECT ROUTE FROM 0-" + CastUtil::lexical_cast<string>(instance->siz-1);
				return t;
			}
			instance->cpmutex.lock();
			instance->onlineroute = nwrt;
			instance->cpmutex.unlock();
			instance->logger << "changed route " << nwrt;
		}
		catch(...)
		{
			return "INVALID ARGUMENT";
		}
	}
	else
	{
		string address = cmd.at(2);
		string ip;
		int por = 80;
		bool ssl = false;
		if(cmd.at(1)!="DR")
		{
			vector<string> ipdetails;
			StringUtil::split(ipdetails, address, ":");
			if(ipdetails.size()!=3)
			{
				instance->logger << "invalid endpoint details";
				return "INVALID ENDPOINT DETAILS";
			}
			if(ipdetails.at(0)=="ssl" || ipdetails.at(0)=="SSL")
				ssl = true;
			ip = ipdetails.at(1);
			try
			{
				por = CastUtil::lexical_cast<int>(ipdetails.at(2));
			}
			catch(...)
			{
				instance->logger << "invalid port specified";
				return "INVALID PORT SPECIFIED";
			}
			char *rip = ClientInterface::get_ip((char*)ip.c_str());
			if(rip==NULL)
			{
				instance->logger << "invalid host specified";
				return "INVALID HOST SPECIFIED";
			}
			ip = "";
			ip.append(rip);
		}
		if(cmd.at(1)=="AR")
		{
			instance->cpmutex.lock();
			instance->ip.push_back(ip);
			instance->port.push_back(por);
			instance->cssl.push_back(ssl);
			instance->siz = instance->ip.size();
			instance->cpmutex.unlock();
		}
		else if(cmd.at(1)=="UR" || cmd.at(1)=="DR")
		{
			int ind = 2;
			if(cmd.at(1)=="UR")
			{
				if(cmd.size()!=4)
				{
					instance->logger << "4 arguments expected";
					return "ARGUMENT MISSING - EXPECTED 4";
				}
				ind = 3;
			}
			
			int index = -1;
			try
			{
				index = CastUtil::lexical_cast<int>(cmd.at(ind));
			}
			catch(...)
			{
				instance->logger << "invalid update index specified";
				return "INVALID UPDATE INDEX SPECIFIED";
			}
			if(index>instance->siz-1)
			{
				instance->logger << "update index out of range";
				return "UPDATE INDEX OUT OF RANGE";
			}
			instance->cpmutex.lock();
			if(cmd.at(1)=="UR")
			{
				instance->ip.at(index) = ip;
				instance->port.at(index) = por;
				instance->cssl.at(index) = ssl;
			}
			else
			{
				instance->ip.erase(instance->ip.begin()+index);
				instance->port.erase(instance->port.begin()+index);
				instance->cssl.erase(instance->cssl.begin()+index);
				instance->siz = instance->ip.size();
			}
			instance->cpmutex.unlock();
		}
	}
	return "COMMAND SUCCESSFULL";
}

void ConnectionPool::release(Connection *conn)
{
	if(!instance->persi)
	{
		delete conn;
	}
	else
	{
		conn->free();
	}
}

bool ConnectionPool::isPersistent()
{
	return instance->persi;
}

Connection::Connection(bool isSSL)
{
	if(isSSL)
	{
		client = new SSLClient();
	}
	else
	{
		client = new Client();
	}
}

Connection::~Connection()
{
	client->closeConnection();
	delete client;
}
