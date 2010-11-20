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
 * IConnectionHandler.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "IConnectionHandler.h"

void IConnectionHandler::handle(IConnectionHandler* handler)
{
	handler->reader = new BufferedReader(handler->props);
	map<int,bool>::iterator it;
	vector<int> fdss;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		for(it=handler->fds.begin();it!=handler->fds.end();it++)
		{
			if(!handler->reader->q[it->first].empty())
			{
				string data = handler->reader->q[it->first].front();
				handler->reader->q[it->first].pop();
				if(data=="")
					boost::this_thread::sleep(boost::posix_time::milliseconds(10));
				else
				{
					handler->service(it->first,data);
				}
			}
			else if(handler->reader->done[it->first])
			{
				fdss.push_back(it->first);
			}
		}
		for(int i=0;i<(int)fdss.size();i++)
		{
			handler->unbind(fdss.at(i));
			handler->fds.erase(fdss.at(i));
			handler->reader->done.erase(fdss.at(i));
			handler->reader->fds.erase(fdss.at(i));
			close(fdss.at(i));
			cout << "connection closed\n" << flush;
		}
		fdss.clear();
	}
}

void IConnectionHandler::service(int fd,string data)
{
	Connection *conn = ConnectionPool::getConnection();
	Client client = conn->client;
	int bytes = client.sendData(data);
	string call,tot;
	while((call=client.getData())!="")
		tot.append(call);
	bytes = send(fd,tot.c_str(),tot.length(), 0);
	this->reader->done[fd] = true;
	conn->free();
	conn->destroyed = true;
	cout << "done with request" << flush;
}

IConnectionHandler::IConnectionHandler(string ip,int port,bool persistent,int poolsize)
{
	boost::thread m_thread(boost::bind(&handle,this));
	ConnectionPool::createPool(poolsize,ip,port,persistent);
}

IConnectionHandler::IConnectionHandler(vector<string> ipps,bool persistent,int poolsize,propMap props)
{
	this->props = props;
	boost::thread m_thread(boost::bind(&handle,this));
	ConnectionPool::createPool(poolsize,ipps,persistent);
}

void IConnectionHandler::add(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	this->fds[fd] = true;
	this->reader->fds[fd] = true;
	this->reader->done[fd] = false;
	bind(fd);
}

IConnectionHandler::~IConnectionHandler() {
	// TODO Auto-generated destructor stub
}

bool IConnectionHandler::bind(int fd)
{
	return true;
}
bool IConnectionHandler::unbind(int fd)
{
	return true;
}
