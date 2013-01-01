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

void BufferedReader::closeSSL(int sockfd,SSL *ssl,BIO* bio)
{
	int r=SSL_shutdown(ssl);
	if(!r){
	  /*If we called SSL_shutdown() first then
	  we always get return value of '0'. In
	  this case, try again, but first send a
	  TCP FIN to trigger the other side's
	  close_notify*/
	  shutdown(sockfd,1);
	  r=SSL_shutdown(ssl);
	}
	switch(r){
	  case 1:
		break; /* Success */
	  case 0:
	  case -1:
	  default:
		  logger << "shutdown failed" << flush;
	}
	BIO_free(bio);
	SSL_free(ssl);
}

void BufferedReader::erase(int fd)
{
	this->p_mutex.lock();
	this->fds.erase(fd);
	this->q.erase(fd);
	if(isSSLEnabled)
	{
		this->sslConns->erase(fd);
	}
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
			sbio=BIO_new_socket(fd,BIO_CLOSE);
			ssl=SSL_new(ctx);
			SSL_set_bio(ssl,sbio,sbio);

			io=BIO_new(BIO_f_buffer());
			ssl_bio=BIO_new(BIO_f_ssl());
			BIO_set_ssl(ssl_bio,ssl,BIO_CLOSE);
			BIO_push(io,ssl_bio);

			int r = SSL_accept(ssl);
			SSL_get_error(ssl,r);
			if(r<=0)
			{
				logger << "SSL accept error";
				this->fds[fd]=true;
				return alldat;
			}

			int er=-1;
			bool flag = true;
			while(flag)
			{
				er = BIO_gets(io,buf,BUFSIZZ-1);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_WANT_READ:
					{
						logger << "more to read error";
						break;
					}
					case SSL_ERROR_WANT_WRITE:
					{
						logger << "more to write error";
						break;
					}
					case SSL_ERROR_NONE:
					{
						break;
					}
					case SSL_ERROR_ZERO_RETURN:
					{
						logger << "SSL - connection closed";
						this->fds[fd]=true;
						return alldat;
					}
					default:
					{
						logger << "SSL read problem";
						this->fds[fd]=true;
						return alldat;
					}
				}
				if(!strcmp(buf,this->hdrdelm.c_str()))
				{
					string tt(buf, er);
					alldat += tt;
					break;
				}
				string temp(buf, er);
				temp = temp.substr(0,temp.length()-1);
				if(temp.find("Host:")!=string::npos)
				{
					temp = "Host: {REPLACE_HOST_f2079930-4a8b-11e2-bcfd-0800200c9a66}";
				}
				alldat += (temp + "\n");
				if(temp.find(this->cntlnhdr)!=string::npos)
				{
					std::string cntle = temp.substr(temp.find(": ")+2);
					cntle = cntle.substr(0,cntle.length()-1);
					try
					{
						cntlen = CastUtil::lexical_cast<int>(cntle);
					}
					catch(...)
					{
						logger << "bad lexical cast" <<endl;
					}
				}
				memset(&buf[0], 0, sizeof(buf));
			}
			//Check if content length is present, if yes get the content
			while(cntlen>0)
			{
				er = BIO_read(io,buf,cntlen);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_WANT_READ:
					{
						logger << "more to read error";
						break;
					}
					case SSL_ERROR_WANT_WRITE:
					{
						logger << "more to write error";
						break;
					}
					case SSL_ERROR_NONE:
						cntlen -= er;
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						logger << "SSL - connection closed";
						this->fds[fd]=true;
						return alldat;
					}
					default:
					{
						logger << "SSL error problem";
						this->fds[fd]=true;
						return alldat;
					}
				}
				string temp(buf, er);
				alldat += temp;
				memset(&buf[0], 0, sizeof(buf));
			}
			SSLConnection sslConn;
			sslConn.ssl = ssl;
			sslConn.io = io;
			sslConns->insert(pair<int, SSLConnection>(fd, sslConn));
			return alldat;
		}
		else
		{
			int er=-1;
			sbio=BIO_new_socket(fd,BIO_NOCLOSE);
			io=BIO_new(BIO_f_buffer());
			BIO_push(io,sbio);
			while(true)
			{
				er = BIO_gets(io,buf,BUFSIZZ-1);
				if(er==0)
				{
					this->fds[fd]=true;
					if(io!=NULL)BIO_free_all(io);
					logger << "\nsocket closed before being serviced" <<flush;
					return alldat;
				}
				if(!strcmp(buf,this->hdrdelm.c_str()) || er<0)
				{
					string tt(buf, er);
					alldat += tt;
					break;
				}
				string temp(buf, er);
				//if(temp.find("Range:")==0 || temp.find("If-Range:")==0)
				//	continue;
				temp = temp.substr(0,temp.length()-1);
				if(temp.find("Host:")!=string::npos)
				{
					temp = "Host: {REPLACE_HOST_f2079930-4a8b-11e2-bcfd-0800200c9a66}";
				}
				alldat += (temp + "\n");
				if(temp.find(this->cntlnhdr)!=string::npos)
				{
					std::string cntle = temp.substr(temp.find(": ")+2);
					cntle = cntle.substr(0,cntle.length()-1);
					try
					{
						cntlen = CastUtil::lexical_cast<int>(cntle);
					}
					catch(...)
					{
						logger << "bad lexical cast" <<endl;
					}
				}

				memset(&buf[0], 0, sizeof(buf));
			}
			while(cntlen>0)
			{
				er = BIO_read(io,buf,cntlen);
				if(er==0)
				{
					this->fds[fd]=true;
					if(io!=NULL)BIO_free_all(io);
					logger << "\nsocket closed before being serviced" <<flush;
					return alldat;
				}
				else if(er>0)
				{
					string temp(buf, er);
					alldat += temp;
					memset(&buf[0], 0, sizeof(buf));
					cntlen -= er;
				}
			}
			if(io!=NULL)BIO_free_all(io);
			return alldat;
		}
	}
	else
	{
		if(isSSLEnabled)
		{
			sbio=BIO_new_socket(fd,BIO_CLOSE);
			ssl=SSL_new(ctx);
			SSL_set_bio(ssl,sbio,sbio);

			io=BIO_new(BIO_f_buffer());
			ssl_bio=BIO_new(BIO_f_ssl());
			BIO_set_ssl(ssl_bio,ssl,BIO_CLOSE);
			BIO_push(io,ssl_bio);

			int r = SSL_accept(ssl);
			SSL_get_error(ssl,r);
			if(r<=0)
			{
				logger << "SSL accept error";
				this->fds[fd]=true;
				return alldat;
			}

			int er=-1;
			if(bfmlen>0)
			{
				er = BIO_read(io,buf,bfmlen);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_WANT_READ:
					{
						logger << "more to read error";
						break;
					}
					case SSL_ERROR_WANT_WRITE:
					{
						logger << "more to write error";
						break;
					}
					case SSL_ERROR_NONE:
					{
						cntlen -= er;
						break;
					}
					case SSL_ERROR_ZERO_RETURN:
					{
						logger << "SSL - connection closed";
						this->fds[fd]=true;
						return alldat;
					}
					default:
					{
						logger << "SSL read problem";
						this->fds[fd]=true;
						return alldat;
					}
				}
				for(int i=0;i<er;i++)
					alldat.push_back(buf[i]);
				memset(&buf[0], 0, sizeof(buf));
			}
		}
		else
		{
			int er=-1;
			while(bfmlen>0)
			{
				sbio=BIO_new_socket(fd,BIO_NOCLOSE);
				io=BIO_new(BIO_f_buffer());
				BIO_push(io,sbio);
				er = BIO_read(io,buf,bfmlen);
				if(er==0)
				{
					this->fds[fd]=true;
					logger << "\nsocket closed before being serviced" <<flush;
					return alldat;
				}
				else if(er>0)
				{
					bfmlen -= er;
					for(int i=0;i<er;i++)
						alldat.push_back(buf[i]);
					memset(&buf[0], 0, sizeof(buf));
				}
			}
		}
		int lengthm = getLength(alldat,bfmlen);
		if(isLengthIncluded)
		{
			lengthm -= bfmlen;
		}
		logger << lengthm << flush;
		if(isSSLEnabled)
		{
			int er=-1;
			while(lengthm>0)
			{
				er = BIO_read(io,buf,lengthm);
				switch(SSL_get_error(ssl,er))
				{
					case SSL_ERROR_NONE:
						lengthm -= er;
						break;
					case SSL_ERROR_ZERO_RETURN:
					{
						logger << "SSL - connection closed";
						this->fds[fd]=true;
						return alldat;
					}
					default:
					{
						logger << "SSL read problem";
						this->fds[fd]=true;
						return alldat;
					}
				}

				for(int i=0;i<er;i++)
					alldat.push_back(buf[i]);
				memset(&buf[0], 0, sizeof(buf));
			}
			SSLConnection sslConn;
			sslConn.ssl = ssl;
			sslConn.io = io;
			sslConns->insert(pair<int, SSLConnection>(fd, sslConn));
		}
		else
		{
			int er=-1;
			while(lengthm>0)
			{
				er = BIO_read(io,buf,lengthm);
				if(er==0)
				{
					this->fds[fd]=true;
					logger << "\nsocket closed before being serviced" <<flush;
					return "";
				}
				else if(er>0)
				{
					lengthm -= er;
					for(int i=0;i<er;i++)
					{
						alldat.push_back(buf[i]);
					}
					memset(&buf[0], 0, sizeof(buf));
				}
			}
			if(io!=NULL)BIO_free_all(io);
		}
	}
	return alldat;
}

