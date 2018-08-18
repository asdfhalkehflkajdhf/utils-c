#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <pthread.h>
#include "../libftp.h"



void * monitor_handle( void *arg)
{
	int ret;
	ret = ftp_client_init("d");
	if(ret !=0){
		printf("1 init error\n");
		return NULL;
	}
	ret = ftp_client_cmd("open 192.168.200.242 liuzc lzc123");
	int i;
	for(i=0; i<2; i++){
		ret = ftp_client_cmd("ls");
		if(ret)
			printf("%s\n", ftp_get_E_errors(ret));
		ret = ftp_client_cmd("help");
		if(ret)
			printf("%s\n", ftp_get_E_errors(ret));
		sleep(2);
	}
	ret = ftp_client_cmd("quit");

	return NULL;
}

void * sche_handle( void *arg)
{
	int ret;
	ret = ftp_client_init("d");
	if(ret !=0){
		return NULL;
	}

	ret = ftp_client_cmd("open 192.168.200.242 liuzc lzc123");
	int i;
	for(i=0; i<1; i++){
		ret = ftp_client_cmd("ls");
		if(ret)
			printf("%s\n", ftp_get_E_errors(ret));

		ret = ftp_client_cmd("mkdir test");
		if(ret)
			printf("%s\n", ftp_get_E_errors(ret));
		sleep(2);
	}
	ret = ftp_client_cmd("quit");

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t monitor_pid;           //monitor thread id
	pthread_t sche_pid;              //monitor thread id
	global_ftp_client_init(GLOBAL_FTP_INIT_DEFAULT);
	//global_ftp_client_init(GLOBAL_FTP_INIT_DEBUG);

	
	pthread_create(&monitor_pid, NULL, monitor_handle, NULL);
	pthread_create(&sche_pid, NULL, sche_handle, NULL);
	
	pthread_join(monitor_pid, NULL);
	pthread_join(sche_pid, NULL);
	
	global_ftp_client_exit();

	return 0;

}