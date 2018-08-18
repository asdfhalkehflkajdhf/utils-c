/*********************2011.08.17 bye pei*******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "main.h"

int main(int argc, char *argv[])
{
	char buf[1024];
	int sockfd, n;
	char str[1024];
	int sockfdd;
	int proj;
	sockfd = socket_t(21);
	user_t(sockfd);
	pass_t(sockfd);

	while(1)
	{

		proj = pasv_t(sockfd);
		sockfdd= socket_t1(proj);
		bzero(buf, sizeof(buf));
		bzero(str, sizeof(str));

		printf("ftp> ");
		switch(ftp_cmd(str)){
			case 1:

				Write(sockfd, "LIST\r\n", 6);
				n = Read(sockfd, buf,1024);
				//write(STDOUT_FILENO, buf, n); 
				n = Read(sockfdd, buf,1024);
				Write(STDOUT_FILENO, buf, n); 

				n = Read(sockfd, buf,1024);
				//write(STDOUT_FILENO, buf, n); 
				Close(sockfdd);
				break;
			case 2:
				ftp_downloaw(str,sockfd,sockfdd);
				n = Read(sockfd, buf,1024);
				Write(STDOUT_FILENO, buf, n); 
				break;
			case 3:
				ftp_intr_upload(sockfd,sockfdd,str);
				n = read(sockfd, buf,1024);
				write(STDOUT_FILENO, buf, n); 
				break;
			case 4:
				Write(sockfd,"QITE\n",7);
				Close(sockfdd);
				return 0;
			default: 
				Close(sockfdd);
				break;

		}
	}
	Close(sockfdd);
	Close(sockfd);
	return 0;
}

