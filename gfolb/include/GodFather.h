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
 * GodFather.h
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#ifndef GODFATHER_H_
#define GODFATHER_H_
#include "IConnectionHandler.h"
#include "string"
#include "vector"
#include "map"
using namespace std;
#include <algorithm>
#include <cstdlib>
#include <dlfcn.h>
#include "sstream"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <queue>
#include <sys/uio.h>
#include <sys/un.h>
#include <stdexcept>
//#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>
#include "string"
#include <sstream>
#include <typeinfo>
#include "Server.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "PropFileReader.h"
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Thread.h"
#include "CastUtil.h"
#include "Logger.h"
#include "SelEpolKqEvPrt.h"
#define MAXEPOLLSIZE 100
#define MAXBUFLENM 32768
#define PORT "9992"
#define NUM_PROC 2
#define SKEYFILE "server.pem"
#define SPASSWORD "password"
#define SDHFILE "dh1024.pem"
#define SCA_LIST "root.pem"

class GodFather {
public:
	GodFather();
	virtual ~GodFather();
};

#endif /* GODFATHER_H_ */
