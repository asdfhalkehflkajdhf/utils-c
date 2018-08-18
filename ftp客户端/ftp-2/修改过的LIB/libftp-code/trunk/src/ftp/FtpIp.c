//Standard C headers
#include <string.h>
#include <stdlib.h>

//Standard UNIX headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

//Library headers
#include <ftp/ftp.h>
#include <ftp/FtpIp.h>

//Library headers not part of the API
#include "FtpIpPrivate.h"

struct FtpIp* ftpAllocateIp(void){
	struct FtpIp *ip=(struct FtpIp*)malloc(sizeof(struct FtpIp));
	if(ip==NULL){
		FTP_ERROR("cannot allocate memory for FtpIp struct");
		return NULL;
	}
	memset((void *)ip,0,sizeof(struct FtpIp));
	ip->private=(struct FtpIpPrivate*)malloc(sizeof(struct FtpIpPrivate));
	if(ip->private==NULL){
		free(ip);
		FTP_ERROR("cannot allocate memory for FtpIp struct");
		return NULL;
	}
	memset((void *)ip->private,0,sizeof(struct FtpIpPrivate));
	return (struct FtpIp*)ip;
}

void ftpFreeIp(struct FtpIp *ip){
	int i;
	if(ip==NULL) return;
	if(ip->private!=NULL){
		if(ip->private->info!=NULL)
			freeaddrinfo(ip->private->info);
		free(ip->private);
	}
	if(ip->version!=NULL)
		free(ip->version);
	if(ip->ip!=NULL){
		for(i=0;i<ip->size;++i){
			if(ip->ip[i]!=NULL)
				free(ip->ip[i]);
		}
		free(ip->ip);
	}
	free(ip);
}

struct FtpIp* ftpGetAddresses(const char *hostname,const char *port){
	//hints: the hints provided to geaddrinfo
	//eg hostname, protocol, version
	//p: pointer to iterate the addinfo list 
	struct addrinfo hints,*p;
	//status: the status returned by unix api calls
	int status,i;
	//size: the number of addresses
	struct FtpIp *ip;

	//initialize hint structure to zero
	memset(&hints, 0, sizeof(hints));
	
	//family (version) unspecified
	hints.ai_family = AF_UNSPEC;
	
	//family (version) IPV4
	//hints.ai_family = AF_INET; 
	
	//family (version) IPV6
	//hints.ai_family = AF_INET6; 
	
	//socket type (protocol) TCP
	hints.ai_socktype = SOCK_STREAM;
	
	hints.ai_flags = AI_PASSIVE;   

	//get the address list
/*
int getaddrinfo(const char *hostname, const char *servname,
	const struct addrinfo *hints, struct addrinfo **res);

getaddrinfo() returns zero on success or one of the error codes listed in
gai_strerror(3) if an error occurs.

*/

	ip=ftpAllocateIp();

	if(ip==NULL){
		FTP_ERROR("cannot allocate memory for Ip struct");
		return NULL;
	}

	status=getaddrinfo(hostname,port, &hints, &ip->private->info);

	if(status){
		FTP_ERROR(gai_strerror(status));
		ftpFreeIp(ip);
		return NULL;
	}

	ip->size=0;
	for(p = ip->private->info;p != NULL; p = p->ai_next)
		++ip->size;

	ip->version=(int *)malloc(ip->size*sizeof(int));
	if(ip->version==NULL){
		FTP_ERROR("cannot allocate memory for version array");
		ftpFreeIp(ip);
		return NULL;
	}
	ip->ip=(char **)malloc(ip->size*sizeof(char *));
	if(ip->ip==NULL){
		FTP_ERROR("cannot allocate memory for ip array");
		ftpFreeIp(ip);
		return NULL;
	}
	for(i=0;i<ip->size;++i){
		ip->ip[i]=(char *)malloc(INET6_ADDRSTRLEN);
		if(ip->ip[i]==NULL){
			FTP_ERROR("cannot allocate memory for ip array");
			ftpFreeIp(ip);
			return NULL;
		}
	}
	i=0;
	for(p = ip->private->info;p != NULL; p = p->ai_next) {
			void *addr;

			// get the pointer to the address itself,
			// different fields in IPv4 and IPv6:
			if (p->ai_family == AF_INET) { // IPv4
					struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
					addr = &(ipv4->sin_addr);
					ip->version[i]=FTP_IPV4;
			} else { // IPv6
					struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
					addr = &(ipv6->sin6_addr);
					ip->version[i]=FTP_IPV6;
			}

			// convert the IP to a string and print it:
			inet_ntop(p->ai_family, addr, ip->ip[i], INET6_ADDRSTRLEN);
			++i;
	}
	return ip;
}
