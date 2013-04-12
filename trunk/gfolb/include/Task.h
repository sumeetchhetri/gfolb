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
 * Task.h
 *
 *  Created on: Mar 23, 2010
 *      Author: sumeet
 */

#ifndef TASK_H_
#define TASK_H_
#include "string"
#include "Timer.h"
#include "TimeUnit.h"
using namespace std;

class Task {
	bool isFuture;
	int tunit;
	int type;
	int priority;
	bool console;
	bool cleanUp;
	Task(int priority);
	Task(int tunit, int type);
	friend class PoolThread;
	friend class ThreadPool;
	friend class TaskPool;
	friend class FutureTask;
	bool isWaitOver(Timer *timer);
public:
	void setCleanUp(bool);
	Task();
	virtual ~Task();
	virtual void run()=0;
protected:
	void init();
	void setPriority(int priority);
	void setTunitType(int tunit, int type);
};

#endif /* TASK_H_ */
