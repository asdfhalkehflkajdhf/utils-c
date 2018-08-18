#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <fcntl.h>
#include <errno.h>



#include <linux/tcp.h>
#include <time.h>


#include "tcp.h"

/*
FD_ZERO(fd_set *fdset) 将指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，如果不清空，由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的。
    FD_SET(fd_set *fdset) 用于在文件描述符集合中增加一个新的文件描述符。
    FD_CLR(fd_set *fdset) 用于在文件描述符集合中删除一个文件描述符。
    FD_ISSET(int fd,fd_set *fdset) 用于测试指定的文件描述符是否在该集合中
*/


int select_fd_init(fd_set *fdset)
{
	FD_ZERO(fdset);
	return 0;
}

int select_fd_del(fd_set *fdset, int fd)
{
	FD_CLR(fd, fdset);
	return 0;
}

int select_fd_add(fd_set *fdset, int fd)
{
	FD_SET(fd, fdset);
	return 0;
}
//在集合中返回<0出错，>0可读写，＝＝0超时
int select_fd_check_r(fd_set *fdset_r, int fd, int sec, int usec)
{
	/*
	返回值：返回状态发生变化的描述符总数。 
负值：select错误

正值：某些文件可读写或出错

0：等待超时，没有可读写或错误的文件
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, fdset_r, NULL, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//在集合中返回<0出错，>0可读写，＝＝0超时
int select_fd_check_w(fd_set *fdset_w, int fd, int sec, int usec)
{
	/*
	返回值：返回状态发生变化的描述符总数。 
负值：select错误

正值：某些文件可读写或出错

0：等待超时，没有可读写或错误的文件
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, NULL, fdset_w, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//在集合中返回<0出错，>0可读写，＝＝0超时
int select_fd_check_rw(fd_set *fdset_r, fd_set *fdset_w, int fd, int sec, int usec)
{
	/*
	返回值：返回状态发生变化的描述符总数。 
负值：select错误

正值：某些文件可读写或出错

0：等待超时，没有可读写或错误的文件
	*/
	struct timeval ptv;
	int ret;
	
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;
	ret = select(fd + 1, fdset_r, fdset_w, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
//当描述符fd在描述符集fdset中返回非零值，否则，返回零。
int select_fd_check2(fd_set *fdset, int fd)
{
	return FD_ISSET(fd, fdset);
}
	/*
	返回值：返回状态发生变化的描述符总数。 
负值：select错误

正值：某些文件可读写或出错

0：等待超时，没有可读写或错误的文件
	*/
int select_fd_timeout_r(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, &fdset, NULL, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
int select_fd_timeout_w(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, NULL, &fdset, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
int select_fd_timeout_rw(int fd, int sec, int usec)
{
    struct timeval ptv;
    fd_set fdset;
	int ret;
	
    ptv.tv_sec = sec;
    ptv.tv_usec = usec;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
	ret = select(fd + 1, &fdset, &fdset, NULL, &ptv);
	if(ret < 0){
		return -1;
	}else if(ret > 0){
		return 1;
	}
	return 0;
}
/*
在阻塞模式下, send函数的过程是将应用程序请求发送的数据拷贝到发送缓存中发送就返回.但由于发送缓存的存在,
表现为:如果发送缓存大小比请求发送的大小要大,那么send函数立即返回,同时向网络中发送数据;
否则,send会等待接收端对之前发送数据的确认,以便腾出缓存空间容纳新的待发送数据,
再返回(接收端协议栈只要将数据收到接收缓存中,就会确认,并不一定要等待应用程序调用recv),如果一直没有空间能容纳待发送的数据,则一直阻塞;
在非阻塞模式下,send函数的过程仅仅是将数据拷贝到协议栈的缓存区而已,如果缓存区可用空间不够,则尽能力的拷贝,立即返回成功拷贝的大小;如缓存区可用空间为0,则返回-1,同时设置errno为EAGAIN.


阻塞就是干不完不准回来，非阻塞就是你先干，我现看看有其他事没有，完了告诉我一声。
*/


int CloseSocket(int sockfd)
{
    shutdown(sockfd, 2);

    close(sockfd);
    sockfd = INVALID_SOCKET;
    
    return (0);
}



int Writen(int sckid, unsigned char *buf, int len, int sec, int usec)
{
	struct timeval ptv = {0,0}; 
	ptv.tv_sec = sec;
    ptv.tv_usec = usec;
	//设置发送超时
	setsockopt(sckid, SOL_SOCKET,SO_SNDTIMEO, (char *)&ptv,sizeof(struct timeval));

	if (send(sckid, buf, len, 0) != len)
	{
		return (-1);
	}
    return (len);
}

int Readn(int sckid, char *buf, int len, int sec, int usec)
{
    int rc = 0;
	struct timeval ptv = {0,0}; 
	ptv.tv_sec = sec;
    ptv.tv_usec = usec;
	
	//设置接收超时
	setsockopt(sckid, SOL_SOCKET,SO_RCVTIMEO, (char *)&ptv,sizeof(struct timeval));
	if ((rc = recv(sckid, buf, len, 0)) <= 0)
	{
		return (-1);
	}
    return (rc);
}


int ConnectRemote(char *ip, int port, int sec, int usec)
{
    struct sockaddr_in psckadd;
    struct linger Linger;

	int sckcli;
    int on = 1;
    
	struct timeval ptv;
    
    memset((char *)(&psckadd), '0', sizeof(struct sockaddr_in));
    psckadd.sin_family = AF_INET;
    psckadd.sin_addr.s_addr = inet_addr(ip);
    psckadd.sin_port = htons((u_short) port);
	
    if ((sckcli = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        return (INVALID_SOCKET);
    }
    


	//设置连接超时时间
	ptv.tv_sec = sec;
	ptv.tv_usec = usec;	
	if (setsockopt(sckcli, SOL_SOCKET, SO_SNDTIMEO, &ptv, sizeof(ptv)))
	{
		close(sckcli);
		return (INVALID_SOCKET);
	}	



    if (connect(sckcli, (struct sockaddr *)(&psckadd), sizeof(struct sockaddr_in)) < 0)
    {
        close(sckcli);
        return (INVALID_SOCKET);
    }

	//设置接收tcp外带数据
    if (setsockopt(sckcli, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)))
    {
        CloseSocket(sckcli);
        return (INVALID_SOCKET);
    } 
	
    on = 1;
    setsockopt(sckcli, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
	
	/*
	TCP_NODELAY和TCP_CORK都是禁用Nagle算法，只不过NODELAY完全关闭算法，立即发送而TCP_CORK完全由自己决定发送时机
	*/
    setsockopt(sckcli, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    return (sckcli);
}


int ListenRemote(int port)
{
	int yes=1;
	int server_socket = INVALID_SOCKET;
    struct sockaddr_in server_addr;

    //创建套接字
    if((server_socket=socket(AF_INET,SOCK_STREAM,0))<0){
        return INVALID_SOCKET;
    }
    
    //设置为可重复使用
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    //设置服务器地址信息设置
    server_addr.sin_family=AF_INET;                    //TCP
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=INADDR_ANY;            //本地IP地址
    
    memset(server_addr.sin_zero, 0,sizeof(server_addr.sin_zero));
    
    //绑定套接字与地址信息
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    
    //侦听
    if(listen(server_socket,5)==-1){
        close(server_socket);
        return INVALID_SOCKET;
    }
    
    return server_socket;
}

int AcceptRemote(int sockfd)
{
	int client_sockfd = INVALID_SOCKET;//客户端套接字  
	struct sockaddr_in remote_addr; //客户端网络地址结构体  
	
	int sin_size=sizeof(struct sockaddr_in);  
      
    /*等待客户端连接请求到达*/  
    if((client_sockfd=accept(sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)  
    {  
        return INVALID_SOCKET;  
    } 
        
    //默认为阻塞模式
    
    return client_sockfd;
}


void setOpetinNoBlock(int fd)
{
	//设置为非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK); 

}

/*

int socket_recv(int sockfd, char* buffer, int buflen)
{
	int rs=1;
	int ret;
	while(rs)
	{
		ret = recv(activeevents[i].data.fd, buffer, buflen, 0);
		if(ret < 0)
		{
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
			// 在这里就当作是该次事件已处理处.
			if(errno == EAGAIN)
				break;

		}
		else if(ret == 0)
		{
			// 这里表示对端的socket已正常关闭.
			break;
		}
		if(buflen == ret)
			rs = 1;   // 需要再次读取
		else
			rs = 0;
	}
	
	return ret;

}



int socket_send(int sockfd, const char* buffer, int buflen)
{
	int tmp;
	int total = buflen;
	const char *p = buffer;

	while(1)
	{
		tmp = send(sockfd, p, total, 0);
		if(tmp < 0)
		{
			// 当send收到信号时,可以继续写,但这里返回-1.
			if(errno == EINTR)
			return -1;

			// 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
			// 在这里做延时后再重试.
			if(errno == EAGAIN)
			{
				usleep(1000);
				continue;
			}

			return -1;
		}

		if((size_t)tmp == total)
			return buflen;

		total -= tmp;
		p += tmp;
	}

	return tmp;
}


*/

