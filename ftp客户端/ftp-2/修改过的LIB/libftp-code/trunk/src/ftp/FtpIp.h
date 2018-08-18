#ifndef FTPIP_H
#define FTPIP_H

struct FtpPrivate;

struct FtpIp{
	struct FtpIpPrivate *private;
	char **ip;
	unsigned int size;
	int *version;
};

struct FtpIp* ftpGetAddresses(const char *hostname,const char *port);
void ftpFreeIp(struct FtpIp *ip);

#endif
