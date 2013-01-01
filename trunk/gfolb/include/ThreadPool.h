/*
	Copyright 2009-2012, Sumeet Chhetri 
  
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
 * ThreadPool.h
 *
 *  Created on: Mar 23, 2010
 *      Author: sumeet
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include "TaskPool.h"
#include "PoolThread.h"
#include "Logger.h"

class ThreadPool {
	Logger logger;
	int maxThreads;
	int initThreads;
	int lowp;
	int highp;
	bool console;
	vector<PoolThread*> *tpool;
	TaskPool *wpool;
	Thread *poller;
	bool prioritypooling;
	static void* poll(void *arg);
	PoolThread* getIdleThread();
	bool started;
public:
	ThreadPool(int,int,int,int,bool);
	ThreadPool(int,int,int,int);
	ThreadPool(int,int,bool);
	ThreadPool(int,int);
	ThreadPool();
	void init(int,int,bool);
	void joinAll();
	void execute(Task &task, int priority);
	void execute(Task &task) ;
	void schedule(Task &task, int tunit, int type);
	virtual ~ThreadPool();
	void start();
};
#endif /* THREADPOOL_H_ */
