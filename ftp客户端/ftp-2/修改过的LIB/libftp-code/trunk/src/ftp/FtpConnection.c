#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#include <ftp/ftp.h>
#include <ftp/FtpConnection.h>
#include <ftp/FtpIp.h>

#include "FtpIpPrivate.h"

//#include "regextest.h"

static const char *ftpGetReplyLine(struct FtpConnection *connection);
static char *ftpGetLine(struct FtpConnection *connection);
static int ftpParsePort(const char *string,char *ip,char *port);
static struct FtpConnection* ftpConnectRaw(struct FtpConnection *s_Fc, int mode, struct FtpIp* address);

//allocate a structure of FtpConnection
static struct FtpConnection* allocateFtpConnection(void);
//free memory allocated for FtpConnection
static void freeFtpConnection(struct FtpConnection *connection);
//UNIX implementation to get a connected socket from addrinfo.
//function blocks, until connection is established but returns if the maximum
//allowed time specified by timeoutms is exceded
static int getConnection(struct addrinfo *info,size_t timeoutms);
static int socketBlocking(int socket);
static int socketNonBlocking(int socket);


static struct FtpConnection* allocateFtpConnection(void){
	struct FtpConnection *connection=NULL;
// --- allocate FtpConnection structure --- //

	//allocate the connection object
	connection=(struct FtpConnection*)malloc(sizeof(struct FtpConnection));
	if(connection==NULL){
		FTP_ERROR("cannot allocate memory for connection");
		return NULL;
	}
	//initialize all to zero
	memset(connection,0,sizeof(struct FtpConnection));
	connection->socket=-1;
	
	//allocate the receive buffer
	connection->receiveBuffer=(char *)malloc(1024);
	if(connection->receiveBuffer==NULL){
		FTP_ERROR("cannot allocate memory for receive buffer");
		//clean up
		freeFtpConnection(connection);
		return NULL;
	}
	//initialize buffer indexes
	connection->receiveBufferSize=1024;
	connection->receiveBufferStart=connection->receiveBufferEnd=0;
	
	//allocate the transmit buffer
	connection->transmitBuffer=(char *)malloc(1024);
	if(connection->transmitBuffer==NULL){
		FTP_ERROR("cannot allocate memory for transmit buffer");
		//clean up
		freeFtpConnection(connection);
		return NULL;
	}
	connection->transmitBufferSize=1024;
	
	return connection;

}


static void freeFtpConnection(struct FtpConnection *connection){
	if(connection==NULL)
		return;

	if(connection->receiveBuffer!=NULL)
		free(connection->receiveBuffer);
		
	if(connection->transmitBuffer!=NULL)
		free(connection->transmitBuffer);
		
	if(connection->passiveConnection!=NULL)
		ftpDisconnect(connection->passiveConnection);
	
	free(connection);

}
#define	UC(b)	(((int)b)&0xff)
static int getConnectionPORT(struct FtpConnection *s_Fc,char *ca){
	int socketFd=-1;
	int on = 1;
	register unsigned char *p, *a;
	struct sockaddr_in data_addr;
// --- get socket --- //

	//get socket to host
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketFd<0){
		FTP_ERROR2("cannot get socket: %s",strerror(errno));
		return -1;
	}
	if(socketFd>=FD_SETSIZE){
		FTP_ERROR("invalid socket value");
	}
	// --- bind socket --- //
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
		FTP_ERROR("ftp: setsockopt (reuse address)");
		goto bad;
	}
	data_addr = s_Fc->myctladdr;
	data_addr.sin_port = 0;
	if (bind(socketFd, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0) {
		FTP_ERROR("ftp: bind error");
		printf("%s\n", strerror(errno));
		goto bad;
	}
	socklen_t len = sizeof (data_addr);
	if (getsockname(socketFd, (struct sockaddr *)&data_addr, &len) < 0) {
		FTP_ERROR("ftp: getsockname");
		goto bad;
	}
	if (listen(socketFd, 1) < 0){
		FTP_ERROR("ftp: listen");
		goto bad;
	}
	
	a = (unsigned char *)&data_addr.sin_addr;
	short unsigned int port = htons(data_addr.sin_port);
	p = (unsigned char *)&port;
	snprintf(ca, 32, "%d,%d,%d,%d,%d,%d",
		  UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
		  UC(p[0]), UC(p[1]));
	
	return socketFd;
bad:
	(void) close(socketFd);
	return (-1);

}

