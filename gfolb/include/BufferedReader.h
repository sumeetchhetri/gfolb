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
 * BufferedReader.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef BUFFEREDREADER_H_
#define BUFFEREDREADER_H_
#include <sys/socket.h>
#include "cstring"
#include "string"
#include "vector"
#include "queue"
#include <iostream>
#include <openssl/ssl.h>
#include "CastUtil.h"
#include "PropFileReader.h"
#include <fcntl.h>
#include <errno.h>
#include "Mutex.h"
#include "Logger.h"
#define CLIENT_AUTH_REQUEST 1
#define CLIENT_AUTH_REQUIRE 2
#define CLIENT_AUTH_REHANDSHAKE 3
#define MAXBUFLEN 32768
#define MAXBUFLENM 32768
#define BUFSIZZ 1024
using namespace std;
typedef vector<string> strVec;

class SSLConnection {
public:
	SSL *ssl;
	BIO *io;
};


class BufferedReader {
	static void readRequests(BufferedReader* reader);
	static void generateRequest(BufferedReader* reader);
	static void read();
	map<int,string> data;
	bool isSSLEnabled,isDefault,isText,isLengthIncluded;
	int bfmlen;
	string hdrdelm,cntlnhdr;
	SSL_CTX *ctx;
	Logger logger;
public:
	Mutex p_mutex;
	string singleRequest(int);
	void erase(int);
	map<int,bool> fds;
	map<int, SSLConnection> *sslConns;
	map<int,std::queue<string> > q;
	BufferedReader(propMap props, SSL_CTX *ctx);
	virtual ~BufferedReader();
	bool isTextData(){return isText;}
	int getHeaderLength(){return bfmlen;}
	bool isHeaderLengthIncluded(){return isLengthIncluded;}
	bool isSSL(){return isSSLEnabled;}
	void closeSSL(int fd,SSL *ssl,BIO* bio);
	void error_occurred(const char *error,int fd,SSL *ssl);
	string getHeaderDelimiter(){return hdrdelm;}
	string getContentLnegthHeader(){return cntlnhdr;}
};

#endif /* BUFFEREDREADER_H_ */
