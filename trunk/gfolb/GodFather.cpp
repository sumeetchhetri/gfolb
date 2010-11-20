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
 * GodFather.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "GodFather.h"
#include "PropFileReader.h"

GodFather::GodFather() {
	// TODO Auto-generated constructor stub

}

GodFather::~GodFather() {
	// TODO Auto-generated destructor stub
}

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int byteArrayToInt(char *b) {
        return ((b[0] & 0xFF) << 24)
                + ((b[1] & 0xFF) << 16)
                + ((b[2] & 0xFF) << 8)
                + (b[3] & 0xFF);
}

char* intToByteArray(int value) {
        char *b = new char[4];
        for (int i = 0; i < 4; i++) {
            int offset = (4 - 1 - i) * 8;
            b[i] = (char) ((value >> offset) & 0xFF);
        }
        return b;
    }


int main(int argc, char* argv[])
{
	PropFileReader pread;
	propMap props = pread.getProperties("gfolb.prop");
	bool con_pers = false;
	string conn_type = props["CONN_PERS"];//Persistent or Non-persistent
	if(conn_type=="true" || conn_type=="TRUE")
		con_pers = true;
	string req_type = props["REQ_TYPE"];//Text or Binary Fixed Length
	string req_del_def = props["REQ_DEF_DEL"];//For Text (Header delimiter--\n|\r\n) For BFL (length of header in bytes)
	string req_con_len_txt = props["REQ_CNT_LEN_TXT"];//For Text only the header having content length (with the separator -- "Content-Length: ")
	string prot_type = props["PROT_TYPE"];//TCP or UDP
	string serv_addrs = props["SERV_ADDRS"];//localhost:8080;10.101.10.10:9999
	string gfolb_mode = props["GFOLB_MODE"];//LB or FO or CH or IR
	int conn_pool = 10;
	try
	{
		if(props["SPOOL_SIZ"]!="")
		conn_pool = boost::lexical_cast<int>(props["SPOOL_SIZ"]);//Connection pool size to the servers
	}
	catch(...)
	{
		conn_pool = 10;
	}
	string mod_mode = props["GFOLB_MMODE"];//Default or Name of module (http,smpp,sip...)
	string ssl_enb = props["SSL_ENAB"];//is SSL enabled
	string port_no = props["GFOLB_PORT"];//GFOLB listening port number
	strVec ipprts;
	boost::iter_split(ipprts, serv_addrs, boost::first_finder(";"));
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;


    int yes=1;
    int rv;


	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, port_no.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (listen(sockfd, BACKLOGM) == -1) {
        perror("listen");
        exit(1);
    }
    //fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    ofstream ofs("serv.ctrl");
    ofs << "Proces" << flush;
    ofs.close();
    IConnectionHandler *handler = new IConnectionHandler(ipprts,con_pers,conn_pool,props);//new IConnectionHandler("localhost",8080,false,10);
    cout << "listening on port "<< port_no << endl;
    ifstream ifs("serv.ctrl");
    while(ifs.is_open())
    {
    	new_fd = -1;
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}
		else
		{
			handler->add(new_fd);
		}
    }
    ifs.close();
}