static int getConnection(struct addrinfo *info,size_t timeoutms){
	int socketFd=-1;
	fd_set fdSet;
	FD_ZERO(&fdSet);
	struct timeval tv={0};
	int so_error,status;
	socklen_t so_error_size=sizeof(int);

// --- get socket --- //

	//get socket to host
	socketFd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if(socketFd==-1){
		FTP_ERROR2("cannot get socket: %s",strerror(errno));
		return -1;
	}
	if(socketFd>=FD_SETSIZE){
		FTP_ERROR("invalid socket value");
	}
	
// --- connect socket --- //

	//set socket non-blocking during connect
	status=socketNonBlocking(socketFd);
	if(status==-1){
		FTP_ERROR("cannot set socket non-blocking");
		close(socketFd);
		return -1;
	}
	//call connect non-blocking
	status=connect(socketFd, info->ai_addr, info->ai_addrlen);
	if(status){
		
		if(errno==EINPROGRESS){
/*
	The socket is non-blocking and the connection cannot be completed
	immediately. It is possible to select(2) or poll(2) for completion
	by selecting the socket for writing. After select(2) indicates
	writability, use getsockopt(2) to read the SO_ERROR option at level
	SOL_SOCKET to determine whether connect() completed successfully
	(SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual
	error codes listed here, explaining the reason for the failure). 
*/

			//connection in progress
			//select to wait for connection using timeout

			//zero file descriptor set
			FD_ZERO(&fdSet);
			//add socket to file descriptro set
			FD_SET(socketFd, &fdSet);
			//setup timeout
			tv.tv_sec=10;
			tv.tv_usec=0;
			//call select
			status=select(socketFd+1,NULL,&fdSet,NULL,&tv);
			if(status<0){
				//select failed
				FTP_ERROR2("cannot connect: %s",strerror(errno));
				return -1;
			}else if(status>0){
				//select ok. need to check SO_ERROR off socket at level SOL_SOCKET
				
				status=getsockopt(socketFd,SOL_SOCKET,SO_ERROR,&so_error,&so_error_size);
				if(status<0){
					FTP_ERROR2("cannot connect %s",strerror(errno));
					return -1;
				}
				if(so_error){
					FTP_ERROR2("cannot connect %s",strerror(errno));
					return -1;
				}
			}else{
				FTP_ERROR("cannot connect: timeout");
				return -1;
			}
		}else{
			FTP_ERROR2("cannot connect: %s",strerror(errno));
			return -1;
		}
	}
	status=socketBlocking(socketFd);
	if(status<0){
		FTP_ERROR("cannot connect: error setting socket in blocking mode");
		close(socketFd);
		return -1;
	}
	
	return socketFd;

}

static int socketBlocking(int socketFd){
	int socketFlags=-1;
	if(socketFd<0 || socketFd > FD_SETSIZE){
		FTP_ERROR("cannot set socket to blocking mode: invalid socket value");
		return -1;
	}
	socketFlags=fcntl(socketFd,F_GETFL);
	if(socketFlags<0){
		FTP_ERROR2("cannot set socket to blocking mode: %s",strerror(errno));
		return -1;
	}
	socketFlags&=~O_NONBLOCK;
	socketFlags=fcntl(socketFd,F_SETFL,socketFlags);
	if(socketFlags<0){
		FTP_ERROR2("cannot set socket to blocking mode %s",strerror(errno));
		return -1;
	}
	return 0;
}

static int socketNonBlocking(int socketFd){
	int socketFlags=-1,status=-1;
	if(socketFd<0 || socketFd > FD_SETSIZE){
		FTP_ERROR("cannot set socket to non-blocking mode: invalid socket value");
		return -1;
	}
	//get socket flags
	socketFlags=fcntl(socketFd,F_GETFL,0);
	if(socketFlags<0){
		FTP_ERROR2("cannot set socket to non-blocking mode: %s",strerror(errno));
		return -1;
	}
	//set socket flags + O_NONBLOCK flag
	status=fcntl(socketFd,F_SETFL,socketFlags|O_NONBLOCK);
	if(status<0){
		FTP_ERROR2("cannot set socket to non-blocking mode %s",strerror(errno));
		return -1;
	}
	return socketFd;
}

struct FtpConnection* ftpConnect(struct FtpIp* address){
	int replyCode;
	struct FtpConnection *connection=NULL;
	
	connection=ftpConnectRaw(NULL, PASV, address);

/*from RFC 959
            Connection Establishment
               120
                  220
               220
               421
*/

