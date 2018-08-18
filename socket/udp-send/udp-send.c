/*
 * app-udp-send.c
 *
 *  Created on: 2014-7-11
 */
 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "app-udp-send.h"

int UdpConnectInit(unsigned int addr, int port, struct sockaddr_in *address)
{
	int fd;
  
    address->sin_family=AF_INET;  
    address->sin_addr.s_addr=addr;
    address->sin_port=htons(port);  
	bzero(&(address->sin_zero), 8);
    
    fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd < 0 ){
		printf("faild udp socket init\n");
		return -1;
	}
	return fd;
}
void UdpConnectDeInit(int fd)
{
	close(fd);
}
int UdpSend(int fd, struct sockaddr_in address, char *data, int datalen)
{
	int ret;
	if(fd == -1){
		return -1;
	}
    ret = sendto(fd,data,datalen,0,(struct sockaddr *)&address,sizeof(struct sockaddr_in));
	if(EBADF == ret || ENOTSOCK == ret){
		return -1;
	}
	return 0;
}
int UdpSend2( char *ip, int port, char *msg, int buff_len)
{

	int sockfd = 0;
	struct sockaddr_in server_addr;
	int sin_size = sizeof(struct sockaddr_in);
	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		return -1;
	}
	server_addr.sin_family =AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	bzero(&(server_addr.sin_zero), 8);
	sendto(sockfd, msg, buff_len, 0, (struct sockaddr *)&server_addr, sin_size);
	close(sockfd);
	return 0;
}

