#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <sys/stat.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include <fcntl.h>
#include <arpa/inet.h>
#include "p.h"

int port=12345;
char username[USER_NAME_LEN]={0};
char passwd[PASS_WORD_LEN]={0};
char subdir[512]={0};
int connect_timeout=30;
char connect_addr[20]={0};



int connect_init(char *addr, int port)
{
	struct sockaddr_in client_addr;
	bzero(&client_addr,sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
	client_addr.sin_port = htons(0);	//0表示让系统自动分配一个空闲端口

	int client_socket = socket(AF_INET,SOCK_STREAM,0);
	if( client_socket < 0)
	{
		printf("Create Socket Failed!\n");
		return -1;
	}
	if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n");
		return -1;
	}
	
	//设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(inet_aton(addr,&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
	{
		printf("Server IP Address Error!\n");
		return -1;
	}
	server_addr.sin_port = htons(port);
	socklen_t server_addr_length = sizeof(server_addr);
	//向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
	if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n",addr);
		return -1;
	}
	return client_socket;
	 
}
void connect_exit(int cfd)
{
	close(cfd);
}
int connect_the_certification(int cfd, char *username, char *password)
{
	pkt_head pkthead;
	int datalen = PKT_HEAD_SIZE+CERTIFICA_SIZE;
	int ret;
	int recv_b[2]={-1,-1};
	
	pkthead.type = CERTIFICATION;
	//pkthead->data = (void *)certific;
	
	memcpy(pkthead.data.certific.username, username, USER_NAME_LEN);
	memcpy(pkthead.data.certific.password, password, PASS_WORD_LEN);
	pkthead.data.certific.connect_time = connect_timeout;

	//printf("send u:%s p:%s\n", pkthead.data.certific.username, pkthead.data.certific.password);
	send(cfd,&pkthead,datalen,0);
    fd_set fd_sets;
	struct timeval tv;
	tv.tv_sec=connect_timeout;
	tv.tv_usec=0;
	FD_ZERO(&fd_sets);
	FD_SET(cfd,&fd_sets);
	ret = select(cfd+1, &fd_sets, NULL, NULL, &tv);
	if (ret > 0 && FD_ISSET(cfd, &fd_sets)){			
		recv(cfd,recv_b,sizeof(recv_b),0);
		if(recv_b[0] != CERTIFICATION || recv_b[1]!=OK){
			printf("pkt t%d d%d\n", recv_b[0], recv_b[1]);
			return -1;
		}
		printf("certifica ok\n");
		return 0;
	}
	printf("connect timeout\n");
	return -1;
}
int connect_the_disconnect(int cfd)
{
	int type = DISCONNECT;
	int recv_b[2]={-1,-1};
	int ret;

	send(cfd,&type,sizeof(int),0);
    fd_set fd_sets;
	struct timeval tv;
	tv.tv_sec=connect_timeout;
	tv.tv_usec=0;
	FD_ZERO(&fd_sets);
	FD_SET(cfd,&fd_sets);
	ret = select(cfd+1, &fd_sets, NULL, NULL, &tv);
	printf("ret %d %d\n", ret, FD_ISSET(cfd, &fd_sets));
	if (ret > 0 && FD_ISSET(cfd, &fd_sets)){			
		recv(cfd,recv_b,sizeof(recv_b),0);
		printf("pkt t%d d%d\n", recv_b[0], recv_b[1]);
		if(recv_b[0] != DISCONNECT || recv_b[1]!=OK){
			return -1;
		}
		return 0;
	}
	return -1;
}