	if(connection!=NULL){
		replyCode=ftpGetReply(connection);
		if(replyCode==120){
			replyCode=ftpGetReply(connection);
			if(replyCode==220){
				connection->connected=1;
			}else{
				ftpDisconnect(connection);
				connection=NULL;
			}
		}else if(replyCode==220){
			connection->connected=1;
		}else{
			ftpDisconnect(connection);
			connection=NULL;
		}
	}
	return connection;
}

void ftpDisconnect(struct FtpConnection *connection){
	if(connection==NULL)
		return;

	if(connection->socket>=0){
		close(connection->socket);
	}
	
	freeFtpConnection(connection);

}

struct FtpConnection* ftpConnectRaw(struct FtpConnection *s_Fc, int mode, struct FtpIp* address){
// --- function variables --- /
	struct addrinfo *info=NULL;
	struct FtpConnection *connection=NULL;
	int socketFd=-1;


// --- validation --- //
	
	//validate address
	if(address==NULL){
		FTP_ERROR("cannot connect: function argument address cannot be NULL");
		return NULL;
	}
	if(address->private==NULL){
		FTP_ERROR("cannot connect: function argument address is uninitialized");
		return NULL;
	}
	
	//validate info
	info=address->private->info;
	while(info!=NULL && info->ai_family!=AF_INET){
		info=info->ai_next;
	}
	if(info==NULL){
		FTP_ERROR("cannot connect: no valid ipv4 address found");
		return NULL;
	}

	if(mode == PASV){
		socketFd=getConnection(info,10000);
		if(socketFd==-1){
			return NULL;
		}
	}else if(mode == PORT){
		char ca[64]="PORT ";
		socketFd=getConnectionPORT(s_Fc, &ca[5]);
		if(socketFd==-1){
			return NULL;
		}
		ftpWrite(s_Fc,ca);
		//ftpWrite(connection,"PASV");
		const char *constBuffer=ftpGetReplyLine(s_Fc);
		if(constBuffer==NULL){
			FTP_ERROR("error entering passive mode");
			return NULL;
		}
#ifdef FTP_DEBUG
		fputs(constBuffer,stdout);
#endif
	}
	
	connection=allocateFtpConnection();
	if(connection!=NULL){
		//asign the socket
		connection->socket=socketFd;
		//we are connected
		connection->connected=1;
	}else{
		close(socketFd);
	}
	return connection;
}

int ftpWriteData(struct FtpConnection *connection,char *buffer,unsigned int size){
	int bytesSent,totalBytesSent=0;
	if(connection==NULL){
		FTP_ERROR("cannot write data: function argument connection cannot be NULL");
		return -1;
	}
	if(size>0){
		
		if(buffer==NULL){
			FTP_ERROR("cannot write data: function argument buffer cannot be NULL when size is not zero");
			return -1;
		}
		int socket=connection->socket;

		while(totalBytesSent<size){
			bytesSent=send(socket,&buffer[totalBytesSent],size-totalBytesSent,0);
			if(bytesSent>0){
				totalBytesSent+=bytesSent;
			}else{
				FTP_ERROR2("cannot write data: %s",strerror(errno));
				return -1;
			}
		}
		return 0;
	}
	return 0;
}

int ftpReadData(struct FtpConnection *connection,char *buffer,unsigned int size){
	int bytesReceived;
	if(connection==NULL){
		FTP_ERROR("cannot read data: function argument connection cannot be NULL");
		return -1;
	}
	if(size>0){
		
		if(buffer==NULL){
			FTP_ERROR("cannot read data: function argument buffer cannot be NULL when size is not zero");
			return -1;
		}
		int socket=connection->socket;
		bytesReceived=recv(socket,buffer,size,0);
		if(bytesReceived<0){
			FTP_ERROR2("cannot read data: %s",strerror(errno));
			return -1;
		}
		return bytesReceived;
	}
	return 0;
}

int ftpCwd(struct FtpConnection *connection,char *directory){
	char *buffer;
	
	if(directory==NULL){
		FTP_ERROR("directory cannot be NULL");
		return -1;
	}

	buffer=(char *)malloc(strlen(directory)+5);
	if(buffer==NULL){
		FTP_ERROR("cannot allocate memory for buffer");
		return -1;
	}
	
	sprintf(buffer,"CWD %s",directory);
	ftpWrite(connection,buffer);
	free(buffer);
	int replyCode=ftpGetReply(connection);
	if(replyCode) return 0;
	else return -1;
}

