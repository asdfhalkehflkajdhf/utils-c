#ifndef __DOWNFILE_H
#define __DOWNFILE_H

 /* mifg 命令接口  sufg 数据接口    q  1.c */
#define IP 	"192.168.0.100"
#define N	1024

#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

void downfile(int mifg,int sufg,char *q);
int user_t(int sockfd);
int socked_t(int port);
int socked_t1(int );
void pass_t(int sockfd);
int pasv_t(int sockfd);
//int  ftp_download(int cmdfd,int datfd,char *s);
int ftp_downloaw(char *p,int sockfd,int sockfdd);
int cmd(int sockfd,char * str);
int cmd1(int sockfd,char * str);
//void ftp_cmd(char *stt,char *cmd,char *filename);
int ftp_intr_upload(int cmdfd,int datfd,char *filename);
int ftp_cmd(char *cmd);


/***************************wrap.c*******************************/
void Close(int fd);
void perr_exit(const char *s);
ssize_t Read(int fd, void *ptr, size_t nbytes);
int Socket(int family, int type, int protocol);
ssize_t Write(int fd, const void *ptr , size_t nbytes);
void Connect (int fd, const struct sockaddr *sa , socklen_t salen);
#endif
