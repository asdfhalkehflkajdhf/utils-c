#ifndef FTPCONNECTION_H
#define FTPCONNECTION_H
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

struct FtpIp;

struct FtpConnection{
	int socket;
	int replyCode;
	char replyCodeString[5];
	int passive;
	struct sockaddr_in myctladdr;
	char destIp[16];
	char passiveIp[16];
	char passivePort[6];
	struct FtpConnection *passiveConnection;
	char *receiveBuffer;
	size_t receiveBufferEnd,receiveBufferStart,receiveBufferSize;
	char multiline;
	int connected;
	char *transmitBuffer;
	size_t transmitBufferSize;
	char system[100];
};

struct FtpFilenameList{
	char *filename;
	struct FtpFilenameList *next;
};

//Functions to allocate/free FtpConnection structures and connect to hosts
struct FtpConnection* ftpConnect(struct FtpIp* address);
struct FtpConnection *ftpConnectHost(const char *hostname,const char *port);
void ftpDisconnect(struct FtpConnection *connection);

//Functions for I/O
int ftpReadData(struct FtpConnection *connection,char *buffer,unsigned int size);
int ftpWriteData(struct FtpConnection *connection,char *buffer,unsigned int size);
int ftpWrite(struct FtpConnection *connection,const char *string);
int ftpWriteSensitive(struct FtpConnection *connection,const char *string);

//Ftp commands
//login to server
int ftpLogin(struct FtpConnection *connection,const char *username,const char *password);
//Change Working Directory
int ftpCwd(struct FtpConnection *connection,char *directory);
//Change to parent directory
int ftpCdUp(struct FtpConnection *connection);
//Reinitialize connection
int ftpReinitialize(struct FtpConnection *connection);
//Upload local file to server
int ftpStore(struct FtpConnection *connection,const char *file,const char *localFile);
//Quit, logout. The server closes the connection
int ftpQuit(struct FtpConnection *connection);
//Perform a directory listing
int ftpList(struct FtpConnection *connection);
//Perform a directory scan
struct FtpFilenameList *ftpDirectoryScan(struct FtpConnection *connection);

//Download remote file
int ftpRetrieve(struct FtpConnection *connection,const char *file,const char *localFile);
//Delete file
int ftpDelete(struct FtpConnection *connection,const char *filename);

//enter passive mode
int ftpPassive(struct FtpConnection *connection);

int ftpSystem(struct FtpConnection *connection);

/*int ftpAbord(struct FtpConnection *connection);
int ftpAccount(struct FtpConnection *connection);
int ftpAuthData(struct FtpConnection *connection);
int ftpAllocate(struct FtpConnection *connection);
int ftpAppend(struct FtpConnection *connection);
int ftpAuthenticate(struct FtpConnection *connection);
int ftpClearCommnadChannel(struct FtpConnection *connection);
int ftpConfidentialityProtection(struct FtpConnection *connection);
*/

/*int ftpDelete(struct FtpConnection *connection,char *directory);
int ftpPrivacyChannel(struct FtpConnection *connection);
int ftpExtendedAddress(struct FtpConnection *connection);*/


int ftpGetReply(struct FtpConnection *connection);


#endif
