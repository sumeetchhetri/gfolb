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
 * BufferedReader.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: sumeet
 */

#include "BufferedReader.h"

void error_occurred(const char *error,int fd,SSL *ssl)
{
	cout << error << flush;
	//close(fd);
	int r=SSL_shutdown(ssl);
	if(!r){
	  /* If we called SSL_shutdown() first then
		 we always get return value of '0'. In
		 this case, try again, but first send a
		 TCP FIN to trigger the other side's
		 close_notify*/
	  //shutdown(fd,1);
	  r=SSL_shutdown(ssl);
	}
	switch(r){
	  case 1:
		break; /* Success */
	  case 0:
	  case -1:
	  default:
		  cout << "shutdown failed" << flush;
	}
	SSL_free(ssl);
}

void closeSSL(int fd,SSL *ssl,BIO* bio)
{
	BIO_free_all(bio);
	int r=SSL_shutdown(ssl);
	if(!r){
	  /* If we called SSL_shutdown() first then
		 we always get return value of '0'. In
		 this case, try again, but first send a
		 TCP FIN to trigger the other side's
		 close_notify*/
	  //shutdown(fd,1);
	  r=SSL_shutdown(ssl);
	}
	switch(r){
	  case 1:
		break; /* Success */
	  case 0:
	  case -1:
	  default:
		  cout << "shutdown failed" << flush;
	}
	SSL_free(ssl);
}

void BufferedReader::readRequests(BufferedReader* reader)
{
	char buf[MAXBUFLEN];
	int bytes = -1;
	map<int,bool>::iterator it;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		for(it=reader->fds.begin();it!=reader->fds.end();it++)
		{
			if((bytes=recv(it->first, buf, sizeof(buf), MSG_DONTWAIT))>0)
			{
				cout << "added data to reader\n" << buf << endl << endl;
				reader->data[it->first].append(buf);
				memset(&buf[0], 0, sizeof(buf));
				bytes = -1;
			}
			else if(bytes==0)
				reader->fds[it->first] = true;
			else if(bytes==-1 && errno==ECONNRESET)
				reader->fds[it->first] = true;
		}
	}
}

void BufferedReader::generateRequest(BufferedReader* reader)
{
	map<int,bool>::iterator it;
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		for(it=reader->fds.begin();it!=reader->fds.end();it++)
		{
			if(reader->fds[it->first]==false)
			{
				string ind = reader->singleRequest(it->first);
				if(ind!="")
				{
					reader->q[it->first].push(ind.c_str());
					reader->fds[it->first]=true;
				}
			}
			else
			{

			}
		}
	}
}

void BufferedReader::erase(int fd)
{
	this->p_mutex.lock();
	this->fds.erase(fd);
	this->q.erase(fd);
	this->p_mutex.unlock();
}

int getLength(string header,int size)
{
	int totsize = header[size-1] & 0xff;
	for (int var = 0; var < size-1; var++)
	{
		totsize |= ((header[var] & 0xff) << (size-1-var)*8);
	}
	return totsize;
}

