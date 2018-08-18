/*
 * app-udp-send.h
 *
 *  Created on: 2012-12-17
 */

#ifndef __APP_UDP_SEND_H__
#define __APP_UDP_SEND_H__
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <errno.h>

extern int UdpConnectInit(unsigned int addr, int port, struct sockaddr_in *address);
extern void UdpConnectDeInit(int fd);
extern int UdpSend(int fd, struct sockaddr_in address, char *data, int datalen);
extern int UdpSend2( char *ip, int port, char *msg, int buff_len);

#endif /* __APP_UDP_SEND_H__ */
