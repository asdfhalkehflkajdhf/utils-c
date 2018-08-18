#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
 
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include<errno.h>

#include<sys/time.h>
#include<pthread.h>

#include "p.h"
#include "utils.h"

typedef struct accept_
{
	int new_client_fd;
	struct sockaddr_in addr;
    fd_set *allset;
}accept_t;
accept_t *accept_new=NULL;
int listen_count=20;

certification *g_certifi=NULL;
int user_count=1;

char g_path[PATH_LEN]="./tmp";

int config_init()
{
	int i;
	accept_new = (accept_t *)malloc(sizeof(accept_t) * listen_count);
	if(accept_new == NULL){
		printf("Server Listen client array Failed!");
		return -1;
	}
    for(i=0;i<listen_count;i++){
		accept_new[i].new_client_fd= -1;
	}

	g_certifi = (certification *)malloc(sizeof(certification) * user_count);
	if(g_certifi == NULL){
		printf("Server Listen client array Failed!");
		return -1;
	}
	for(i=0;i<user_count;i++){
		strcpy(g_certifi[i].username,"admin");
		strcpy(g_certifi[i].password, "123456");
	}
}
void config_exit()
{
	if(accept_new){
		free(accept_new);
	}
	if(g_certifi){
		free(g_certifi);
	}

}
int listen_init(char *addr, int port, int listen_num)
{
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	if(inet_aton(addr,&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
	{
		printf("Server IP Address Error!\n");
		return -1;
	}
	//创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
	int server_socket = socket(AF_INET,SOCK_STREAM,0);
	if( server_socket < 0)
	{
		printf("Create Socket Failed!");
		return -1;
	}
	 
	if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!", port);
		return -1;
	}
	 
	if ( listen(server_socket, listen_num) )
	{
		printf("Server Listen Failed!");
		return -1;
	}
	return server_socket;
}

void listen_exit(int sfd)
{
	close(sfd);
}
int new_client_fd_get_id(int fd, int listen_num)
{
	int i;
	for(i=0;i<listen_num;i++)
	{
		if (accept_new[i].new_client_fd<0)
		{
			accept_new[i].new_client_fd=fd;	 //找一个最小的插进入
			return i;
		}
	}
	return -1;
}
int new_client_fd_set(int index, fd_set *set)
{
	return FD_SET(accept_new[index].new_client_fd,set);   //新加入的描述符，还没判断是否可以或者写，所以后面使用rset而不是allset
}
int new_client_fd_isset(int index, fd_set *set)
{
	return FD_ISSET(accept_new[index].new_client_fd,set);
}

int connect_certification(certification *cur, int *timeout)
{
	int i;
	for(i=0; i<user_count; i++){
		printf("recv u:%s p:%s\n", cur->username, cur->password);
		printf("cur  u:%s p:%s\n", g_certifi[i].username, g_certifi[i].password);
		if(strcmp(g_certifi[i].username, cur->username)==0
			&& strcmp(g_certifi[i].password, cur->password)==0){
			*timeout = cur->connect_time;
			return OK;
		}
	}
	return FAIL;
}

//服务器处理
void send_ok_state(int fd, int type)
{
	int send_b[2];
	send_b[0]=type;
	send_b[1]=OK;
	send(fd,send_b,sizeof(send_b),0);

}
//服务器处理
void send_fail_state(int fd, int type)
{
	int send_b[2];
	send_b[0]=type;
	send_b[1]=FAIL;
	send(fd,send_b,sizeof(send_b),0);

}
void send_state(int fd, int type, int state)
{
	if(state)
		send_fail_state(fd, type);
	else
		send_ok_state(fd, type);
}

int debug_data(char *data, int len)
{
    if (len <= 0) {
        return 0;
    }

    printf("----------len=%u(0x%x)\n", len, len);
    int i;
    for (i=0; i<len; i++) {
        printf("0x%02x,", data[i]);
        if ((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n");

    for (i=0; i<len; i++) {
        if (data[i] >= 0x20 && data[i] <= 0x7e)
            printf("%c", data[i]);
        else
            printf(" ");

        if ((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n");
    fflush(stdout);

    return 0;
}

void str_echo(accept_t *accept_cur)            // 服务器收到客户端的消息后的响应
{
	pkt_head *pkthead = (pkt_head *)malloc(sizeof(pkt_head));
	data_head *datahead=NULL;
	data_push *datapush=NULL;
	int filesize=0;
	int recvsize=0;
	char full_path[PATH_LEN]={0};

    ssize_t n;
	if(pkthead == NULL){
		return;
	}

	int timeout=1;
	int ret=OK;
	FILE *fd=NULL;
    fd_set fd_sets;
	struct timeval tv;
	printf("new connect %d\n", accept_cur->new_client_fd);

    while(1)
    {
		tv.tv_sec=timeout;
		tv.tv_usec=0;
		FD_ZERO(&fd_sets);
		FD_SET(accept_cur->new_client_fd,&fd_sets);
		ret = select(accept_cur->new_client_fd+1, &fd_sets, NULL, NULL, &tv);
		if (ret <= 0 || !FD_ISSET(accept_cur->new_client_fd, &fd_sets)){			
			goto out;
		}
		
		n=recv(accept_cur->new_client_fd,pkthead,sizeof(pkt_head),0);
		if (n <= 0 || errno == EINTR)
		{
			goto out;
		}

    	switch(pkthead->type)
    	{
			case CERTIFICATION:
				ret = connect_certification((certification *)&pkthead->data.certific, &timeout);
				send_state(accept_cur->new_client_fd,pkthead->type,ret);
				break;
			case DISCONNECT:
				goto out;
			case DATA_HEAD:
				datahead=(data_head *)&pkthead->data.h;
				snprintf(full_path, PATH_LEN, "%s/%s", g_path, datahead->subdir);
				if (recursive_make_dir(full_path, DIR_MODE) != 0){
					printf("mkdir error [%s]\n", full_path);
					send_fail_state(accept_cur->new_client_fd,pkthead->type);
					break;
				}
				
				snprintf(full_path, PATH_LEN, "%s/%s/%s", g_path, datahead->subdir, datahead->filename);
				if(file_exists_state(full_path)==0)delete_file(full_path);
				if (fd != NULL) {fclose(fd);fd=NULL;}
				fd = fopen(full_path, "a");
				if (fd == NULL){
					printf("open error [%s]\n", full_path);
					send_fail_state(accept_cur->new_client_fd,pkthead->type);
					break;
				}
				filesize=datahead->filesize;
				recvsize = 0;
				if(filesize == 0){
					if (fd != NULL) {fclose(fd);fd=NULL;}
				}
				send_ok_state(accept_cur->new_client_fd,pkthead->type);
				break;
			case DATA_PUSH:
				datapush = (data_push *)&pkthead->data.p;
				recvsize+=datapush->buff_len;
				if(fd != NULL){
					fwrite(datapush->buff, datapush->buff_len, 1, fd);
				}else{
					printf("data push fail\n");
					delete_file(full_path);
					break;
				}
				if(filesize>0 && recvsize == filesize){
					printf("write end [%s]\n", full_path);
					if (fd != NULL) {fclose(fd);fd=NULL;}
				}
				send_state(accept_cur->new_client_fd,pkthead->type,OK);
				break;
			default:
				break;
		}

    }
out:
	printf("disconnect %d\n", accept_cur->new_client_fd);
	if (fd != NULL){
		fclose(fd);
	}
	if(pkthead){
		free(pkthead);
	}
}


//使用线程代替进程来处理客户请求
void* threadPerClient(void *arg)
{
    accept_t *accept_cur = ((accept_t *)arg);
    pthread_detach(pthread_self());

    str_echo(accept_cur);


	FD_CLR(accept_cur->new_client_fd,accept_cur->allset);
    close(accept_cur->new_client_fd);
	accept_cur->new_client_fd = -1;
    return NULL;
}

void listen_start(int sfd, int listen_num)
{
    //initial "select" elements
    int maxfd=sfd;            //新增listenfd，所以更新当前的最大fd
	int i;
    //for select
    int nready;
    ssize_t n;
    fd_set allset;
    fd_set reset;
	struct sockaddr_in chiaddr;
	int chilen=sizeof(struct sockaddr_in);

	int new_fd;

	
    FD_ZERO(&allset);
    FD_SET(sfd,&allset);    
    //end initial
	struct timeval tv;

    int cliconn=0;
	int new_fd_i;
    while(1)
    {
		FD_ZERO(&reset);
		FD_SET(sfd, &reset);  // add fd
		reset = allset;
		// timeout setting
		tv.tv_sec = 1;
		tv.tv_usec = 0;

        nready=select(maxfd+1,&reset,NULL,NULL,&tv);
        if(nready>0 && FD_ISSET(sfd,&reset)){
            new_fd=accept(sfd,(struct sockaddr*)&chiaddr,&chilen);
            //阻塞在accept，直到三次握手成功了才返回
            if(new_fd<0)
            	printf("accept client error: %s\n",strerror(errno));

			new_fd_i = new_client_fd_get_id(new_fd, listen_num);

			if(new_fd_i<0){
				printf("client connected is max ,close and sleep(%d)\n",1);
				close(new_fd);
				sleep(1);
				continue;
			}
			
			FD_SET(new_fd,&allset);

            if(new_fd>maxfd) maxfd=new_fd;

			//for thread
			pthread_t pid;
			
			memcpy(&accept_new[new_fd_i].addr, &chiaddr, chilen);
			accept_new[new_fd_i].allset = &allset;
			int errerr=pthread_create(&pid,NULL,threadPerClient,&accept_new[new_fd_i]);
			if(errerr!=0)
				printf("err %s",strerror(errno));

		}
    }

}
int main(int argc, char **argv)
{
	int fd;
	config_init();
	fd = listen_init("0.0.0.0", 12345, listen_count);
	if(fd<0){
		printf("listen_init Failed!\n");
		return 0;
	}

	listen_start(fd, listen_count);
	
	listen_exit(fd);
	config_exit();
	return 0;
}
