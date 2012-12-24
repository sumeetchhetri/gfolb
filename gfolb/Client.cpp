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
 * Client.cpp
 *
 *  Created on: Mar 27, 2010
 *      Author: sumeet
 */

#include "Client.h"

Client::Client() {
	connected = false;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
}

bool Client::connection(string host,int port)
{
	struct sockaddr_in *remote;
	//int sock;
	int tmpres;
	char *ip;
	//char *get;

	sockfd = create_tcp_socket();
	ip = get_ip((char*)host.c_str());
	fprintf(stderr, "IP is %s\n", ip);
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
	if( tmpres < 0)
	{
		perror("Can't set remote->sin_addr.s_addr");
		//exit(1);
	}
	else if(tmpres == 0)
	{
		fprintf(stderr, "%s is not a valid IP address\n", ip);
		//exit(1);
	}
	remote->sin_port = htons(port);

	if(connect(sockfd, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
		perror("Could not connect");
		connected = false;
	} else {
		connected = true;
	}
	//if(!blk)fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
	/*get = build_get_query(host, page);
	  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

	  //Send the query to the server
	  sendData(get, sock);
	  //now it is time to receive the page
	  getData(sock);
	  free(get);*/
	free(remote);
	free(ip);
	//close(sock);
	//return 0;
	
	return connected;
}

void Client::setSocketBlocking()
{
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
}

void Client::setSocketNonBlocking()
{
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_SYNC);
}

int Client::sendData(string data)
{
	char* get = (char*)data.c_str();
	int sent = 0;
	while(data.length()>0)
	{
		int tmpres = send(sockfd, data.c_str(), data.length(), 0);
		if(tmpres == -1){
			perror("Can't send data");
		}
		data = data.substr(tmpres);
	}
	return sent;
}

string Client::getTextData()
{
	string alldat;
	char buf[BUFSIZ+1];
	memset(buf, 0, sizeof(buf));
	int tmpres = 0;
	while((tmpres = recv(sockfd, buf, BUFSIZ, 0)) > 0){
		string temp;
		for (int var = 0; var < tmpres; ++var) {
			temp.push_back(buf[var]);
		}
		alldat += temp;
		memset(buf, 0, sizeof(buf));
	}
	if(tmpres < 0)
	{
		perror("Error receiving data");
	}
	memset(buf, 0, sizeof(buf));
	//cout << alldat.length() << endl;
	return alldat;
}

int Client::receive(string& buf,int flag)
{
	char buff[BUFSIZ+1];
	memset(buff, 0, sizeof(buff));
	int t = recv(sockfd, buff, BUFSIZ, flag);
	buf = buff;
	memset(buff, 0, sizeof(buff));
	return t;
}

int Client::receivelen(string& buf,int len, int flag)
{
	char buff[len+1];
	memset(buff, 0, sizeof(buff));
	int t = recv(sockfd, buff, len, flag);
	buf = buff;
	memset(buff, 0, sizeof(buff));
	return t;
}

int Client::sendlen(string buf,int len)
{
	return send(sockfd, buf.c_str(), len, 0);
}

string Client::getBinaryData(int len, bool isLengthIncluded)
{
	cout << len << endl;
	string alldat;
	char *buf1 = new char[len+1];
	memset(buf1, 0, len);
	recv(sockfd, buf1, len, 0);
	for (int var = 0; var < len; ++var) {
		alldat.push_back(buf1[var]);
	}
	memset(buf1, 0, len);

	int leng = getLengthCl(alldat, len);
	if(isLengthIncluded)
	{
		leng -= len;
	}
	cout << "done reading header length " << leng << endl;
	char *buf = new char[leng+1];
	memset(buf, 0, sizeof(buf));
	recv(sockfd, buf, leng, 0);
	for (int var = 0; var < leng; ++var) {
		alldat.push_back(buf[var]);
	}
	memset(buf, 0, sizeof(buf));
	cout << alldat.length() << endl;
	return alldat;
}

void Client::closeConnection()
{
	connected = false;
	close(sockfd);
}

bool Client::isConnected()
{
	/*int numbytes;
	char buf[1];
	if ((numbytes = recv(sockfd, buf, 1, MSG_PEEK)) == 0)
	{
		return false;
	}*/
	return connected;
}
