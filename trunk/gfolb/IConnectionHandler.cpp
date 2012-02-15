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

void IConnectionHandler::handle(IConnectionHandler* handler) {
	handler->reader = new BufferedReader(handler->props);
	map<int, bool>::iterator it;
	vector<int> fdss;
	while (true) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		for (it = handler->fds.begin(); it != handler->fds.end(); it++) {
			if (handler->reader->q.find(it->first) != handler->reader->q.end()
					&& !handler->reader->q[it->first].empty()) {
				string data = handler->reader->q[it->first].front();
				handler->reader->q[it->first].pop();
				if (data.length()==0)
					boost::this_thread::sleep(
							boost::posix_time::milliseconds(1));
				else {
					handler->service(it->first, data);
				}
			} else if (handler->reader->fds[it->first]) {
				fdss.push_back(it->first);
			}
		}
		for (int i = 0; i < (int) fdss.size(); i++) {
			handler->unbind(fdss.at(i));
			handler->qmutex.lock();
			handler->fds.erase(fdss.at(i));
			handler->qmutex.unlock();
			close(fdss.at(i));
			handler->reader->erase(fdss.at(i));
			cout << "connection closed\n" << flush;
		}
		fdss.clear();
	}
}

bool IConnectionHandler::isSockConnected(int fd, int num) {
	char buf[num];
	int err;
	if ((err = recv(fd, buf, num, MSG_PEEK)) == 0) {
		cout << "socket closed before being service" << endl;
		return false;
	}
	return true;
}

void IConnectionHandler::service(int fd, string data) {

	if (!isSockConnected(fd, 1)) {
		this->reader->fds[fd] = true;
		return;
	}

	Connection *conn = ConnectionPool::getConnection();
	Client client = conn->client;
	if (!client.isConnected()) {
		this->reader->fds[fd] = true;
		ConnectionPool::release(conn);
		return;
	}
	if(this->reader->isTextData())
	{
		boost::replace_first(data, "Connection: keep-alive", "Connection: close");
		boost::replace_first(data, "Connection: Keep-alive", "Connection: close");
		boost::replace_first(data, "Connection: Keep-Alive", "Connection: close");
		boost::replace_first(data, "Connection: keep-Alive", "Connection: close");
	}
	//boost::replace_first(data, "HTTP/1.1", "HTTP/1.0");
	int bytes = client.sendData(data);
	cout << "===================Request START===================" << endl;
	cout << data << endl;
	cout << "====================Request END====================" << endl;

	string call, tot;
	if(!this->reader->isTextData())
		tot = client.getBinaryData(this->reader->getHeaderLength(), this->reader->isHeaderLengthIncluded());
	else
		tot = client.getTextData();
	//tot += client.getData();
	cout << "===================Response START===================" << endl;
	cout << tot << endl;
	cout << "====================Response END====================" << endl;

	int toto = tot.length();
	//cout << "data=" << toto << endl;
	while (toto > 0) {
		bytes = send(fd, tot.c_str(), tot.length(), 0);
		//cout << tot.length() << "=total and sent=" << bytes << endl;
		if (bytes == 0 || bytes == -1)
			break;
		tot = tot.substr(bytes);
		toto -= bytes;
	}
	if (ConnectionPool::isPersistent()){}
	else
	{
		this->reader->fds[fd] = true;
		//this->reader->done[fd] = true;

	}
	ConnectionPool::release(conn);
	//cout << "done with request" << flush;
}

/*IConnectionHandler::IConnectionHandler(string ip,int port,bool persistent,int poolsize)
 {
 boost::thread m_thread(boost::bind(&handle,this));
 ConnectionPool::createPool(poolsize,ip,port,persistent);
 }*/

IConnectionHandler::IConnectionHandler(vector<string> ipps, bool persistent,
		int poolsize, propMap props) {
	if (props["GFOLB_MODE"] == "LB" || props["GFOLB_MODE"] == "FO"
			|| props["GFOLB_MODE"] == "CH" || props["GFOLB_MODE"] == "OR") {
		this->mode = props["GFOLB_MODE"];
		cout << "GFOLB mode => " << this->mode << endl;
	} else {
		cout << "Invalid GFOLB mode" << endl;
		exit(0);
	}
	this->props = props;
	boost::thread m_thread(boost::bind(&handle, this));
	ConnectionPool::createPool(poolsize, ipps, persistent, this->mode);
}

void IConnectionHandler::add(int fd) {
	if (!isSockConnected(fd, 12)) {
		close(fd);
		return;
	}
	//cout << "added to conns" << endl;
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	qmutex.lock();
	this->fds[fd] = true;
	qmutex.unlock();
	this->reader->p_mutex.lock();
	this->reader->fds[fd] = false;
	this->reader->p_mutex.unlock();
	//this->reader->done[fd] = false;
	bind(fd);
}

IConnectionHandler::~IConnectionHandler() {
	// TODO Auto-generated destructor stub
}

bool IConnectionHandler::bind(int fd) {
	/*if(props["MOD_NAME"]!="" && !this->reader->isTextData())
	{
		dlib = dlopen("libmodule.so", RTLD_NOW|RTLD_GLOBAL);
		if(dlib==NULL)
		{
			cout << dlerror() << endl;
			Logger::info("Could not load Library");
		}
		else
		{
			Logger::info("Library loaded successfully");

		}
	}*/
	return true;
}

bool IConnectionHandler::unbind(int fd) {
	return true;
}
