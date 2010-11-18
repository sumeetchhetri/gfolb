/*
 * IConnectionHandler.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "IConnectionHandler.h"

void IConnectionHandler::handle(IConnectionHandler* handler)
{
	handler->reader = new BufferedReader();
	map<int,bool>::iterator it;
	vector<int> fdss;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		for(it=handler->fds.begin();it!=handler->fds.end();it++)
		{
			if(!handler->reader->q[it->first].empty())
			{
				char* data = handler->reader->q[it->first].front();
				handler->reader->q[it->first].pop();
				if(data==NULL)
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

void IConnectionHandler::service(int fd,char* data)
{
	send(fd,data , strlen(data), 0);
}

IConnectionHandler::IConnectionHandler()
{
	boost::thread m_thread(boost::bind(&handle,this));
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