int ftpProactive(struct FtpConnection *connection){

	//ftpParsePort(constBuffer,&connection->passiveIp[0],&connection->passivePort[0]);
	
	struct FtpIp *address=ftpGetAddresses(connection->destIp,NULL);
	if(connection->passiveConnection!=NULL){
		ftpDisconnect(connection->passiveConnection);
		connection->passiveConnection=NULL;
	}
	connection->passiveConnection=ftpConnectRaw(connection, PORT, address);
	ftpFreeIp(address);
	return 0;

}

int ftpPassive(struct FtpConnection *connection){

	if(connection->passive == PORT){
		goto portactive;
	}
	ftpWrite(connection,"PASV");
	const char *constBuffer=ftpGetReplyLine(connection);
	if(constBuffer==NULL){
		FTP_ERROR("error entering passive mode");
		return -1;
	}
#ifdef FTP_DEBUG
		fputs(constBuffer,stdout);
#endif
	ftpParsePort(constBuffer,&connection->passiveIp[0],&connection->passivePort[0]);
	
	struct FtpIp *address=ftpGetAddresses(connection->passiveIp,connection->passivePort);
	if(connection->passiveConnection!=NULL){
		ftpDisconnect(connection->passiveConnection);
		connection->passiveConnection=NULL;
	}
	connection->passiveConnection=ftpConnectRaw(NULL, PASV, address);
	ftpFreeIp(address);
	return 0;
portactive:
	return ftpProactive(connection);

}

int ftpWriteSensitive(struct FtpConnection *connection,const char *string){
	char *buffer;
	int status;
	size_t size=strlen(string);
	buffer=(char *)malloc(size+3);
	if(buffer==NULL){
		FTP_ERROR("cannot allocate memory for communication buffer");
		return -1;
	}
	strcpy(buffer,string);
	buffer[size]='\r';
	buffer[size+1]='\n';
	buffer[size+2]='\0';
	status=ftpWriteData(connection,buffer,size+2);
	free(buffer);
	return status;
}

int ftpWrite(struct FtpConnection *connection,const char *string){
	int ret;
	ret=ftpWriteSensitive(connection,string);
	return ret;
}

char *ftpGetLine(struct FtpConnection *connection){
	int bytesReceived;
	size_t size;
	int status;
	
	if(connection==NULL){
		FTP_ERROR("function argument connection cannot be NULL");
		return NULL;
	}
	
	int eol=0;
	while(!eol){//loop until we get a telnet end-of-line (\r\n)
		
		//check the buffer
		if(connection->receiveBufferStart==connection->receiveBufferEnd){
			connection->receiveBufferStart=connection->receiveBufferEnd=0;
		}
		
		if(connection->receiveBufferStart==0 && connection->receiveBufferEnd==connection->receiveBufferSize){
			//buffer full. need to realloc!
			char *newBuffer=(char *)realloc(connection->receiveBuffer,connection->receiveBufferSize+1024);
			if(newBuffer==NULL){
				//completly out of memory
				return NULL;
			}else{
				connection->receiveBuffer=newBuffer;
				connection->receiveBufferSize+=1024;
			}
		}
		if(connection->receiveBufferStart>connection->receiveBufferSize/2){
			memmove(
				connection->receiveBuffer,
				&connection->receiveBuffer[connection->receiveBufferStart],
				connection->receiveBufferEnd-connection->receiveBufferStart);

			connection->receiveBufferEnd=connection->receiveBufferEnd-connection->receiveBufferStart;
			connection->receiveBufferStart=0;

		}
		
		char *buffer=&connection->receiveBuffer[connection->receiveBufferStart];
		char *end=&connection->receiveBuffer[connection->receiveBufferEnd];

		int state=0;
		int eol=0;
		while(buffer!=end && !eol){
			switch(state){
			case 0:
				if(*buffer=='\r') state=1;
				break;
			case 1:
				if(*buffer=='\n') eol=1;
				else state=0;
				break;
			}
			++buffer;
		}
		
		if(eol){
			//calculate consumed size
			size=buffer-&connection->receiveBuffer[connection->receiveBufferStart];
			
			//fixup the line
			--buffer;
			*buffer='\0';
			--buffer;
			*buffer='\n';
			
			char *ret=&connection->receiveBuffer[connection->receiveBufferStart];
			connection->receiveBufferStart+=size;
			return ret;
			
		}else{
			//need more data
			int socket=connection->socket;
			
			fd_set fdSet;
			struct timeval tv;
			tv.tv_sec=10;
			tv.tv_usec=0;
			FD_ZERO(&fdSet); 
			FD_SET(socket, &fdSet);
			status=select(socket+1,&fdSet,NULL,NULL,&tv);
			if(status<0){
				FTP_ERROR(strerror(errno));
				return NULL;
			}else if(status==0){
				FTP_ERROR("timeout");
				return NULL;
			}
			bytesReceived=recv(socket,&connection->receiveBuffer[connection->receiveBufferEnd],connection->receiveBufferSize-connection->receiveBufferEnd,0);
			
			if(bytesReceived<0){
				FTP_ERROR(strerror(errno));
				return NULL;
			}
			
			if(!bytesReceived)
				return NULL;

			connection->receiveBufferEnd+=bytesReceived;
		}
	}
	FTP_ERROR("buffer overflow");
	return NULL;
}

