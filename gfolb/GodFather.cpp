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
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SSLClient.h"
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

void signalSIGPIPE(int dummy)
{
	signal(SIGPIPE,signalSIGPIPE);
	/*string filename;
	stringstream ss;
	ss << servd;
	ss << getpid();
	ss >> filename;
	filename.append(".cntrl");
	remove(filename.c_str());*/
	void * array[25];
	int nSize = backtrace(array, 25);
	char ** symbols = backtrace_symbols(array, nSize);
	string tempo;
	for (int i = 0; i < nSize; i++)
	{
		tempo = symbols[i];
		tempo += "\n";
	}
	free(symbols);
	cout << "Broken pipe ignore it" << getpid() << "\n" << tempo << flush;
	//abort();
}


void signalSIGSEGV(int dummy)
{
	signal(SIGSEGV,signalSIGSEGV);
	/*string filename;
	stringstream ss;
	ss << servd;
	ss << getpid();
	ss >> filename;
	filename.append(".cntrl");
	remove(filename.c_str());*/
	void * array[25];
	int nSize = backtrace(array, 25);
	char ** symbols = backtrace_symbols(array, nSize);
	string tempo;
	for (int i = 0; i < nSize; i++)
	{
		tempo = symbols[i];
		tempo += "\n";
	}
	free(symbols);
	cout << "segmentation fault" << getpid() << "\n" << tempo << flush;
	//abort();
}

void service(int fd)
{
	char buf[MAXDATASIZE];
	int numbytes;
	while ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) == -1)
	{
		//perror("recv");
		if(errno!=EAGAIN)
			return;
	}
	string data(buf,buf+numbytes);
	memset(&buf[0], 0, sizeof(buf));
	strVec command;
	boost::replace_all(data,"\r","");
	boost::replace_all(data,"\n","");
	boost::iter_split(command, data, boost::first_finder(" "));
	if(command.size()<3 || command.at(0)!="OR" || command.at(1)!="CR")
	{
		send(fd,"Invalid command",15,0);
	}
	else
	{
		string flag = ConnectionPool::validate(command);
		send(fd,flag.c_str(),flag.length(),0);
	}
	close(fd);
}

int main(int argc, char* argv[])
{
	signal(SIGSEGV,signalSIGSEGV);
	signal(SIGPIPE,signalSIGPIPE);
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;


	struct epoll_event ev;
	//struct rlimit rt;
	int yes=1;
	//char s[INET6_ADDRSTRLEN];
	int rv,nfds;
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
	string admin_port_no = props["GFOLB_ADMIN_PORT"];//GFOLB listening port number
	strVec ipprts;
	boost::iter_split(ipprts, serv_addrs, boost::first_finder(";"));


	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
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
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    struct epoll_event events[MAXEPOLLSIZE];
	int epoll_handle = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN | EPOLLPRI;
	ev.data.fd = sockfd;
	if (epoll_ctl(epoll_handle, EPOLL_CTL_ADD, sockfd, &ev) < 0)
	{
		fprintf(stderr, "epoll set insertion error: fd=%d\n", sockfd);
		return -1;
	}
	else
		printf("listener socket to join epoll success!\n");

    ofstream ofs("serv.ctrl");
    ofs << "Proces" << flush;
    ofs.close();
    IConnectionHandler *handler = new IConnectionHandler(ipprts,con_pers,conn_pool,props);//new IConnectionHandler("localhost",8080,false,10);
    cout << "listening on port "<< port_no << endl;

    Server server(admin_port_no,false,500,&service,true);

    ifstream ifs("serv.ctrl");
    int curfds = 1;
    while(ifs.is_open())
	{

    	nfds = epoll_wait(epoll_handle, events, curfds,-1);

		if (nfds == -1)
		{
			perror("epoll_wait main process");
			//logfile << "Interruption Signal Received\n" << flush;
			printf("Interruption Signal Received\n");
			curfds = 1;
			if(errno==EBADF)
				cout << "\nInavlid fd" <<flush;
			else if(errno==EFAULT)
				cout << "\nThe memory area pointed to by events is not accessible" <<flush;
			else if(errno==EINTR)
				cout << "\ncall was interrupted by a signal handler before any of the requested events occurred" <<flush;
			else
				cout << "\nnot an epoll file descriptor" <<flush;
			//break;
		}
		for(int n=0;n<nfds;n++)
		{
			if (events[n].data.fd == sockfd)
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
					curfds++;
					fcntl(new_fd, F_SETFL, fcntl(new_fd, F_GETFD, 0) | O_NONBLOCK);
					ev.events = EPOLLIN | EPOLLPRI;
					ev.data.fd = new_fd;
					if (epoll_ctl(epoll_handle, EPOLL_CTL_ADD, new_fd, &ev) < 0)
					{
						perror("epoll");
						cout << "\nerror adding to epoll cntl list" << flush;
						return -1;
					}
				}
			}
			else
			{
				char buf[10];
				int err;
				if((err=recv(events[n].data.fd,buf,10,MSG_PEEK))==0)
				{
					close(events[n].data.fd);
					cout << "\nsocket conn closed before being serviced" << flush;
					epoll_ctl(epoll_handle, EPOLL_CTL_DEL, events[n].data.fd,&ev);
					curfds--;
				}
				else if(err==10)
				{
					handler->add(events[n].data.fd);
					//epoll_ctl(epoll_handle, EPOLL_CTL_DEL, events[n].data.fd,&ev);
					//curfds--;
					//cout << "new valid conn" << endl;
					string ind = handler->reader->singleRequest(events[n].data.fd);
					//cout << ind.c_str() << endl;
					if(ind.length()>0)
					{
						//cout << "sending pdu" << flush;
						handler->reader->p_mutex.lock();
						handler->reader->q[events[n].data.fd].push(ind);
						handler->reader->fds[events[n].data.fd]=false;
						handler->reader->p_mutex.unlock();
					}

				}
			}
		}
	}
    ifs.close();
}

