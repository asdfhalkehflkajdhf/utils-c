#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp.h"


int main(int rgvs, char **rgva)
{
	
	int ret;
	int len;
	int sckcli;
	int i=0;
	char buff[1024];
	if(rgvs <4){
		printf("%s <destip> <destport> <conect time>\n", rgva[0]);
		return 0;		
	}

	
	for(i=0;i<atoi(rgva[3]); i++)
	{
		sckcli = ConnectRemoteBlock(rgva[1], atoi(rgva[2]));
		if(INVALID_SOCKET == sckcli)
		{
			printf("%s conect %s %s error \n", rgva[0], rgva[1], rgva[2]);
			return 0;
		}			
		sprintf(buff, "%d",i);
		printf("Enter string to send: %s\n", buff);

		len=WritenBlock(sckcli,buff,strlen(buff),10);
		len=ReadnBlock(sckcli,buff,BUFSIZ,10);
		if(len < 0){
			printf("read error\n");
		}else{
			buff[len]='\0';
			printf("received:%s\n", buff);			
		}

		close(sckcli);			
	}
		

		
	return 0;
	
}