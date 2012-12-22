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
#include <boost/thread/thread.hpp>
#include "cstring"
#include "string"
#include "vector"
#include "queue"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <openssl/ssl.h>
#include <boost/lexical_cast.hpp>
#include "PropFileReader.h"
#include <fcntl.h>
#include <errno.h>
#define CLIENT_AUTH_REQUEST 1
#define CLIENT_AUTH_REQUIRE 2
#define CLIENT_AUTH_REHANDSHAKE 3
#define MAXBUFLEN 32768
#define MAXBUFLENM 32768
#define BUFSIZZ 1024
using namespace std;
typedef vector<string> strVec;

class BufferedReader {

	static void readRequests(BufferedReader* reader);
	static void generateRequest(BufferedReader* reader);
	static void read();
	map<int,string> data;
	bool isSSLEnabled,isDefault,isText,isLengthIncluded;
	int bfmlen;
	string hdrdelm,cntlnhdr;
public:
	boost::mutex p_mutex;
	string singleRequest(int);
	void erase(int);
	map<int,bool> fds;
	map<int,queue<string> > q;
	BufferedReader(propMap props);
	virtual ~BufferedReader();
	bool isTextData(){return isText;}
	int getHeaderLength(){return bfmlen;}
	bool isHeaderLengthIncluded(){return isLengthIncluded;}
};

#endif /* BUFFEREDREADER_H_ */
