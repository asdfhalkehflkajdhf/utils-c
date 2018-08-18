#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "tcp.h"


void* work(void* arg)
{
    int sckcli = *(int*) arg;
	char buff[1024];
	int ret;


	
	ret = Readn( sckcli, buff, 1024, 3, 0);
	if(ret < 0){
		printf("read error\n");
			close(sckcli);
		return NULL;
	}else{
		buff[ret]='\0';
		printf("received:%s\n", buff);			
	}


	ret = Writen(sckcli, buff, ret, 3, 0);
	if(ret < 0){
		printf("write error\n");
	}
	close(sckcli);
	
}



void* accpet_work(void* arg)
{
	int sckcli;
	int sckser = *(int *)arg;
	pthread_t       tid; 
    pthread_attr_t  attr; 
	fd_set fd;
	int ret;
	
	while(1)
	{
		
		ret = select_fd_timeout_r(sckser, 3, 0);
		if(ret <= 0){
			printf("1AcceptRemote ret  socket Accept错误errno = [%d] %d\n", errno, ret);
			continue;
		}  
		sckcli = AcceptRemote(sckser);
		if(INVALID_SOCKET == sckcli)
		{
			printf("2AcceptRemote ret  socket Accept错误errno = [%d]\n", errno);
			usleep(1);
			continue;
		}
		pthread_attr_init(&attr); 
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
		pthread_create(&tid, &attr, work, &sckcli);

	}
	
}

/* 
	pthread_t       tid; 
    pthread_attr_t  attr; 
    pthread_attr_init(&attr); 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
    pthread_create(&tid, &attr, THREAD_FUNCTION, arg); 
	*/


int main(int rgvs, char **rgva)
{
	
	int ret;
	int len;
	int sckser1;
	int sckser2;

	int i=0;
	char buff[1024];

	if(rgvs <3){
		printf("%s <listen port port>\n", rgva[0]);
		return 0;		
	}


	pthread_t       tid1; 
	pthread_t       tid2; 
    pthread_attr_t  attr; 
		
	sckser1 = ListenRemote(atoi(rgva[1]));
	if(sckser1 >0){
		setOpetinNoBlock(sckser1);
		pthread_create(&tid1, NULL, accpet_work, &sckser1);	
	}
	sckser2 = ListenRemote(atoi(rgva[2]));
	if(sckser2 > 0){
		setOpetinNoBlock(sckser2);	
		pthread_create(&tid2, NULL, accpet_work, &sckser2);
	}
	
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	
		
		
	close(sckser1);
	close(sckser2);

	
}