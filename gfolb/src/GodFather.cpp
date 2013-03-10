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

static SSL_CTX *ctx;
char* pass = NULL;
static IConnectionHandler *handler = NULL;
static bool isServerSSL = false,isThreadprq = false;
static int thrdpsiz/*,shmid*/;
Logger logger;

int s_server_session_id_context = 1;
int s_server_auth_session_id_context = 2;

GodFather::GodFather() {
}

GodFather::~GodFather() {
}

void sigchld_handlergf(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

/*The password code is not thread safe*/
int password_cb(char *buf,int num, int rwflag,void *userdata)
{
    if(num<(int)(strlen(pass)+1))
      return(0);

    strcpy(buf,pass);
    return(strlen(pass));
}

void load_dh_params(SSL_CTX *ctx,char *file)
{
    DH *ret=0;
    BIO *bio;

    if ((bio=BIO_new_file(file,"r")) == NULL)
    	logger << "Couldn't open DH file" << flush;

    ret=PEM_read_bio_DHparams(bio,NULL,NULL,
      NULL);
    BIO_free(bio);
    if(SSL_CTX_set_tmp_dh(ctx,ret)<0)
    	logger << "Couldn't set DH parameters" << flush;
  }

void sigpipe_handle(int x){
}

SSL_CTX *initialize_ctx(char *keyfile,char *password, char* ca_list)
{
    SSL_METHOD *meth;
    SSL_CTX *ctx;

    /* Global system initialization*/
    SSL_library_init();
    SSL_load_error_strings();

    /* Set up a SIGPIPE handler */
    signal(SIGPIPE,sigpipe_handle);

    /* Create our context*/
    meth=(SSL_METHOD*)SSLv23_method();
    ctx=SSL_CTX_new(meth);

    /* Load our keys and certificates*/
    if(!(SSL_CTX_use_certificate_chain_file(ctx,
      keyfile)))
    	logger << "Can't read certificate file" << flush;

    pass=password;
    SSL_CTX_set_default_passwd_cb(ctx,
      password_cb);
    if(!(SSL_CTX_use_PrivateKey_file(ctx,
      keyfile,SSL_FILETYPE_PEM)))
    	logger << "Can't read key file" << flush;

    /* Load the CAs we trust*/
    if(!(SSL_CTX_load_verify_locations(ctx, ca_list,0)))
    	logger << "Can't read CA list" << flush;
	#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    	SSL_CTX_set_verify_depth(ctx,1);
	#endif

    return ctx;
}

void destroy_ctx(SSL_CTX *ctx)
{
	SSL_CTX_free(ctx);
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
	logger << "Broken pipe ignore it" << getpid() << "\n" << flush;
}


void signalSIGSEGV(int dummy)
{
	signal(SIGSEGV,signalSIGSEGV);
	logger << "segmentation fault" << getpid() << "\n" << flush;
	exit(0);
}

int send_connection(int fd,int descriptor)
{
	struct msghdr msg;
	struct iovec  iov [1];
	int           n;

	/* need to send some data otherwise client can't distinguish between
	* EOF and "just sending a file descriptor"
	*/
	iov [0].iov_base = (void *)"x";
	iov [0].iov_len  = 1;

	/* not relevant for connected sockets */
	msg.msg_name    = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov     = iov;
	msg.msg_iovlen  = 1;
	msg.msg_flags   = 0;

	msg.msg_control = NULL;
	msg.msg_controllen = 0;

	/* put the descriptor in the ancillary data */
	{
		/* using a union to ensure its correctly aligned */
		union
		{
		  struct cmsghdr cmsg;
		  char           control [CMSG_SPACE (sizeof (int))];
		} msg_control;
		struct cmsghdr *cmsg;

		msg.msg_control    = &msg_control;
		msg.msg_controllen = sizeof (msg_control);

		cmsg = CMSG_FIRSTHDR (&msg);

		cmsg->cmsg_len   = CMSG_LEN (sizeof (int));
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type  = SCM_RIGHTS;

		*((int *) CMSG_DATA (cmsg)) = descriptor;
	}

	if((n= sendmsg(fd, &msg, 0)) < 0 )
	{
	  perror("sendmsg()");
	  exit(1);
	}
	close(descriptor);
	return n;
}

int receive_fd(int fd)
{
	union
	{
		struct cmsghdr cmsg;
		char           control [CMSG_SPACE (sizeof (int))];
	} msg_control;

	struct msghdr   msg;
	struct iovec    iov [1];
	struct cmsghdr *cmsg;
	char            buf [192];
	int             n;

	iov [0].iov_base = buf;
	iov [0].iov_len  = sizeof (buf);

	/* not relevant for connected sockets */
	msg.msg_name       = NULL;
	msg.msg_namelen    = 0;
	msg.msg_iov        = iov;
	msg.msg_iovlen     = 1;
	msg.msg_control    = &msg_control;
	msg.msg_controllen = sizeof (msg_control);
	msg.msg_flags      = 0;

	n = recvmsg (fd, &msg, 0);
	if(n == 1)
	{
		for (cmsg = CMSG_FIRSTHDR (&msg); cmsg; cmsg = CMSG_NXTHDR (&msg, cmsg))
		{
			int descriptor;

			if (cmsg->cmsg_len   != CMSG_LEN (sizeof (int)) ||
					cmsg->cmsg_level != SOL_SOCKET              ||
					cmsg->cmsg_type  != SCM_RIGHTS)
				continue;

			descriptor = *((int *) CMSG_DATA (cmsg));
			return descriptor;
		}
	}
	return -1;
}

void* service(void* arg)
{
	int fd = *(int*)arg;
	char buf[MAXBUFLE];
	int numbytes;
	while ((numbytes = recv(fd, buf, MAXBUFLE-1, 0)) == -1)
	{
		if(errno!=EAGAIN)
			return NULL;
	}
	string data(buf,buf+numbytes);
	memset(&buf[0], 0, sizeof(buf));
	strVec command;
	StringUtil::replaceAll(data,"\r","");
	StringUtil::replaceAll(data,"\n","");
	StringUtil::split(command, data, " ");
	if(command.size()<3 || command.at(0)!="OR" || 
		(command.at(1)!="CR" && command.at(1)!="AR" && command.at(1)!="DR"
			&& command.at(1)!="UR"))
	{
		send(fd,"Invalid command",15,0);
	}
	else
	{
		string flag = ConnectionPool::validate(command);
		send(fd,flag.c_str(),flag.length(),0);
	}
	close(fd);
	return NULL;
}

void* getRequest(void *arg)
{
	RequestProp *requestProp = (RequestProp*)arg;
	int fd = requestProp->fd;
	IConnectionHandler *handler = requestProp->handler;

	logger << "start read - singleRequest";
	string ind = handler->reader->singleRequest(fd);
	logger << "end read - singleRequest";
	if(ind.length()>0)
	{
		handler->reader->p_mutex.lock();
		handler->reader->q[fd].push(ind);
		handler->reader->fds[fd]=false;
		handler->reader->p_mutex.unlock();
	}
	delete requestProp;
	return NULL;
}

pid_t createChildProcess(int sp[],int sockfd)
{
	pid_t pid;
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sp) == -1)
	{
		perror("socketpair");
		exit(1);
	}
	if((pid=fork())==0)
	{
		if(isServerSSL)
		{
			PropFileReader pread;
			propMap props = pread.getProperties("gfolb.prop");
			string key_file = props["KEYFILE"];
			string dh_file = props["DHFILE"];
			string ca_list = props["CA_LIST"];
			string rand_file = props["RANDOM"];
			string sec_password = props["PASSWORD"];
			string tempcl = props["CLIENT_SEC_LEVEL"];
			/*HTTPS related*/
			//client_auth=CLIENT_AUTH_REQUIRE;
			/* Build our SSL context*/
			ctx = initialize_ctx((char*)key_file.c_str(), (char*)sec_password.c_str(), (char*)ca_list.c_str());
			load_dh_params(ctx, (char*)dh_file.c_str());

			SSL_CTX_set_session_id_context(ctx,
			  (const unsigned char*)&s_server_session_id_context,
			  sizeof s_server_session_id_context);

			/* Set our cipher list */
			/*if(ciphers){
			  SSL_CTX_set_cipher_list(ctx,ciphers);
			}*/
		}
		string filename;
		stringstream ss;
		ss << "./";
		ss << getpid();
		ss >> filename;
		filename.append(".cntrl");
		logger << "generated file " << filename << flush;
		ofstream cntrlfile;
		cntrlfile.open(filename.c_str());
		cntrlfile << "Process Running" << flush;
		cntrlfile.close();

		close(sockfd);

		SelEpolKqEvPrt selEpolKqEvPrtHandler;
		selEpolKqEvPrtHandler.initialize(sp[1]);
		ThreadPool pool;
		if(!isThreadprq)
		{
			pool.init(thrdpsiz,30,true);
		}

		while(1)
		{
			int nfds = selEpolKqEvPrtHandler.getEvents();
			if (nfds == -1)
			{
				perror("poller wait child process");
				logger << "\n----------poller child process----" << flush;
			}
			else
			{
				int fd = receive_fd(sp[1]);
				selEpolKqEvPrtHandler.reRegisterServerSock();
				if(isServerSSL)
				{
					fcntl(fd, F_SETFL, O_SYNC);
				}

				char buf[10];
				int err;
				if((err=recv(fd,buf,10,MSG_PEEK))==0)
				{
					close(fd);
					logger << "\nsocket conn closed before being serviced" << flush;
					continue;
				}

				handler->add(fd);

				RequestProp *requestProp = new RequestProp();
				requestProp->fd = fd;
				requestProp->handler = handler;
				if(isThreadprq)
				{
					Thread getRequest_thread(&getRequest, requestProp);
					getRequest_thread.execute();
				}
				else
				{
					requestProp->setCleanUp(true);
					pool.execute(*requestProp);
				}
			}
		}
	}
	return pid;
}


