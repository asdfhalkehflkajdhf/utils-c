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


int socket_t(int port)
{
	static struct sockaddr_in servaddr;
	char buf[128];
	int sockfd, n;
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	n = Read(sockfd, buf, 128);
	Write(STDOUT_FILENO, buf, n);
	bzero(buf, sizeof(buf));
	return sockfd;
}
