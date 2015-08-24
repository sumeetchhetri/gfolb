#Configuration of the Generic Load Balancer and Fail Over

# Configuration #

gfolb has a configuration file named gfolb.properties namely,

```
#Is the connection to server pool persistent or non-persistent (HTTP connections are non-persistent)
CONN_PERS=false
#Servers in the pool support text based protocol(HTTP/SIP) or binary protocols (SMPP/ISO-8583)
REQ_TYPE=text
#If servers in the pool support text based protocols, how are headers in the request delimited
(Http \r\n is the delimiter)
REQ_DEF_DEL=\r\n
#If servers in the pool support text based protocols, which header has the length of the actual content
REQ_CNT_LEN_TXT=Content-Length: 
#Protocol type, not used currently
PROT_TYPE=tcp
#List of server host names/ip addresses and port numbers in the pool separated by ';' 
SERV_ADDRS=google.com:80;yahoo.com:80
#The GFOLB mode namely 
#OR->Online Routing
#FO->Fail-over
#LB->Load Balancer
GFOLB_MODE=OR
#Module name, not used currently
GFOLB_MMODE=default
#GFOLB SSL support enabled/disabled
SSL_ENAB=false
#GFOLB Server port number
GFOLB_PORT=8088
#GFOLB administration port, where commands can be fired for online routing mode
GFOLB_ADMIN_PORT=8089
#In case of binary protocols, the length of the request message is defined in the initial bytes of the message, 
in such cases the Length header size (For SMPP length header value is 4)
BFM_LEN=4
#For binary protocols does the length header include itself in the message size
BFM_INC_LEN=true
#The Server pool size
SPOOL_SIZ=10
```