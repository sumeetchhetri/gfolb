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

void* IConnectionHandler::handleRequests(void* arg) {
	IConnectionHandler* handler = (IConnectionHandler*)arg;
	map<int, bool>::iterator it;
	while (true) {
		Thread::mSleep(1);
		for (it = handler->fds.begin(); it != handler->fds.end(); it++) {
			if (handler->reader->q.find(it->first) != handler->reader->q.end()
					&& !handler->reader->q[it->first].empty()) {
				string data = handler->reader->q[it->first].front();
				handler->reader->q[it->first].pop();
				if (data.length()>0) {
					RequestProp *requestProp = new RequestProp();
					requestProp->fd = it->first;
					requestProp->handler = handler;
					requestProp->data = data;

					Thread service_thread(&service, requestProp);
					service_thread.execute();
				}
			}
		}
	}
	return NULL;
}

void* IConnectionHandler::handleReleaseConnections(void* arg) {
	IConnectionHandler* handler = (IConnectionHandler*)arg;
	map<int, bool>::iterator it;
	vector<int> fdss;
	while (true) {
		Thread::mSleep(1);
		for (it = handler->fds.begin(); it != handler->fds.end(); it++) {
			if (handler->reader->fds[it->first]) {
				fdss.push_back(it->first);
			}
		}
		for (int i = 0; i < (int) fdss.size(); i++) {
			handler->unbind(fdss.at(i));
			handler->qmutex.lock();
			handler->fds.erase(fdss.at(i));

			if(handler->reader->isSSL() && handler->reader->sslConns->find(fdss.at(i))!=handler->reader->sslConns->end())
			{
				SSLConnection sslConn = handler->reader->sslConns->find(fdss.at(i))->second;
				handler->reader->closeSSL(fdss.at(i), sslConn.ssl, sslConn.io);
			}
			else
			{
				close(fdss.at(i));
			}

			handler->reader->erase(fdss.at(i));
			handler->qmutex.unlock();
			handler->logger << "connection closed\n" << flush;
		}
		fdss.clear();
	}
	return NULL;
}

void* IConnectionHandler::service(void* arg) {

	RequestProp *requestProp = (RequestProp*)arg;
	int fd = requestProp->fd;
	string data = requestProp->data;
	IConnectionHandler* handler = requestProp->handler;

	handler->logger << "start request service";

	if (!ClientInterface::isConnected(fd)) {
		handler->reader->p_mutex.lock();
		handler->reader->fds[fd] = true;
		handler->reader->p_mutex.unlock();
		return NULL;
	}

	Connection *conn = ConnectionPool::getConnection();
	if (!conn->client->isConnected()) {
		handler->reader->p_mutex.lock();
		handler->reader->fds[fd] = true;
		handler->reader->p_mutex.unlock();

		ConnectionPool::release(conn);
		return NULL;
	}

	if(handler->reader->isTextData())
	{
		StringUtil::replaceFirst(data, "Connection: keep-alive", "Connection: close");
		StringUtil::replaceFirst(data, "Connection: Keep-alive", "Connection: close");
		StringUtil::replaceFirst(data, "Connection: Keep-Alive", "Connection: close");
		StringUtil::replaceFirst(data, "Connection: keep-Alive", "Connection: close");
		StringUtil::replaceFirst(data, "{REPLACE_HOST_f2079930-4a8b-11e2-bcfd-0800200c9a66}", conn->host);
	}

	int bytes = conn->client->sendData(data);
	/*handler->logger << "Request START";
	handler->logger << data;
	handler->logger << "Request END";*/

	string call, tot;
	if(!handler->reader->isTextData())
		tot = conn->client->getBinaryData(handler->reader->getHeaderLength(), handler->reader->isHeaderLengthIncluded());
	else
		tot = conn->client->getTextData(handler->reader->getHeaderDelimiter(), handler->reader->getContentLnegthHeader());

	/*handler->logger << "Response START";
	handler->logger << tot;
	handler->logger << "Response END";*/

	int toto = tot.length();

	if(handler->reader->isSSL())
	{
		if(handler->reader->sslConns->find(fd)!=handler->reader->sslConns->end())
		{
			SSLConnection sslConn = handler->reader->sslConns->find(fd)->second;
			int r;
			if((r=BIO_write(sslConn.io, tot.c_str(), tot.length()))<=0)
			{
				handler->logger << "send failed";
			}
			if((r=BIO_flush(sslConn.io))<0)
			{
				handler->logger << "Error flushing BIO";
			}
		}
	}
	else
	{
		while (toto > 0) {
			bytes = send(fd, tot.c_str(), tot.length(), 0);
			if (bytes == 0 || bytes == -1)
				break;
			tot = tot.substr(bytes);
			toto -= bytes;
		}
	}
	if (ConnectionPool::isPersistent()){}
	else
	{
		handler->reader->p_mutex.lock();
		handler->reader->fds[fd] = true;
		handler->reader->p_mutex.unlock();

	}
	ConnectionPool::release(conn);
	delete requestProp;
	handler->logger << "done request service";
	return NULL;
}

IConnectionHandler::IConnectionHandler(vector<string> ipps, bool persistent,
		int poolsize, propMap props, bool isSSL, SSL_CTX *ctx) {
	logger = Logger::getLogger("IConnectionHandler");

	if (props["GFOLB_MODE"] == "LB" || props["GFOLB_MODE"] == "FO"
			|| props["GFOLB_MODE"] == "CH" || props["GFOLB_MODE"] == "OR") {
		this->mode = props["GFOLB_MODE"];
		logger << "GFOLB mode => " << this->mode;
	} else {
		logger << "Invalid GFOLB mode";
		exit(0);
	}
	int numretries = 3;
	if (props["NUM_RETRIES"]!="")
	{
		try {
			numretries = CastUtil::lexical_cast<int>(props["NUM_RETRIES"]);
		} catch(...) {
		}
	}

	this->props = props;
	this->reader = new BufferedReader(props, ctx);

	Thread handleRequests_thread(&handleRequests, this);
	handleRequests_thread.execute();

	Thread handleReleaseConnections_thread(&handleReleaseConnections, this);
	handleReleaseConnections_thread.execute();

	ConnectionPool::createPool(poolsize, ipps, persistent, this->mode, numretries);
}

void IConnectionHandler::add(int fd) {
	if (!ClientInterface::isConnected(fd)) {
		close(fd);
		return;
	}
	qmutex.lock();
	this->fds[fd] = true;
	qmutex.unlock();
	this->reader->p_mutex.lock();
	this->reader->fds[fd] = false;
	this->reader->p_mutex.unlock();
	bind(fd);
}

IConnectionHandler::~IConnectionHandler() {
}

bool IConnectionHandler::bind(int fd) {
	/*if(props["MOD_NAME"]!="" && !this->reader->isTextData())
	{
		dlib = dlopen("libmodule.so", RTLD_NOW|RTLD_GLOBAL);
		if(dlib==NULL)
		{
			logger << dlerror();
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

void RequestProp::run()
{
	string ind = handler->reader->singleRequest(fd);
	if(ind.length()>0)
	{
		handler->reader->p_mutex.lock();
		handler->reader->q[fd].push(ind);
		handler->reader->fds[fd]=false;
		handler->reader->p_mutex.unlock();
	}
}
