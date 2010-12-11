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
	// TODO Auto-generated constructor stub

}

ConnectionPool::~ConnectionPool() {
	// TODO Auto-generated destructor stub
}

void ConnectionPool::cleanUP()
{
	for (int var = 0; var < (int)instance->conns.size(); ++var)
	{
		instance->conns.at(var).client.closeConnection();
	}
}

void ConnectionPool::keepspawiningConnections()
{try{
	bool secfld = false;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		if(instance->mode=="LB")
		{
			vector<int> backlog;
			set<int> backwhch;
			for (int var1 = 0; var1 < instance->siz; ++var1)
			{
				for (int var = var1*instance->tem;var < var1*instance->tem + instance->tem,instance->conns.size()>var; ++var)
				{
					if(instance->conns.at(var).destroyed)
					{
						cout << "respawned destroyed connection" << flush;
						instance->conns.at(var).client.closeConnection();
						Connection conn;
						Client client;
						bool flag = client.connection(instance->ip.at(var1),instance->port.at(var1));
						if(!flag)
						{
							backlog.push_back(var);
						}
						else
						{
							conn.client = client;
							instance->conns[var] = conn;
						}
					}
					else if(!instance->conns.at(var).client.isConnected())
					{
						Connection conn;
						Client client;
						bool flag = client.connection(instance->ip.at(var1),instance->port.at(var1));
						if(!flag)
						{
							backlog.push_back(var);
						}
						else
						{
							conn.client = client;
							instance->conns[var] = conn;
						}
					}
					else
					{
						backwhch.insert(var1);
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
						Connection conn;
						Client client;
						bool flag = client.connection(instance->ip.at(*it),instance->port.at(*it));
						if(flag)
						{
							conn.client = client;
							instance->conns[var2] = conn;
						}
						else
							break;
					}
				}
				for (it=backwhch.begin();it!=backwhch.end();++it,cnt++)
				{
					while(backeachrem>0)
					{
						Connection conn;
						Client client;
						bool flag = client.connection(instance->ip.at(*it),instance->port.at(*it));
						if(flag)
						{
							conn.client = client;
							instance->conns[cnt*backeach+backeachrem] = conn;
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
				cout << "Now trying Primary again.." << endl;
				cout << "Will quit if this fails.." << endl;
			}
			for (int var = 0; var < instance->num; ++var)
			{
				if(!instance->conns.at(var).client.isConnected())
				{
					Connection conn;
					Client client;
					bool flag = client.connection(instance->ip.at(instance->fonum),instance->port.at(instance->fonum));
					if(!flag)
					{
						cout << "Failover Secondary link failed..." << endl;
						instance->fonum = 0;
						secfld = true;
						if(secfld)
						{
							cout << "Both links down will quit now..." << endl;
							cleanUP();
							exit(-1);
						}
						else
							break;
					}
					conn.client = client;
					instance->conns[var] = conn;
				}
			}
		}
	}}catch(...){cout << "exception occurred " << endl;}
}
