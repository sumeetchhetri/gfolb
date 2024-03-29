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
 * PropFileReader.h
 *
 *  Created on: Aug 18, 2009
 *      Author: sumeet
 */

#ifndef MODULEXTENSION_H_
#define MODULEXTENSION_H_
#include "string"

using namespace std;
class ModuleExtension {
public:
	ModuleExtension();
	virtual ~ModuleExtension();
	virtual void bind(int)=0;
	virtual void unbind(int)=0;
};

#endif /* MODULEXTENSION_H_ */