const char *ftpGetReplyLine(struct FtpConnection *connection){
	char *buffer=ftpGetLine(connection);
	if(buffer==NULL) return buffer;
	size_t size=strlen(buffer);
	
	if(connection->multiline){
		
		if(size>3 && !strncmp(buffer,connection->replyCodeString,4)){
			//got multi-line reply end
			connection->multiline=0;
		}else{
			//more lines to follow
		}
	}else{
		char c=buffer[3];
		buffer[3]='\0';
		connection->replyCode=atoi(buffer);
		buffer[3]=c;
		if(connection->replyCode){
			if(c=='-'){
				connection->multiline=1;
				strncpy(connection->replyCodeString,buffer,3);
				connection->replyCodeString[3]=' ';
				connection->replyCodeString[4]=0;
			}else if(c==' '){
				connection->multiline=0;
			}else{
				//invalid reply code
			}
		}else{
			//invalid reply code
		}
	}
	return buffer;
}

int ftpParsePort(const char *string,char *ip,char *port){
	int portHigh,portLow,iPort;
	int numberCount=0;
	const char *b=string;
	char *p;
	const char *start=NULL;
	int state=0;
	while(*b!='\0' && state!=5){
		switch(state){
		case 0:
			if(*b>='0' && *b<='9'){
				start=b;
				state=1;
			}
			break;
		case 1:
		case 2:
		case 3:
			if(*b>='0' && *b<='9'){
				++state;
			}else if(*b==','){
				state=4;
				++numberCount;
			}else if(numberCount==5){
				++numberCount;
				state=5;
			}else{
				numberCount=0;
				state=0;
			}
			break;
		case 4:
			if(*b>='0' && *b<='9'){
				state=1;
			}else{
				state=0;
			}
		}
		++b;
	}
	
	if(state>0 && state<4 && numberCount==5){
		state=5;
	}
	
	if(state==5){
		numberCount=0;
		b=start;
		while(numberCount!=4){
			if(*b==',')
				++numberCount;
			++b;
		}
		strncpy(ip,start,b-start-1);
		ip[b-start-1]='\0';
		start=b;
		p=ip;
		while(*p!='\0'){
			if(*p==',') *p='.';
			++p;
		}
		b=start;
		int high=1;
		int done=0;
		while(*b!='\0' && !done){
			if(high){
				if(*b==','){
					strncpy(port,start,b-start);
					port[b-start]='\0';
					portHigh=atoi(port);
					high=0;
					start=b;
				}
			}else{
				if(*b<'0' || *b>'9'){
					strncpy(port,start+1,b-start-1);
					port[b-start-1]='\0';
					portLow=atoi(port);
					done=1;
				}
			}
			++b;
		}
		if(!done){
			strncpy(port,start+1,b-start-1);
			port[b-start-1]='\0';
			portLow=atoi(port);
		}
		iPort=portHigh*256+portLow;
		sprintf(port,"%d",iPort);
		
	}
	return 0;
}
int ftpSetPassive(struct FtpConnection *connection, int mode)
{
	connection->passive = mode;
	return mode;
}
int ftpGetReply(struct FtpConnection *connection){
	const char *constBuffer;
	
	constBuffer=ftpGetReplyLine(connection);
	if(constBuffer==NULL){
		FTP_ERROR("error getting reply from control channel");
		return -1;
	}
	#ifdef FTP_DEBUG
		fputs(constBuffer,stdout);
	#endif
	while(connection->multiline){
		constBuffer=ftpGetReplyLine(connection);
		if(constBuffer==NULL){
			FTP_ERROR("error getting reply from control channel");
			return -1;
		}
		#ifdef FTP_DEBUG
			fputs(constBuffer,stdout);
		#endif
	}
	return connection->replyCode;
	
}

