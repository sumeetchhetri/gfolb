/*
 * IConnectionHandler.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef ICONNECTIONHANDLER_H_
#define ICONNECTIONHANDLER_H_
#include "BufferedReader.h"
#include <fcntl.h>
class IConnectionHandler {
	BufferedReader *reader;
	map<int,bool> fds;
	static void handle(IConnectionHandler* handler);
	void service(int fd,char* data);
public:
	IConnectionHandler();
	virtual ~IConnectionHandler();
	void add(int);
	bool bind(int);
	bool unbind(int);
};

#endif /* ICONNECTIONHANDLER_H_ */
