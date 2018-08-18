/**************************break put by Mr. chen at 2011.08.15***********************/

#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include "main.h"
#include <dirent.h>

int ftp_intr_upload(int cmdfd,int datfd,char *filename)
{
	char buf[100] = "0";
	char buf1[1024] = "0";
	int fp,i,j=0,stat=0;
	unsigned long n,len;
	DIR * dir = NULL;

	if((dir=opendir("."))==NULL){
		perror("for opendir");
		exit(-1);
	}
	struct dirent *rd;
	while((rd=readdir(dir))!=NULL){
		if(strcmp(rd->d_name,filename)!=0)
			break;

	}
	bzero(buf,sizeof(buf));
	sprintf(buf,"SIZE %s\r\n",filename);
	printf("%s",buf);
	//puts(buf);
	Write(cmdfd,buf,strlen(buf));
	bzero(buf,sizeof(buf));
	n = Read(cmdfd,buf,sizeof(buf));   /*  */
	printf("%s",buf);
	for(i=0;buf[i]!='\n';i++)
	{
		if(buf[i] == ' ')  {
			stat = 1;
			continue;
		}
		if(stat == 1 && buf[i] != '\n')
		{
			buf1[j] = buf[i] ;
			j++;
		}
	}
	n = atoi(buf1);    /*n is the size of remout file*/

	bzero(buf,sizeof(buf));
	sprintf(buf,"APPE %s\r\n",filename);     
	printf("%s",buf);
	Write(cmdfd,buf,strlen(buf));
	bzero(buf,sizeof(buf));
	Read(cmdfd,buf,sizeof(buf));
	printf("%s",buf);

	fp = open(filename,O_RDONLY);
	len = lseek(fp,0,SEEK_END);
	if(n < len)
	{
		lseek(fp,n,SEEK_SET);
		while(1)
		{
			bzero(buf1,sizeof(buf1));
			n = Read(fp,buf1,sizeof(buf1));
			if(n < 1)   break ;
			Write(datfd,buf1,strlen(buf1));
		}
	//	bzero(buf1,sizeof(buf1));
	//	Read(datfd,buf1,sizeof(buf1));
	//	puts(buf1);
	}
	else  puts("nothing to do for the file:!!");
	Close(datfd);
	return 1;	
}