int ftpLogin(struct FtpConnection *connection,const char *username,const char *password){
	//userSize: the string length of the username
	//passSize: the string length of the password
	//size: used as a return parameter from ftpGetReplyLine
	size_t userSize,passSize,size;
	//buffer: dynamicly allocated buffer to generate the correct USER, and PASS
	//commands
	//receivedLine: buffer to copy the received line
	char *buffer;
	//count the string length of username and password
	userSize=strlen(username);
	passSize=strlen(password);
	//find which is larger and store it to size
	if(userSize>passSize){
		size=userSize;
	}else{
		size=passSize;
	}
	//allocate enough memory to hold the username/password + the command
	//(USER/PASS) and the string termination character
	buffer=(char *)malloc(size+6);
	//out of memory?
	if(buffer==NULL){
		FTP_ERROR("cannot allocate memory for comunication buffer");
		return -1;
	}
	
	//load up the buffer
	sprintf(buffer,"USER %s",username);
	//send the command
	ftpWrite(connection,buffer);
	//get the reply
	int replyCode=ftpGetReply(connection);

	//check the response
	/*valid responses according to RFC959:
		230
		530
		500, 501, 421
		331, 332
	*/
	
	switch(replyCode){
	case 230:
		//User logged in, proceed.
		free(buffer);
		return 0;
	case 202:
		//Command not implemented, superfluous at this site.
		free(buffer);
		return 0;
	case 530:
		//Not logged in.
		free(buffer);
		return 530;
	case 500:
		//Syntax error, command unrecognized.\nThis may include errors such as command line too long.
		free(buffer);
		return 500;
	case 501:
		//Syntax error in parameters or arguments.
		free(buffer);
		return 501;
	case 421:
		//Service not available, closing control connection.
		//This may be a reply to any command if the service knows it must shut down.
		free(buffer);
		return 421;
	case 331:
		//User name okay, need password.
		break;
	case 332:
		//TODO
		free(buffer);
		//Need account for login.
		FTP_ERROR("TODO: acount login not implemented yet");
		return -1;
	default:
		if(replyCode)
			return replyCode;
		else return -1;
	}
	

	//load up the buffer
	sprintf(buffer,"PASS %s",password);
	ftpWriteSensitive(connection,buffer);

	
	//dont need buffer any more. free it
	free(buffer);
	
	//Get the response
	replyCode=ftpGetReply(connection);
	
	//check the response
	/*valid responses according to RFC959:
		230
		202
		530
		500, 501, 503, 421
		332
	*/
	switch(replyCode){
	case 230:
		//User logged in, proceed.
		return 0;
	case 202:
		//Command not implemented, superfluous at this site.
		return 0;
	case 530:
		//Not logged in.
		return 530;
	case 500:
		//Syntax error, command unrecognized.
		//This may include errors such as command line too long.
		return 500;
	case 501:
		//Syntax error in parameters or arguments.
		return 501;
	case 503:
		//Bad sequence of commands.
		return 503;
	case 421:
		//Service not available, closing control connection.
		//This may be a reply to any command if the service knows it must shut down.
		return 421;
	case 332:
		//Need account for login.
		FTP_ERROR("TODO: ACCOUNT command not implemented yet");
		return -1;
	default:
		if(replyCode)
			return replyCode;
		else
			return -1;
	}

}

int ftpQuit(struct FtpConnection *connection){

	if(connection==NULL){
		FTP_ERROR("function argument connection cannot be NULL");
		return -1;
	}
	ftpWrite(connection,"QUIT");
	int replyCode=ftpGetReply(connection);
	if(replyCode==221){
		return 0;
	}else{
    return replyCode;
	}
}

