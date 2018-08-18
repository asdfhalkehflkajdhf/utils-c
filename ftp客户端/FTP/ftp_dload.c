/**********************break download by Mr.zhou at 2011.08.15*******************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "main.h"

int ftp_downloaw(char *p,int sockfd,int sockfdd)
{
	DIR *dir=NULL;
	char str[1024]={0};
	char buf[1024]={0};
	int fd;
	int n=0;
	if((dir=opendir("."))==NULL){
		perror("for opendir");
		exit(-1);
	}
	struct dirent *rd;
	while((rd=readdir(dir))!=NULL){
		if(strcmp(rd->d_name,p)==0)
		break;
		
	}
	if(rd!=NULL)
	{
		sprintf(str,"TYPE I\r\n");/*指定于二进制的形式传送*/
		Write(sockfd, str, strlen(str));
		n = Read(sockfd, buf, 1024);

		if((fd=open(p,O_RDWR|O_CREAT,0644))<0){
			perror("for open 1");
			exit(-1);
		}

		long long fff=lseek(fd,0,SEEK_END);/*找位  重设了文件指针*/
		sprintf(str,"REST %lld\r\n",fff);/*指定传过来的位置*/
		Write(sockfd, str, strlen(str));
		n = Read(sockfd, buf, 1024);
	}
	else{
		 if((fd = open(p,O_WRONLY|O_CREAT,0644))<0){
                                perror("for open 2");
                                return -1;
                }   

	}

		sprintf(str,"RETR %s\r\n",p);/*下载指令*/
		Write(sockfd, str, strlen(str));
		n = Read(sockfd, buf, 1024);

		while((n = Read(sockfdd, buf, 1024))>0)/*写入到文件*/
			Write(fd,buf,n);
		Close(fd);
		Close(sockfdd);
		return 0;
}
              