BufferedReader::BufferedReader(propMap props, SSL_CTX *ctx)
{
	logger = Logger::getLogger("BufferedReader");
	isSSLEnabled = false;
	if(props["SSL_ENAB"]=="true" || props["SSL_ENAB"]=="TRUE")
		isSSLEnabled = true;
	isDefault = false;
	if(props["GFOLB_MMODE"]=="true" || props["GFOLB_MMODE"]=="TRUE")
		isDefault = true;
	isText = false;
	if(props["REQ_TYPE"]=="text" || props["REQ_TYPE"]=="TEXT")
		isText = true;
	hdrdelm = props["REQ_DEF_DEL"];
	StringUtil::replaceAll(hdrdelm,"\\r","\r");
	StringUtil::replaceAll(hdrdelm,"\\n","\n");
	cntlnhdr = props["REQ_CNT_LEN_TXT"];
	isLengthIncluded = false;
	if(props["BFM_INC_LEN"]=="true" || props["BFM_INC_LEN"]=="TRUE")
		isLengthIncluded = true;
	if(!isText && props["BFM_LEN"]=="")
	{
		logger << "invalid message length specified in BFM_LEN";
		exit(0);
	}
	else
	{
		try
		{
			bfmlen = CastUtil::lexical_cast<int>(props["BFM_LEN"]);//Connection pool size to the servers
		}
		catch(...)
		{
			logger << "invalid message length specified in BFM_LEN";
			exit(0);
		}
		if(bfmlen>4)
		{
			logger << "invalid message length specified in BFM_LEN should be less than or equal to 4";
			exit(0);
		}
	}
	if(isText)
	{
		logger << "Protocol Type = text";
		logger << "Header delimiter = " << hdrdelm;
		logger << "Content length header label = " <<  cntlnhdr;
	}
	else
	{
		logger << "Protocol Type = binary";
		logger << "Binary message length = " << bfmlen;
		logger << "Includes Length = " << props["BFM_INC_LEN"];
	}
	if(isDefault)
		logger << "Module Type = default";
	else
		logger << "Protocol Type = " << props["GFOLB_MMODE"];
	if(isSSLEnabled)
	{
		this->ctx = ctx;
		sslConns = new map<int, SSLConnection>();
		logger << "SSL is enabled";
	}
	else
		logger << "SSL is disabled";
}

BufferedReader::~BufferedReader() {
}