string BufferedReader::singleRequest(int fd)
{
	SSL *ssl=NULL;
	BIO *sbio=NULL;
	BIO *io=NULL,*ssl_bio=NULL;
	int cntlen = 0;
	char buf[MAXBUFLENM];
	string alldat;
	if(isText)
	{
		if(isSSLEnabled)
		{
			sbio=BIO_new_socket(fd,BIO_NOCLOSE);
			//cout << "\nBefore = " << ssl << flush;
			//ssl=SSL_new(ctx);
			//cout << "\nAfter = " << ssl << flush;
			SSL_set_bio(ssl,sbio,sbio);
			int r;
			if((r=SSL_accept(ssl)<=0))
			{
				error_occurred("SSL accept error",fd,ssl);
				this->fds[fd]=true;
				if(io!=NULL)BIO_free_all(io);
				return "";
			}

			io=BIO_new(BIO_f_buffer());
			ssl_bio=BIO_new(BIO_f_ssl());
			BIO_set_ssl(ssl_bio,ssl,BIO_NOCLOSE);
			BIO_push(io,ssl_bio);
			int er=-1;
			bool flag = true;
			while(flag)
			{
				er = BIO_gets(io,buf,BUFSIZZ-1);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_NONE:
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						error_occurred("SSL error problem",fd,ssl);
						if(io!=NULL)BIO_free_all(io);
						this->fds[fd]=true;
						return "";
					}
					default:
					{
						error_occurred("SSL read problem",fd,ssl);
						if(io!=NULL)BIO_free_all(io);
						this->fds[fd]=true;
						return "";
					}
				}
				if(!strcmp(buf,this->hdrdelm.c_str()))
				{
					alldat += (buf);
					break;
				}
				string temp(buf);
				temp = temp.substr(0,temp.length()-1);
				alldat += (temp + "\n");
				if(temp.find(this->cntlnhdr)!=string::npos)
				{
					std::string cntle = temp.substr(temp.find(": ")+2);
					cntle = cntle.substr(0,cntle.length()-1);
					try
					{
						cntlen = boost::lexical_cast<int>(cntle);
					}
					catch(boost::bad_lexical_cast&)
					{
						cout << "bad lexical cast" <<endl;
					}
				}
				memset(&buf[0], 0, sizeof(buf));
			}
			if(io!=NULL && cntlen==0)closeSSL(fd,ssl,io);
		}
		else
		{
			int er=-1;
			bool flag = true;
			sbio=BIO_new_socket(fd,BIO_NOCLOSE);
			io=BIO_new(BIO_f_buffer());
			BIO_push(io,sbio);
			//biosss[fd] = sbio;
			while(flag)
			{
				er = BIO_gets(io,buf,BUFSIZZ-1);
				if(er==0)
				{
					this->fds[fd]=true;
					if(io!=NULL)BIO_free_all(io);
					cout << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				if(!strcmp(buf,this->hdrdelm.c_str()) || er<0)
				{
					alldat += (buf);
					break;
				}
				string temp(buf);
				//if(temp.find("Range:")==0 || temp.find("If-Range:")==0)
				//	continue;
				temp = temp.substr(0,temp.length()-1);
				if(temp.find("Host:")!=string::npos)
				{
					temp = "Host: {REPLACE_HOST_f2079930-4a8b-11e2-bcfd-0800200c9a66}";
				}
				alldat += (temp + "\n");
				//cout << temp <<endl;
				if(temp.find(this->cntlnhdr)!=string::npos)
				{
					std::string cntle = temp.substr(temp.find(": ")+2);
					cntle = cntle.substr(0,cntle.length()-1);
					//cout << "contne-length="<<cntle <<endl;
					try
					{
						cntlen = boost::lexical_cast<int>(cntle);
					}
					catch(boost::bad_lexical_cast&)
					{
						cout << "bad lexical cast" <<endl;
					}
				}

				memset(&buf[0], 0, sizeof(buf));
			}
			//if(io!=NULL && cntlen==0)BIO_free_all(io);
		}
		if(isSSLEnabled)
		{
			int er=-1;
			if(cntlen>0)
			{
				//cout << "reading conetnt " << cntlen << endl;
				er = BIO_read(io,buf,cntlen);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_NONE:
						cntlen -= er;
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						error_occurred("SSL error problem",fd,ssl);
						if(io!=NULL)BIO_free_all(io);
						this->fds[fd]=true;
						return "";
					}
					default:
					{
						error_occurred("SSL read problem",fd,ssl);
						if(io!=NULL)BIO_free_all(io);
						this->fds[fd]=true;
						return "";
					}
				}
				string temp(buf);
				alldat += (temp);
				memset(&buf[0], 0, sizeof(buf));
			}
			if(io!=NULL)closeSSL(fd,ssl,io);
		}
		else if(cntlen>0)
		{
			int er=-1;
			while(cntlen>0)
			{
				er = BIO_read(io,buf,cntlen);
				if(er==0)
				{
					this->fds[fd]=true;
					if(io!=NULL)BIO_free_all(io);
					cout << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				else if(er>0)
				{
					string temp(buf);
					alldat += (temp);
					memset(&buf[0], 0, sizeof(buf));
					cntlen -= er;
				}
				//cout << cntlen << " " << er << endl;
			}

			if(io!=NULL)BIO_free_all(io);
		}
		else
		{
			int er=-1;
			while(cntlen>0)
			{
				er = BIO_read(io,buf,cntlen);
				if(er<=0 && errno !=EWOULDBLOCK)
				{
					this->fds[fd]=true;
					if(io!=NULL)BIO_free_all(io);
					cout << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				else if(er>0)
				{
					string temp(buf);
					alldat += (temp);
					memset(&buf[0], 0, sizeof(buf));
					cntlen -= er;
				}
				//cout << cntlen << " " << er << endl;
			}

			if(io!=NULL)BIO_free_all(io);
		}
	}
	else
	{
		//cout << "ISHOULD NVERE BE HERE" << endl;
		stringstream ss;
		if(isSSLEnabled)
		{
			sbio=BIO_new_socket(fd,BIO_NOCLOSE);
			//cout << "\nBefore = " << ssl << flush;
			//ssl=SSL_new(ctx);
			//cout << "\nAfter = " << ssl << flush;
			SSL_set_bio(ssl,sbio,sbio);
			int r;
			if((r=SSL_accept(ssl)<=0))
			{
				error_occurred("SSL accept error",fd,ssl);
				this->fds[fd]=true;
				if(io!=NULL)BIO_free_all(io);
				return "";
			}

			io=BIO_new(BIO_f_buffer());
			ssl_bio=BIO_new(BIO_f_ssl());
			BIO_set_ssl(ssl_bio,ssl,BIO_NOCLOSE);
			BIO_push(io,ssl_bio);
			int er=-1;
			if(bfmlen>0)
			{
				//cout << "reading conetnt " << cntlen << endl;
				er = BIO_read(io,buf,bfmlen);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_NONE:
						cntlen -= er;
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						error_occurred("SSL error problem",fd,ssl);
						if(io!=NULL)BIO_free(io);
						this->fds[fd]=true;
						return "";
					}
					default:
					{
						error_occurred("SSL read problem",fd,ssl);
						if(io!=NULL)BIO_free(io);
						this->fds[fd]=true;
						return "";
					}
				}
				ss << buf;
				memset(&buf[0], 0, sizeof(buf));
			}
		}
		else
		{
			//cout << bfmlen << flush;
			int er=-1;
			if(bfmlen>0)
			{
				sbio=BIO_new_socket(fd,BIO_NOCLOSE);
				io=BIO_new(BIO_f_buffer());
				BIO_push(io,sbio);
				er = BIO_read(io,buf,bfmlen);
				if(er==0)
				{
					this->fds[fd]=true;
					cout << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				else if(er>0)
				{
					for(int i=0;i<bfmlen;i++)
						alldat.push_back(buf[i]);
					memset(&buf[0], 0, sizeof(buf));
				}
			}
		}
		int lengthm = getLength(alldat,bfmlen);
		/*if(bfmlen==4)
			lengthm = ((alldat[0] & 0xff) << 24) | ((alldat[1] & 0xff) << 16) | ((alldat[2] & 0xff) << 8) | ((alldat[3] & 0xff));
		else if(bfmlen==3)
			lengthm = ((alldat[0] & 0xff) << 16) | ((alldat[1] & 0xff) << 8) | ((alldat[2] & 0xff));
		else if(bfmlen==2)
			lengthm = ((alldat[0] & 0xff) << 8) | ((alldat[1] & 0xff));
		else
			lengthm = ((alldat[0] & 0xff));*/
		if(isLengthIncluded)
		{
			lengthm -= bfmlen;
		}
		cout << lengthm << flush;
		if(isSSLEnabled)
		{
			int er=-1;
			if(lengthm>0)
			{
				//cout << "reading conetnt " << cntlen << endl;
				er = BIO_read(io,buf,lengthm);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_NONE:
						cntlen -= er;
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						error_occurred("SSL error problem",fd,ssl);
						if(io!=NULL)BIO_free(io);
						this->fds[fd]=true;
						return "";
					}
					default:
					{
						error_occurred("SSL read problem",fd,ssl);
						if(io!=NULL)BIO_free(io);
						this->fds[fd]=true;
						return "";
					}
				}
				ss << buf;
				memset(&buf[0], 0, sizeof(buf));
			}
		}
		else
		{
			int er=-1;
			if(lengthm>0)
			{
				er = BIO_read(io,buf,lengthm);
				//cout << er << endl;
				if(er==0)
				{
					this->fds[fd]=true;
					cout << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				else if(er>0)
				{
					for(int i=0;i<er;i++)
					{
						//cout << "=" << buf[i] << endl;
						alldat.push_back(buf[i]);
					}
					memset(&buf[0], 0, sizeof(buf));
				}
			}
		}
	}
	//cout << alldat << flush;
	return alldat;
}

