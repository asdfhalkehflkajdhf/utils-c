#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>


#include <ftp/ftp.h>




void ftpError(const char *filename,const int linenumber,const char * err){
	fprintf(stderr,"%s:%d: %s\n",filename,linenumber,err);
}




int main(int argc, char *argv[]){
	char *hostname=NULL;
/**/
	if(argc!=2){
		printf("Usage: %s hostname\n",argv[0]);
		return -1;
	}
	hostname = argv[1];
	struct FtpConnection *connection=ftpConnectHost(hostname,"21");
	if(connection!=NULL){
		printf("%s\n",connection->passiveIp);
		ftpLogin(connection,"liuzc","lzc123");
		/*
		ftpCwd(connection, "aa");
		ftpCwd(connection, "..");
		*/
		//ftpWrite(connection, "RNFR aa/abc.txt");
		//ftpWrite(connection, "RNTO aa/qwe.txt");
		ftpSetPassive(connection,PASV);
		ftpStore(connection, "abc.txt", "/workspace/t/libftp-code/trunk/src/ftp/ftp.txt");
		ftpStore(connection, "abc2.txt", "/workspace/t/libftp-code/trunk/src/ftp/ftp.txt");
		ftpCwd(connection, "aa");
		ftpList(connection);

		//ftpList(connection);
		ftpDisconnect(connection);
	}
	return 0;

}
