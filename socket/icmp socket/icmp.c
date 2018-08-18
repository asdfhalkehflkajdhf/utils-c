/*
2000.11.13
自定义发IP包例子(TCP/IP包发送)
给目标主机的端口发送一个 syn请求,注意目标主机的信息会发给发送IP地址的主机
这说明TCP/IP协议本身有IP期骗的漏洞
这种方运可以自己写成特殊的基于IP协议上层的自定义协议
ddxxkk@21cn.com
ddxxkk.myrice.com/ddxxkk.hongnet.com
*/
//   目标IP               端口                 发送IP
char srcip[]="100.100.100.100",dstip[]="10.9.123.65";
//  目标IP端口  发送端IP端口
int srcport=25,dstport=99;
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>   //ip
#include <netinet/tcp.h> //tcp
#include <stdlib.h>
#include <string.h>
//以下是从ip.h和tcp.h取的,但BSD和LINUX用的名称有些不一样主要是TCP不一样
/*
struct ip
  {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;                //little-endian IP头长度(单位为32位)4位
    unsigned int ip_v:4;                 //版本号4 IP4用4 
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_v:4;                // version 
    unsigned int ip_hl:4;               // header length 
#endif
    u_int8_t ip_tos;                    //服务类型  一般为0
    u_short ip_len;                     //数据总长度 (单位为32位)
    u_short ip_id;                      //标识16 
    u_short ip_off;                     //分段偏移 
#define IP_RF 0x8000                    // reserved fragment flag  //标志
#define IP_DF 0x4000                    // dont fragment flag 
#define IP_MF 0x2000                    // more fragments flag 
#define IP_OFFMASK 0x1fff               // mask for fragmenting bits 
    u_int8_t ip_ttl;                     //生存时间 
    u_int8_t ip_p;                      //传输协议 tcp是6 
    u_short ip_sum;                     //头校验和
    struct in_addr ip_src, ip_dst;      // 源地址 目标地址 
  };
struct tcphdr
  {
    u_int16_t source;               // 源端口 
    u_int16_t dest;                // 目的端口 
    tcp_seq seq;                  // 序号 
    tcp_seq ack_seq;             // 确认号 收到的TCP信息的序号+1 
#if __BYTE_ORDER == __LITTLE_ENDIAN
    u_int16_t res1:4;               //保留
    u_int16_t doff:4;               //保留
    u_int16_t fin:1;               //标志 结束
    u_int16_t syn:1;               //标志 同步
    u_int16_t rst:1;               //标志 重置
    u_int16_t psh:1;               //标志 入栈
    u_int16_t ack:1;               //标志 确认
    u_int16_t urg:1;               //标志 紧急 
    u_int16_t res2:2;
#elif __BYTE_ORDER == __BIG_ENDIAN
    u_int16_t doff:4;
    u_int16_t res1:4;
    u_int16_t res2:2;
    u_int16_t urg:1;
    u_int16_t ack:1;
    u_int16_t psh:1;
    u_int16_t rst:1;
    u_int16_t syn:1;
    u_int16_t fin:1;
#else
#error  "Adjust your <bits/endian.h> defines"
#endif
    u_int16_t window;           //窗口
    u_int16_t check;            //校验和
    u_int16_t urg_ptr;          //紧急指针
};
*/
//校验和涵数 ip头
unsigned short csum (unsigned short *packet, int packlen)
{
	register unsigned long sum = 0;
	while (packlen > 1)
	{
		sum+= *(packet++);
		packlen-=2;
	}
	if (packlen > 0)
		sum += *(unsigned char *)packet;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (unsigned short) ~sum;
}
//校验和涵数 tcp头
unsigned short tcpcsum (unsigned char *iphdr, unsigned short *packet,int packlen)
{
	unsigned short *buf;
	unsigned short res;
	buf = malloc(packlen+12);
	if(buf == NULL) return 0;
	memcpy(buf,iphdr+12,8); //源IP地址和目标IP地址
	*(buf+4)=htons((unsigned short)(*(iphdr+9)));
	*(buf+5)=htons((unsigned short)packlen);
	memcpy(buf+6,packet,packlen);
	res = csum(buf,packlen+12);
	free(buf);
	return res;
}
int main()
{
	int sock, bytes_send, fromlen,n,id,s;
	unsigned char buffer[65535];
	struct sockaddr_in toaddr;
	struct ip  *ip;
	struct tcphdr *tcp;
	for (n=0;n<60000;n++ )
	{
		buffer[n]=0;
	}
//发送地址
	toaddr.sin_family =AF_INET;
//        inet_aton("192.168.11.38",&toaddr.sin_addr);   //字符串转入地址
	inet_aton(dstip,&toaddr.sin_addr);   //字符串转入地址
  //建立原始TCP包方式IP+TCP信息包
	sock = socket(AF_INET, SOCK_RAW,IPPROTO_RAW);   //IP方式
//     sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sock>0) {printf("socket ok\n");}
	else {printf ("socket error \n");}
	n=1;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &n, sizeof(n)) < 0)
	{ 
		printf("2");
		perror("IP_HDRINCL"); 
		exit(1); 
	}
	ip=(struct ip *)buffer;
	ip->ip_id=0x9911;                
	ip->ip_v=4;
	ip->ip_hl=5;                            
	ip->ip_ttl=36;                          
	ip->ip_p=6;     //tcp 为6
	ip->ip_len=htons(60);
	inet_aton(dstip,&ip->ip_dst);
	//        inet_aton("192.168.11.38",&ip->ip_dst);
	tcp = (struct tcphdr *)(buffer + (4*ip->ip_hl));
	tcp->source=htons(srcport);                            //源端口
	tcp->dest=htons(dstport);   
	tcp->seq=htons(0x9990);
	tcp->doff=0x05;
	tcp->ack_seq=0;
	tcp->syn=1;
	tcp->window=htons(0x20);
	inet_aton(srcip,&ip->ip_src);
 
//        ip->ip_src.s_addr=htonl(0x09010101+n);
	tcp->check=0x0;        
	tcp->check=tcpcsum((unsigned char *)buffer, (unsigned short *)tcp,40);
	ip->ip_sum=0;
	ip->ip_sum=csum((unsigned short *)buffer,20);
	bytes_send=sendto(sock,buffer,60,0,(struct sockaddr *)&toaddr,sizeof(toaddr));    
	if (bytes_send>0)
	{
		printf("OK bytes_send %d \n",bytes_send);
		printf("IP_source address ::: %s \n",inet_ntoa(ip->ip_src));
		printf("IP_dest address ::: %s \n",inet_ntoa(ip->ip_dst));
	}
  
	return 0;
 }