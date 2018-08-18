/************************command !! by Mr.zhou at 2011.08.15**********************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ftp_cmd(char *cmd)
{
	char stt[1024]={0};
	char *cmd_name[] = {"ls\n","get","put","bye\n","ls","bye"};
	int  i = 0 ;
	int  j = 0;
	char *p = NULL;
	char *pp= NULL;
	while(1){
		bzero(cmd,sizeof(cmd));
		fgets(stt,1024,stdin);

		if(*stt =='\n')
		{
			printf("error command,please put again!\n");
			continue;
		}

		p=strtok(stt," ");
		if((strcmp(p,cmd_name[0]))== 0){
			return 1;
		}
		if((strcmp(p,cmd_name[4]))== 0){
			return 1;
		}
		if((strcmp(p,cmd_name[3]))== 0){
			return 4;	
		}
		if((strcmp(p,cmd_name[5]))== 0){
			return 4;	
		}

		pp=strtok(NULL,"\n");
		if(pp!=NULL){
			for(i=0;i<200;i++){
				if(pp[i]!=' '){
					j=i;
					for(;i<200;i++){
						if(pp[i]==' '){
							pp[i]='\0';
							i=200;
						}
					}
					break;
				}
			}
			strcpy(cmd,&pp[j]);

			for(i=1;i<4;i++)
			{
				if((strcmp(p,cmd_name[i])) == 0)
					return i+1;
			}
		}
		printf("error command,please put again!\n");
		continue;
	}
}
/*
   int main(void)
   {
   char cmd[1024] = {0};
   int nn=ftp_cmd(cmd);
   printf("%d\n",nn);
   printf("%s\n",cmd);
   return 0;
   }

 */