BufferedReader::BufferedReader(propMap props)
{
	if(props["SSL_ENAB"]=="true" || props["SSL_ENAB"]=="TRUE")
		isSSLEnabled = true;
	if(props["GFOLB_MMODE"]=="true" || props["GFOLB_MMODE"]=="TRUE")
		isDefault = true;
	if(props["REQ_TYPE"]=="text" || props["REQ_TYPE"]=="TEXT")
		isText = true;
	hdrdelm = props["REQ_DEF_DEL"];
	boost::replace_all(hdrdelm,"\\r","\r");
	boost::replace_all(hdrdelm,"\\n","\n");
	cntlnhdr = props["REQ_CNT_LEN_TXT"];
	if(props["BFM_INC_LEN"]=="true" || props["BFM_INC_LEN"]=="TRUE")
		isLengthIncluded = true;
	if(!isText && props["BFM_LEN"]=="")
	{
		cout << "invalid message length specified in BFM_LEN" << endl;
		exit(0);
	}
	else
	{
		try
		{
			bfmlen = boost::lexical_cast<int>(props["BFM_LEN"]);//Connection pool size to the servers
		}
		catch(...)
		{
			cout << "invalid message length specified in BFM_LEN" << endl;
			exit(0);
		}
		if(bfmlen>4)
		{
			cout << "invalid message length specified in BFM_LEN should be less than or equal to 4" << endl;
			exit(0);
		}
	}
	if(isText)
	{
		cout << "Protocol Type = text" << endl;
		cout << "Header delimiter = " << hdrdelm << endl;
		cout << "Content length header label = " <<  cntlnhdr << endl;
	}
	else
	{
		cout << "Protocol Type = binary" << endl;
		cout << "Binary message length = " << bfmlen << endl;
		cout << "Includes Length = " << props["BFM_INC_LEN"] << endl;
	}
	if(isDefault)
		cout << "Module Type = default" << endl;
	else
		cout << "Protocol Type = " << props["GFOLB_MMODE"] << endl;
	if(isSSLEnabled)
		cout << "SSL is enabled" << endl;
	else
		cout << "SSL is disabled" << endl;
	//boost::thread r_thread(boost::bind(&readRequests,this));
	//boost::thread g_thread(boost::bind(&generateRequest,this));
}

BufferedReader::~BufferedReader() {
	// TODO Auto-generated destructor stub
}
