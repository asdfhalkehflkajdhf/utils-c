/*****************************input user   by pei 8.15*******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include "main.h"

int user_t(int sockfd)
{
	char buf[N];
	int  n;
	char str[N];
	int i;

	fgets(buf,N,stdin);
	strcpy(str,"user ");
	for(i = 0;buf[i]!= '\0';i++){
		str[i+5] = buf[i];
	}
	
//	strcpy(str,"user gaozhuang\r\n");
	write(sockfd, str, strlen(str));
	n = read(sockfd, buf, N);
	write(STDOUT_FILENO, buf, n);
	bzero(buf, sizeof(buf));
	bzero(str, sizeof(str));

	return 0;
}

void pass_t(int sockfd)
{
	char buf[128];
	char str[128];
	int i;
	int n;

	fgets(buf,N,stdin);
	strcpy(str,"pass ");
	for(i = 0;buf[i]!= '\0';i++){
		str[i+5] = buf[i];
	}
//	strcpy(str,"pass 3021033\r\n");
	write(sockfd, str, strlen(str));
	n = read(sockfd, buf, 128);
	write(STDOUT_FILENO, buf, n);
	bzero(buf, sizeof(buf));
	bzero(str, sizeof(str));

}
