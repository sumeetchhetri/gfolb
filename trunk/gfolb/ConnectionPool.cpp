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

void ConnectionPool::keepspawiningConnections(ConnectionPool *pool)
{
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		for (int var1 = 0; var1 < (int)instance->ip.size(); ++var1)
		{
			for (int var = 0; var < pool->num; ++var)
			{
				if(pool->conns.at(var).destroyed)
				{
					cout << "respawned destroyed connection" << flush;
					pool->conns.at(var).client.closeConnection();
					Connection conn;
					Client client;
					client.connection(pool->ip.at(var1),pool->port.at(var1));
					conn.client = client;
					pool->conns[var] = conn;
				}
			}
		}
	}
}