int ftpNList(struct FtpConnection *connection){
	if(connection->passiveConnection==NULL){
		ftpPassive(connection);
	}
	//char cmd[1024] = {0};
	//snprintf(cmd, 1024, "NLIST %s",dir);
	ftpWrite(connection,"NLST");
	
	
	
/* RFC 959 says:
               LIST
                  125, 150
                     226, 250
                     425, 426, 451
                  450
                  500, 501, 502, 421, 530
*/
	
	int replyCode=ftpGetReply(connection);
	
	if(replyCode!=150 && replyCode!=125){
		return replyCode;
	}
	
	char *buffer;
	int bufferSize;
	struct FtpConnection *passive=connection->passiveConnection;
	buffer=passive->receiveBuffer;
	bufferSize=passive->receiveBufferSize;
	
	//size_t size;
	
	char * constBuffer=ftpGetLine(passive);
	while(constBuffer!=NULL){
		int r=regextestMatch(constBuffer);
		if(!r){
			//printf("%s",constBuffer);
		}
#ifdef FTP_DEBUG
		printf("%d %s",r, constBuffer);
#endif
		constBuffer=ftpGetLine(passive);
	}
	/*
	do{
		const char * constBuffer=ftpGetLine(passive);
		if(constBuffer==NULL){
			printf("no data\n");
		}else{
			int r=regextestMatch(constBuffer);
			if(!r){
				printf("%s",constBuffer);
			}
		}
		return 0;
		size=ftpReadData(passive,buffer,bufferSize-1);
		if(size>0){
			buffer[size]='\0';
			fputs(buffer,stdout);
		}
	}while(size>0);
	*/

	replyCode=ftpGetReply(connection);
	if(replyCode==226){
		ftpDisconnect(passive);
		connection->passiveConnection=NULL;
	}

	return 0;
	
	
}

int ftpList(struct FtpConnection *connection){
	if(connection->passiveConnection==NULL){
		ftpPassive(connection);
	}
	
	ftpWrite(connection,"LIST");
	
	
	
/* RFC 959 says:
               LIST
                  125, 150
                     226, 250
                     425, 426, 451
                  450
                  500, 501, 502, 421, 530
*/
	
	int replyCode=ftpGetReply(connection);
	
	if(replyCode!=150 && replyCode!=125){
		return replyCode;
	}
	
	char *buffer;
	int bufferSize;
	struct FtpConnection *passive=connection->passiveConnection;
	buffer=passive->receiveBuffer;
	bufferSize=passive->receiveBufferSize;
	
	//size_t size;
	
	char * constBuffer=ftpGetLine(passive);
	while(constBuffer!=NULL){
		int r=regextestMatch(constBuffer);
		if(!r){
			//printf("%s",constBuffer);
		}
#ifdef FTP_DEBUG
		printf("%d %s",r, constBuffer);
#endif
		constBuffer=ftpGetLine(passive);
	}
	/*
	do{
		const char * constBuffer=ftpGetLine(passive);
		if(constBuffer==NULL){
			printf("no data\n");
		}else{
			int r=regextestMatch(constBuffer);
			if(!r){
				printf("%s",constBuffer);
			}
		}
		return 0;
		size=ftpReadData(passive,buffer,bufferSize-1);
		if(size>0){
			buffer[size]='\0';
			fputs(buffer,stdout);
		}
	}while(size>0);
	*/

	replyCode=ftpGetReply(connection);
	if(replyCode==226){
		ftpDisconnect(passive);
		connection->passiveConnection=NULL;
	}

	return 0;
	
	
}

int ftpRetrieve(struct FtpConnection *connection,const char *file,const char *localFile){
	if(connection->passiveConnection==NULL){
		ftpPassive(connection);
	}
	
	ftpWrite(connection,"TYPE I");
	ftpGetReply(connection);
	
	char *commandBuffer;
	size_t size;
	size=strlen(file);
	commandBuffer=(char *)malloc(size+6);
	sprintf(commandBuffer,"RETR %s",file);
	
	FILE *out=fopen(localFile,"wb");
	
	ftpWrite(connection,commandBuffer);
	
	
	
/* RFC 959 says:
               LIST
                  125, 150
                     226, 250
                     425, 426, 451
                  450
                  500, 501, 502, 421, 530
*/
	
	int replyCode=ftpGetReply(connection);
	
	if(replyCode!=150 && replyCode!=125){
		return replyCode;
	}
	
	char *buffer;
	int bufferSize;
	struct FtpConnection *passive=connection->passiveConnection;
	buffer=passive->receiveBuffer;
	bufferSize=passive->receiveBufferSize;
	

	do{
		size=ftpReadData(passive,buffer,bufferSize);
		fwrite(buffer,1,size,out);
	}while(size);

	fclose(out);

	replyCode=ftpGetReply(connection);
	if(replyCode==226){
		ftpDisconnect(passive);
		connection->passiveConnection=NULL;
	}
	free(commandBuffer);

	return 0;
	
	
}

struct FtpConnection *ftpConnectHost(const char *hostname,const char *port){
	struct FtpConnection *connection;
	struct FtpIp *address=ftpGetAddresses(hostname,port);
	if(address==NULL){
		return NULL;
	}
	
