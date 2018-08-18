#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>   //ip
#include <netinet/tcp.h> //tcp
#include <linux/icmp.h>
#include <stdlib.h>
#include <string.h>


struct llb_hc_icmp_packet{
	struct icmphdr head;
	char * data[32];
};


u_short icmp_cksum (u_char * addr, int len)
{
  register int sum = 0;
  u_short answer = 0;
  u_short *wp;

  for (wp = (u_short *) addr; len > 1; wp++, len -= 2)
    sum += *wp;

  /* Take in an odd byte if present */
  if (len == 1)
    {
      *(u_char *) & answer = *(u_char *) wp;
      sum += answer;
    }

  sum = (sum >> 16) + (sum & 0xffff);	/* add high 16 to low 16 */
  sum += (sum >> 16);		/* add carry */
  answer = ~sum;		/* truncate to 16 bits */
  return answer;
}


int main(int argc, char ** argv)
{
	int ret, sock, n;
	struct sockaddr_in addr;
	struct llb_hc_icmp_packet icmp_packet;
	
	if(argc!=2)
	{
		printf("not dest addr!\n");
	}
	
	sock = socket(AF_INET, SOCK_RAW,IPPROTO_ICMP);   //IP包方式 IPPROTO_RAW
	if (sock>0) {printf("socket ok\n");}
	else {printf ("socket error \n");}
	
	memset(&icmp_packet, 0, sizeof(icmp_packet));
	icmp_packet.head.type  = ICMP_ECHO;
	icmp_packet.head.un.echo.id = getpid();
	memcpy(icmp_packet.data, "abcdefghigklmnopqrstuvwabcdefghi", 32);
	memset(&addr,0,sizeof(addr));
	
	addr.sin_family = AF_INET;
	inet_aton(argv[1],&addr.sin_addr);   //字符串转入地址

	icmp_packet.head.checksum = 0;
	icmp_packet.head.checksum = icmp_cksum((char *)(&icmp_packet), sizeof(icmp_packet));
	ret = sendto(sock, (char *)(&icmp_packet), sizeof(icmp_packet), 0, (struct sockaddr *) &addr, sizeof (struct sockaddr_in));
	if(ret < 0)
	{
		printf("[icmp sendto] err:%d\n", ret);
		return -1;  
	}

	while(1)
	{
		sleep(2);
	}
	
	return 0;
}