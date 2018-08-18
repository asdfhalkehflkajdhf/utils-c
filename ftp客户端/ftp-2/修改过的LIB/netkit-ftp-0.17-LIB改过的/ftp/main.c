#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <pthread.h>
#include "libftp.h"

int
main(volatile int argc, char **volatile argv)
{
	(void)argv;
	(void)argc;
	int ret;

	global_ftp_client_init(GLOBAL_FTP_INIT_DEBUG);

	ret = ftp_client_init("d");
	if(ret !=0){
		return ret;
	}



	//setpeer(argc, &argv[0]);
	ret = ftp_client_cmd("open 192.168.200.242 liuzc lzc123");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("ls");
	printf("ftp_test %d\n",ret);

	ret = ftp_client_cmd("mkdir test");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("help");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("quit");
	printf("ftp_test %d\n",ret);
	
	global_ftp_client_exit();
	return 0;
}

