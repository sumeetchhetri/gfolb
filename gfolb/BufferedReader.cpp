/*
 * BufferedReader.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "BufferedReader.h"

void BufferedReader::readRequests(BufferedReader* reader)
{
	char buf[MAXBUFLEN];
	int bytes = -1;
	map<int,bool>::iterator it;
	while(true)
	{
		for(it=reader->fds.begin();it!=reader->fds.end();it++)
		{
			if((bytes=recv(it->first, buf, sizeof(buf), MSG_DONTWAIT))>0)
			{
				cout << "added data to reader\n" << buf << endl << endl;
				reader->data[it->first].append(buf);
				memset(&buf[0], 0, sizeof(buf));
				bytes = -1;
			}
			else if(bytes==0)
				reader->done[it->first] = true;
			else if(bytes==-1 && errno==ECONNRESET)
				reader->done[it->first] = true;
		}
	}
}

void BufferedReader::generateRequest(BufferedReader* reader)
{
	map<int,bool>::iterator it;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		for(it=reader->fds.begin();it!=reader->fds.end();it++)
		{
			int ind = reader->singleRequest();
			if(ind==-1 || reader->data[it->first].length()<ind)
				continue;
			string temp = reader->data[it->first].substr(0,ind);
			cout << "generated new request\n" << temp << endl << endl;
			reader->q[it->first].push((char*)temp.c_str());
			reader->p_mutex.lock();
			reader->data[it->first] = reader->data[it->first].substr(10);
			reader->p_mutex.unlock();
		}
	}
}

int BufferedReader::singleRequest()
{
	return 10;
}

BufferedReader::BufferedReader() {
	boost::thread r_thread(boost::bind(&readRequests,this));
	boost::thread g_thread(boost::bind(&generateRequest,this));
}

BufferedReader::~BufferedReader() {
	// TODO Auto-generated destructor stub
}
