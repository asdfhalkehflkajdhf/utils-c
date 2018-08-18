#ifndef FTP_H
#define FTP_H


enum {
	PORT,
	PASV,
};

#define FTP_DEBUG

#ifdef FTP_DEBUG

#define FTP_ERROR(err) ftpError(__FILE__,__LINE__,err);
#define FTP_ERROR2(err, ...) printf(err, ##__VA_ARGS__);

#else

#define FTP_ERROR(err) ;
#define FTP_ERROR2(err, ...) ;

#endif

#define FTP_IPV4 AF_INET
#define FTP_IPV6 AF_INET6
#define FTP_TYPE_ASCII 0
#define FTP_TYPE_IMAGE 1
#define FTP_TYPE_EBCDIC 2


#include <ftp/FtpIp.h>
#include <ftp/FtpConnection.h>


void ftpError(const char *filename,const int linenumber,const char * err);

#endif
