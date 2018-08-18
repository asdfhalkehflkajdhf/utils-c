/*************PAVE**************PEI *****************8.15*******************/

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

int pasv_t(int sockfd)
{
	char buf[128];
	char str[128];
	int i;
	int n;
	strcpy(str,"PASV\r\n");
	Write(sockfd, str, strlen(str));
	n = read(sockfd, buf, 128);
	Write(STDOUT_FILENO, buf, n);
/*
	strcpy(str,"PASV\r\n");
	write(sockfd, str, strlen(str));
	n = read(sockfd, buf, 128);
	write(STDOUT_FILENO, buf, n);
*/

	int nn=0;
	int mm=0;
	strtok(buf,",");
	for(i=0;i<4;i++){

		if(i==3){
			nn=atoi(strtok(NULL,","));
			mm=atoi(strtok(NULL,","));
//			printf("%d\n",nn);
//			printf("%d\n",mm);
			break;
		}
		strtok(NULL,",");
	}
	int nnn=nn*256+mm;
//	printf("%d\n",nnn);
	
	return nnn;
}

int socket_t1(int proj)
{
	static struct sockaddr_in servaddr1;
	char buf[128];
	int sockfd;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr1, sizeof(servaddr1));
	servaddr1.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &servaddr1.sin_addr);
	servaddr1.sin_port = htons(proj);
	Connect(sockfd, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	bzero(buf, sizeof(buf));
	return sockfd;
}
