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
 * PropFileReader.cpp
 *
 *  Created on: Aug 18, 2009
 *      Author: sumeet
 */

#include "PropFileReader.h"

PropFileReader::PropFileReader()
{

}
PropFileReader::~PropFileReader() {
	// TODO Auto-generated destructor stub
}

propMap PropFileReader::getProperties(string filepath)
{
	propMap all;
	string line;
	if (filepath=="")
	{
		return all;
	}
	ifstream myfile (&filepath[0],ios::in | ios::binary);
    if (myfile.is_open())
    {
      strVec vemp;
	  while(getline(myfile,line,'\n'))
	  {
		  if(line!="")
		  {
			  vemp = StringUtil::split(line, ("="));
		  	  if(vemp.size()==2)
		  	  {
		  		  string temp = vemp.at(1);
		  		  StringUtil::replaceAll(temp,"\n","");
		  		  StringUtil::replaceAll(temp,"\r","");
		  		  if(all[vemp.at(0)]!="")
		  			temp += ("," + all[vemp.at(0)]);
		  		  all[vemp.at(0)] = temp;
		  	  }
		  }
	  }
	  myfile.close();
    }
    return all;
}