	connection=ftpConnect(address);
	socklen_t len = sizeof (connection->myctladdr);
	if (getsockname(connection->socket, 
		(struct sockaddr *)&connection->myctladdr, &len) < 0) {
		return NULL;
	}
	connection->passive = PASV;
	strcpy(connection->destIp, hostname);
	ftpFreeIp(address);
	return connection;
}

int ftpCdUp(struct FtpConnection *connection){
	ftpWrite(connection,"CDUP");
	int replyCode=ftpGetReply(connection);
	if(replyCode) return 0;
	else return -1;
}

int ftpReinitialize(struct FtpConnection *connection){
	ftpWrite(connection,"REIN");
	int replyCode=ftpGetReply(connection);
	if(replyCode) return 0;
	else return -1;
}

int ftpStore(struct FtpConnection *connection,const char *file,const char *localFile){
	
	if(connection->passiveConnection==NULL){
		ftpPassive(connection);
	}
	
	ftpWrite(connection,"TYPE I");
	ftpGetReply(connection);
	
	char *commandBuffer;
	size_t size;
	size=strlen(file);
	commandBuffer=(char *)malloc(size+6);
	sprintf(commandBuffer,"STOR %s",file);
	
	FILE *in=fopen(localFile,"rb");
	
	ftpWrite(connection,commandBuffer);
	
	
	
/* RFC 959 says:
               LIST
                  125, 150
                     226, 250
                     425, 426, 451
                  450
                  500, 501, 502, 421, 530
*/
	
	int replyCode=ftpGetReply(connection);
	
	if(replyCode!=150 && replyCode!=125){
		return replyCode;
	}
	
	char *buffer;
	struct FtpConnection *passive=connection->passiveConnection;
	buffer=passive->transmitBuffer;
	int bufferSize=passive->transmitBufferSize;
	

	size=fread(buffer,1,bufferSize,in);
	while(size){
		ftpWriteData(passive,buffer,size);
		size=fread(buffer,1,bufferSize,in);
	}
	
	ftpDisconnect(passive);
	connection->passiveConnection=NULL;
	
	fclose(in);

	replyCode=ftpGetReply(connection);
	free(commandBuffer);

	return 0;
}

int ftpDelete(struct FtpConnection *connection,const char *filename){
	char *buffer;
	size_t size=strlen(filename);
	buffer=(char *)malloc(size+6);
	sprintf(buffer,"DELE %s",filename);
	ftpWrite(connection,buffer);
	ftpGetReply(connection);
	free(buffer);
	return connection->replyCode==250 ? 0 : connection->replyCode;
}

struct FtpFilenameList *ftpDirectoryScan(struct FtpConnection *connection){
	if(connection->passiveConnection==NULL){
		ftpPassive(connection);
	}
	
	ftpWrite(connection,"NLST");
	
	
	
/* RFC 959 says:
               LIST
                  125, 150
                     226, 250
                     425, 426, 451
                  450
                  500, 501, 502, 421, 530
*/
	
	int replyCode=ftpGetReply(connection);
	
	if(replyCode!=150 && replyCode!=125){
		return NULL;
	}
	
	char *buffer;
	struct FtpConnection *passive=connection->passiveConnection;
	
	struct FtpFilenameList *listStart,*currentItem,*newItem;
	listStart=currentItem=NULL;
	
	size_t size;
	buffer=ftpGetLine(passive);
	while(buffer!=NULL){
		size=strlen(buffer);
		buffer[size-1]='\0';
		newItem=(struct FtpFilenameList *)malloc(sizeof(struct FtpFilenameList));
		newItem->next=NULL;
		newItem->filename=(char *)malloc(size);
		strcpy(newItem->filename,buffer);
		if(listStart==NULL){
			currentItem=listStart=newItem;
		}else{
			currentItem->next=newItem;
			currentItem=newItem;
		}
		buffer=ftpGetLine(passive);
	}

	replyCode=ftpGetReply(connection);
	if(replyCode==226){
		ftpDisconnect(passive);
		connection->passiveConnection=NULL;
	}

	return listStart;
}

int ftpSystem(struct FtpConnection *connection){
	ftpWrite(connection,"SYST");
	const char *constBuffer=ftpGetReplyLine(connection);
	if(constBuffer==NULL) return -1;
	size_t size=strlen(constBuffer);
	if(size>99-4){
		size=99-4;
	}
	strncpy(connection->system,&constBuffer[4],size);
	connection->system[size-1]=0;
	return 0;
}
