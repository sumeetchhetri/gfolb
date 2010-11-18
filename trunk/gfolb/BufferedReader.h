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
#include "queue"
#include <iostream>
#define MAXBUFLEN 32768

using namespace std;
class BufferedReader {

	static void readRequests(BufferedReader* reader);
	static void generateRequest(BufferedReader* reader);
	static void read();
	map<int,string> data;
	boost::mutex p_mutex;
public:
	int singleRequest();
	map<int,bool> fds;
	map<int,queue<char*> > q;
	map<int,bool> done;
	BufferedReader();
	virtual ~BufferedReader();
};

#endif /* BUFFEREDREADER_H_ */