static int send_data_head(int cfd, char *subdir, char *filename, int filesize)
{
	pkt_head pkthead;
	int datalen = PKT_HEAD_SIZE+DATA_HEAD_SIZE;
	int ret;
	int recv_b[2]={-1,-1};

	pkthead.type = DATA_HEAD;
	//pkthead->data = (void *)datahead;
	
	memcpy(pkthead.data.h.subdir, subdir, SUBDIR_LEN);
	memcpy(pkthead.data.h.filename, filename, FILE_NAME_LEN);
	pkthead.data.h.filesize = filesize;
	
	send(cfd,&pkthead,datalen,0);
    fd_set fd_sets;
	struct timeval tv;
	tv.tv_sec=connect_timeout;
	tv.tv_usec=0;
	FD_ZERO(&fd_sets);
	FD_SET(cfd,&fd_sets);
	ret = select(cfd+1, &fd_sets, NULL, NULL, &tv);
	if (ret > 0 && FD_ISSET(cfd, &fd_sets)){			
		recv(cfd,recv_b,sizeof(recv_b),0);
		if(recv_b[0] != DATA_HEAD || recv_b[1]!=OK){
			printf("dh recv  %d(1) %d(0) \n",recv_b[0],  recv_b[1]);
			return -1;
		}
		printf("sdh ok\n");
		return 0;
	}
	printf("sdh timeout\n");
	return -1;
}

static int send_data_push(int cfd, char *filepath)
{
	pkt_head *pkthead=NULL;
	int ret;
	int sdatahead = PKT_HEAD_SIZE+DATA_PUSH_SIZE;
	int sdatalen = sdatahead;
	int recv_b[2]={-1,-1};
	
	pkthead = (pkt_head *)malloc(sizeof(pkt_head));
	if(pkthead == NULL){
		return -1;
	}
	//pkthead->data = (void *)datapush;
	
	FILE *send_fd = fopen(filepath, "r");
	if(send_fd ==NULL){
		printf("open %s fail\n", filepath);
		if(pkthead)free(pkthead);
		return -1;
	}

	while(1){
		memset(pkthead, 0, sizeof(pkt_head));
		pkthead->type = DATA_PUSH;
		pkthead->data.p.buff_len = fread(pkthead->data.p.buff,1, DATA_BUFFER_SIZE,send_fd);
		//printf("%d read num %d\n",pkthead->type,pkthead->data.p.buff_len);
		fseek(send_fd, SEEK_CUR, pkthead->data.p.buff_len);
		ret = ferror(send_fd);
		if(ret){
			printf("file is error (%d)\n",ret);
			if(pkthead)free(pkthead);
			close(send_fd);
			return -1;
		}
		//sdatalen = sdatahead+pkthead->data.p.buff_len;
		send(cfd,pkthead,sdatalen,0);
		
		fd_set fd_sets;
		struct timeval tv;
		tv.tv_sec=connect_timeout;
		tv.tv_usec=0;
		FD_ZERO(&fd_sets);
		FD_SET(cfd,&fd_sets);
		ret = select(cfd+1, &fd_sets, NULL, NULL, &tv);
		if (ret > 0 && FD_ISSET(cfd, &fd_sets)){
			recv(cfd,recv_b,sizeof(recv_b),0);
			if(recv_b[0] != DATA_PUSH || recv_b[1] != OK){
				printf("recv  %d(2) %d(0) \n",recv_b[0],  recv_b[1]);
				if(pkthead)free(pkthead);
				close(send_fd);
				return -1;
			}
		}else{
			printf("sdp timeout\n");
			goto out;
		}
		
		if(feof(send_fd)){
			printf("file is eof\n");
			break;
		}
	}
out:
	if(pkthead)free(pkthead);
	close(send_fd);
	return 0;
}
static int send_file(int cfd, char *subdir, char *filepath)
{
	
	char *ptr = rindex(filepath, '/');
	if(send_data_head(cfd, subdir, ptr+1, get_file_size(filepath))){
		printf("send data head fail\n");
		return -1;
	}
	if(send_data_push(cfd, filepath)){
		printf("send data push fail\n");
		return -1;
	}
	return 0;
}
int main(int argc, char **argv)
{
	int cfd;
	int ret;
	strcpy(connect_addr, "127.0.0.1");
	strcpy(username, "admin");
	strcpy(passwd, "123456");
	cfd = connect_init(connect_addr, port);
	if(cfd < 0){
		printf("connect init fail\n");
		return -1;
	}
	ret = connect_the_certification(cfd, username, passwd);
	if(ret < 0){
		printf("connect certification fail\n");
		return -1;
	}
	int i;
	for(i=1; i<argc; i++){
		ret = send_file(cfd, subdir, argv[i]);
		if(ret < 0){
			printf("send_file %s fail\n", argv[i]);
			return -1;
		}
	}
	connect_exit(cfd);

	return 0;
}