int main(int argc, char* argv[])
{

	logger = Logger::getLogger("GodFather");

	//signal(SIGSEGV,signalSIGSEGV);
	signal(SIGPIPE,signalSIGPIPE);
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;

	int yes=1;
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
		conn_pool = CastUtil::lexical_cast<int>(props["SPOOL_SIZ"]);//Connection pool size to the servers
	}
	catch(...)
	{
		conn_pool = 10;
	}
	string mod_mode = props["GFOLB_MMODE"];//Default or Name of module (http,smpp,sip...)
	isServerSSL = false;
	string ssl_enb = props["SSL_ENAB"];//is SSL enabled
	if(ssl_enb=="true" || ssl_enb=="TRUE")
		isServerSSL = true;
	string port_no = props["GFOLB_PORT"];//GFOLB listening port number
	string admin_port_no = props["GFOLB_ADMIN_PORT"];//GFOLB listening port number
	strVec ipprts;
	StringUtil::split(ipprts, serv_addrs, ";");

	int preForked = 5;
	if(props["NUM_PROC"]!="")
		preForked = CastUtil::lexical_cast<int>(props["NUM_PROC"]);
	string thrdpreq = props["THRD_PREQ"];
	if(thrdpreq=="true" || thrdpreq=="TRUE")
		isThreadprq = true;
	else
	{
		thrdpreq = props["THRD_PSIZ"];
		if(thrdpreq=="")
			thrdpsiz = 30;
		else
		{
			try
			{
				thrdpsiz = CastUtil::lexical_cast<int>(thrdpreq);
			}
			catch(...)
			{
				logger << "\nInvalid thread pool size defined" << flush;
				thrdpsiz = 30;
			}
		}
	}

	if(isServerSSL)
	{
		string key_file = props["KEYFILE"];
		string dh_file = props["DHFILE"];
		string ca_list = props["CA_LIST"];
		string rand_file = props["RANDOM"];
		string sec_password = props["PASSWORD"];
		string tempcl = props["CLIENT_SEC_LEVEL"];
		/*HTTPS related*/
		//client_auth=CLIENT_AUTH_REQUIRE;
		/* Build our SSL context*/
		ctx = initialize_ctx((char*)key_file.c_str(), (char*)sec_password.c_str(), (char*)ca_list.c_str());
		load_dh_params(ctx, (char*)dh_file.c_str());

		SSL_CTX_set_session_id_context(ctx,
		  (const unsigned char*)&s_server_session_id_context,
		  sizeof s_server_session_id_context);

		/* Set our cipher list */
		/*if(ciphers){
		  SSL_CTX_set_cipher_list(ctx,ciphers);
		}*/
	}

	sockfd = Server::createListener(port_no, false);

    SelEpolKqEvPrt selEpolKqEvPrtHandler;
    selEpolKqEvPrtHandler.initialize(sockfd);

    ofstream ofs("serv.ctrl");
    ofs << "Process" << flush;
    ofs.close();
    handler = new IConnectionHandler(ipprts,con_pers,conn_pool,props,isServerSSL,ctx);//new IConnectionHandler("localhost",8080,false,10);
    logger << "listening on port "<< port_no;

    Server server(admin_port_no,false,500,&service,2);

    int childNo = 0;
    vector<string> files;
    int sp[preForked][2];
    ThreadPool pool;
    if(Constants::IS_FILE_DESC_PASSING_AVAIL)
	{
		for(int j=0;j<preForked;j++)
		{
			pid_t pid = createChildProcess(sp[j],sockfd);
			stringstream ss;
			string filename;
			ss << "./";
			ss << pid;
			ss >> filename;
			filename.append(".cntrl");
			files.push_back(filename);
		}
	}
	else
	{
		if(!isThreadprq)
		{
			pool.init(thrdpsiz,thrdpsiz+30,true);
		}
	}


    ifstream ifs("serv.ctrl");
    while(ifs.is_open())
	{
    	if(childNo>=preForked)
    		childNo = 0;
    	nfds = selEpolKqEvPrtHandler.getEvents();
		if (nfds == -1)
		{
			perror("event handler main process");
			if(errno==EBADF)
				logger << "\nInavlid fd" <<flush;
			else if(errno==EFAULT)
				logger << "\nThe memory area pointed to by events is not accessible" <<flush;
			else if(errno==EINTR)
				logger << "\ncall was interrupted by a signal handler before any of the requested events occurred" <<flush;
			else if(errno==EINVAL)
				logger << "not a poll file descriptor, or maxevents is less than or equal to zero" << endl;
			else
				logger << "\nnot an epoll file descriptor" <<flush;
		}
		for(int n=0;n<nfds;n++)
		{
			int descriptor = selEpolKqEvPrtHandler.getDescriptor(n);
			if (descriptor == sockfd)
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
					selEpolKqEvPrtHandler.reRegisterServerSock();
					selEpolKqEvPrtHandler.registerForEvent(new_fd);
				}
			}
			else if (descriptor!=-1)
			{
				char buf[10];
				int err;
				logger << "got new connection "<< descriptor << endl;
				selEpolKqEvPrtHandler.unRegisterForEvent(descriptor);
				if(Constants::IS_FILE_DESC_PASSING_AVAIL)
				{
					ifstream cntrlfile;
					cntrlfile.open(files.at(childNo).c_str());
					if(cntrlfile.is_open())
					{
						send_connection(sp[childNo][0], descriptor);
						string cno = CastUtil::lexical_cast<string>(childNo);
						childNo++;
					}
					else
					{
						int tcn = childNo;
						for(int o=0;o<preForked;o++)
						{
							cntrlfile.open(files.at(o).c_str());
							if(cntrlfile.is_open())
							{
								send_connection(sp[childNo][0], descriptor);
								string cno = CastUtil::lexical_cast<string>(o);
								childNo = o+1;
								break;
							}
						}
						close(sp[tcn][0]);
						close(sp[tcn][1]);
						logger << "Process got killed" << flush;
						pid_t pid = createChildProcess(sp[tcn],sockfd);
						stringstream ss;
						string filename;
						ss << "./";
						ss << pid;
						ss >> filename;
						filename.append(".cntrl");
						files[tcn] = filename;
						logger << "created a new Process" << flush;
						logger.info("Process got killed hence created a new Process\n");
					}
					cntrlfile.close();
				}
				else
				{
					char buf[10];
					int err;
					if((err=recv(descriptor,buf,10,MSG_PEEK))==0)
					{
						close(descriptor);
						logger << "\nsocket conn closed before being serviced" << flush;
						continue;
					}

					if(isServerSSL)
					{
						fcntl(descriptor, F_SETFL, O_SYNC);
					}

					handler->add(descriptor);

					RequestProp *requestProp = new RequestProp();
					requestProp->fd = descriptor;
					requestProp->handler = handler;
					if(isThreadprq)
					{
						Thread getRequest_thread(&getRequest, requestProp);
						getRequest_thread.execute();
					}
					else
					{
						requestProp->setCleanUp(true);
						pool.execute(*requestProp);
					}
				}
			}
		}
	}
    ifs.close();
}
