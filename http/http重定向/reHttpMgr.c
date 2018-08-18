#include "netlib.h"
#include "tools.h"
#include "common.h"

#include <time.h>
#include "policyDefine.h"

#include "http.h"
#include "reHttpMgr.h"

#undef PRINTF
#define PRINTF(fmt,arg...)  netlib_debug(NL_DEBUG_RE_HTTP, fmt,##arg)
#define WRITE_DATA_BUFFER(data_buff, Location, date_gmt) {\
			snprintf((data_buff),HTTP_RESPONSE_PKT_LEN,\
				"%s\r\n%s\r\nLocation: %s\r\n%s\r\n%s\r\nDate: %s\r\n\r\n",\
				HTTP_RESPONSE_HEAD,\
				HTTP_SERVER,\
				(Location),\
				HTTP_CONTENT_TYPE,HTTP_CONTENT_LENTGH,\
				(date_gmt));\
			}


#define FIN_SYN_ACK		30

//重定向信息数据结构
reHttp_t g_reHttp_config;
//直接使用哈希key为下标。使用链结构在新加删除时还需要加锁。

unsigned int g_portal_module_switch = MODULE_SWITCH_ON;

//ip头校验和函数
unsigned short csum (unsigned short *packet, int packlen)
{
	register unsigned long sum = 0;
	while (packlen > 1)
	{
		sum+= *(packet++);
		packlen-=2;
	}
	if (packlen > 0)
	{
		sum += *(unsigned char *)packet;
	}
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (unsigned short) ~sum;
}
//tcp头校验和函数
unsigned short tcpcsum (unsigned char *iphdr, unsigned short *packet,int packlen)
{
	unsigned short *buf;
	unsigned short res;
	buf = malloc(packlen+12);
	if(buf == NULL) return 0;
	{
		memcpy(buf,iphdr+12,8); //源目地地址
	}
	*(buf+4)=htons((unsigned short)(*(iphdr+9)));
	*(buf+5)=htons((unsigned short)packlen);
	memcpy(buf+6,packet,packlen);
	res = csum(buf,packlen+12);
	free(buf);
	return res;
}

int ip_tcp_csum(netlib_pkt_desc_s *pdesc, struct iphdr *ipv4_hdr, struct tcphdr *tcp_hdr, int data_len)
{

	ipv4_hdr->tot_len = htons( ipv4_hdr->ihl * 4 + tcp_hdr->doff * 4 + data_len);
	ipv4_hdr->check = csum((unsigned short *)ipv4_hdr,ipv4_hdr->ihl * 4);
	tcp_hdr->check = tcpcsum((unsigned char *)ipv4_hdr, (unsigned short *)tcp_hdr, pdesc->tcp_hdr_ptr->doff * 4 +data_len);
	return ERROR_OK;
}

int is_http_get_pkt(netlib_pkt_desc_s *pdesc)
{
	int ipv4_hdr_len = pdesc->ipv4_hdr_ptr->ihl *4;
	int tcp_hdr_len = pdesc->tcp_hdr_ptr->doff * 4;
	int pkt_tot_len = ntohs(pdesc->ipv4_hdr_ptr->tot_len);

    if ( pkt_tot_len - ipv4_hdr_len - tcp_hdr_len <= 0)
	{
		return ERROR_FAIL;
    }
    if ( pkt_tot_len - ipv4_hdr_len - tcp_hdr_len < 5)
	{
		return ERROR_FAIL;
    }
    
	if(memcmp(pdesc->l4_hdr_ptr + tcp_hdr_len, "GET ", 4) != 0)
	{
		return ERROR_FAIL;
	}

	return ERROR_OK;

}


#define HTTP_FRAGMENT_HASH_LEN 1024


typedef struct reHttp_fragment_node
{
	unsigned int srcIp;
	unsigned int destIp;
	unsigned short sPort;
	unsigned short dPort;
	int fragment_num;		//收到两个包要回一个ack
	time_t sec;
	struct reHttp_fragment_node *next;
}reHttp_fragment_node_t;

typedef struct reHttp_fragment_hash
{
	tmc_spin_mutex_t lock;
	reHttp_fragment_node_t list;
}reHttp_fragment_hash_t;

reHttp_fragment_hash_t *g_reHttp_fragment_list;

static unsigned int reHttp_get_hash(unsigned int a, unsigned int b, unsigned short c, unsigned short d, unsigned int mask)
{
  a += c;
  b += d;
  c = 0;
  a = a^b;
  b = a;
  return (jhash_3words(a,(a^c),(a|(b<<16)),0x5123598) & mask);
}

int reply_tcp_fragment_create(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *new_node;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	hash_key = reHttp_get_hash(pdesc->ipv4_hdr_ptr->saddr,pdesc->ipv4_hdr_ptr->daddr,
		pdesc->tcp_hdr_ptr->source,pdesc->tcp_hdr_ptr->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;
	tmc_spin_mutex_lock(&hash_head->lock);

	temp_node = &hash_head->list;
	node = hash_head->list.next;

	while(node !=NULL)
	{
		if(node->destIp != pdesc->ipv4_hdr_ptr->daddr || node->srcIp != pdesc->ipv4_hdr_ptr->saddr ||
			node->dPort != pdesc->tcp_hdr_ptr->dest || node->sPort != pdesc->tcp_hdr_ptr->source)
		{
			temp_node = temp_node->next;
			node = node->next;
			continue;
		}
		
		tmc_spin_mutex_unlock(&hash_head->lock);
		return ERROR_OK;
	}
	new_node = malloc(sizeof(reHttp_fragment_node_t));
	if(new_node != NULL && node == NULL)
	{
		new_node->destIp = pdesc->ipv4_hdr_ptr->daddr;
		new_node->srcIp = pdesc->ipv4_hdr_ptr->saddr;
		new_node->dPort = pdesc->tcp_hdr_ptr->dest;
		new_node->sPort = pdesc->tcp_hdr_ptr->source;
		new_node->fragment_num = 1;
		new_node->next = NULL;
		time(&new_node->sec);
		temp_node->next = new_node;
		PRINTF(" hash_key:%d %x to %x; %x to %x\n",hash_key,new_node->srcIp,new_node->destIp,new_node->sPort,new_node->dPort);
	}
	tmc_spin_mutex_unlock(&hash_head->lock);

	return ERROR_OK;
}
int reply_tcp_fragment_search_updata(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	unsigned int fragment_num;
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *node;

	hash_key = reHttp_get_hash(pdesc->ipv4_hdr_ptr->saddr,pdesc->ipv4_hdr_ptr->daddr,
		pdesc->tcp_hdr_ptr->source,pdesc->tcp_hdr_ptr->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;
	tmc_spin_mutex_lock(&hash_head->lock);

	node = hash_head->list.next;

	while(node !=NULL)
	{
		if(node->destIp != pdesc->ipv4_hdr_ptr->daddr || node->srcIp != pdesc->ipv4_hdr_ptr->saddr ||
			node->dPort != pdesc->tcp_hdr_ptr->dest || node->sPort != pdesc->tcp_hdr_ptr->source)
		{
			node = node->next;
			continue;
		}
		node->fragment_num++;
		time(&node->sec);
		fragment_num = node->fragment_num;
		PRINTF(" hash_key:%d [%x] to [%x]; %x to %x fragment_num:%d\n",hash_key,node->srcIp,node->destIp,node->sPort,node->dPort,fragment_num);
		if(fragment_num % 2 == 0)
		{
			tmc_spin_mutex_unlock(&hash_head->lock);
			return ERROR_OK;
		}
	}
	
	PRINTF(" hash_key:%d [%x] to [%x]; %x to %x find fail\n",hash_key,pdesc->ipv4_hdr_ptr->saddr,pdesc->ipv4_hdr_ptr->daddr,
		pdesc->tcp_hdr_ptr->source,pdesc->tcp_hdr_ptr->dest);
	tmc_spin_mutex_unlock(&hash_head->lock);

	return ERROR_FAIL;

}
int reply_tcp_fragment_del(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	time_t tem_sec;
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	hash_key = reHttp_get_hash(pdesc->ipv4_hdr_ptr->saddr,pdesc->ipv4_hdr_ptr->daddr,
		pdesc->tcp_hdr_ptr->source,pdesc->tcp_hdr_ptr->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;
	tmc_spin_mutex_lock(&hash_head->lock);


	temp_node = &hash_head->list;
	node = temp_node->next;
	time(&tem_sec);
	while(node !=NULL)
	{
		if(node->destIp != pdesc->ipv4_hdr_ptr->daddr || node->srcIp != pdesc->ipv4_hdr_ptr->saddr ||
			node->dPort != pdesc->tcp_hdr_ptr->dest || node->sPort != pdesc->tcp_hdr_ptr->source)
		{
		/*
			if(tem_sec - node->sec >180)
			{
				//结点超时，需要删除
				temp_node = node->next;
				free(node);
				node = temp_node->next;
			}
			else
		*/
			{
				temp_node = temp_node->next;
				node = node->next;
			}
			continue;
		}
		PRINTF(" hash_key:%d %x to %x; %x to %x\n",hash_key,node->srcIp,node->destIp,node->sPort,node->dPort);
		temp_node = node->next;
		free(node);
		break;
	}
	
	tmc_spin_mutex_unlock(&hash_head->lock);

	return ERROR_OK;
}

int reply_packet_iphdr_data(netlib_pkt_desc_s *pdesc, struct iphdr *ipv4_hdr)
{
	ipv4_hdr->id=0;
	ipv4_hdr->version=4;
	ipv4_hdr->ihl=pdesc->ipv4_hdr_ptr->ihl;
	ipv4_hdr->ttl=64;
	ipv4_hdr->protocol=6;	  //tcp 6
	ipv4_hdr->frag_off=ntohs(0x4000);
	ipv4_hdr->tos=pdesc->ipv4_hdr_ptr->tos;

	ipv4_hdr->daddr=pdesc->ipv4_hdr_ptr->saddr;
	ipv4_hdr->saddr=pdesc->ipv4_hdr_ptr->daddr;
	ipv4_hdr->check = 0;

	return ERROR_OK;
}

int reply_packet_tcphdr_data(netlib_pkt_desc_s *pdesc, struct tcphdr *tcp_hdr)
{
	int ipv4_hdr_len = pdesc->ipv4_hdr_ptr->ihl *4;
	int tcp_hdr_len = pdesc->tcp_hdr_ptr->doff * 4;
	int data_len = ntohs(pdesc->ipv4_hdr_ptr->tot_len) - tcp_hdr_len - ipv4_hdr_len;
	int ret;
	int ret_Http;
	int send_falg = 0;
	
	tcp_hdr->dest = pdesc->tcp_hdr_ptr->source;
	tcp_hdr->source = pdesc->tcp_hdr_ptr->dest;
	tcp_hdr->doff = pdesc->tcp_hdr_ptr->doff;
	tcp_hdr->window = pdesc->tcp_hdr_ptr->window;
	tcp_hdr->check = 0;
	tcp_hdr->seq = pdesc->tcp_hdr_ptr->ack_seq;
	tcp_hdr->ack = 1;

	/*分析报文头get 未使用*/
	ret_Http = is_http_get_pkt(pdesc);
	//这是一个ACK　包
	if(pdesc->tcp_hdr_ptr->fin == 0 && pdesc->tcp_hdr_ptr->syn == 0 && 
		pdesc->tcp_hdr_ptr->ack == 1 && pdesc->tcp_hdr_ptr->psh == 0)
	{
		//是ack get包
		if(ret_Http == ERROR_OK)
		{//新建
			reply_tcp_fragment_create(pdesc);
		}
		else
		{//更新
			ret = reply_tcp_fragment_search_updata(pdesc);
			if( ret == ERROR_OK)
			{
				send_falg = -1;
				goto fin_syn_ack;
			}
		}
		return ERROR_FAIL;
	}
	//这是第一个数据包
	if(pdesc->tcp_hdr_ptr->fin == 0 && pdesc->tcp_hdr_ptr->syn == 0 &&
		pdesc->tcp_hdr_ptr->ack == 1 && pdesc->tcp_hdr_ptr->psh == 1 )
	{
		if(ret_Http == ERROR_FAIL)
		{
			//删除
			reply_tcp_fragment_del(pdesc);
		}
		tcp_hdr->ack_seq = htonl(ntohl(pdesc->tcp_hdr_ptr->seq)+ data_len);
		tcp_hdr->psh = 1;
		tcp_hdr->fin= 1;
		return ERROR_OK;
	}
	//这是一个RST_ACK　包
	if(pdesc->tcp_hdr_ptr->fin == 0 && pdesc->tcp_hdr_ptr->rst == 1 &&
		pdesc->tcp_hdr_ptr->ack == 1 && pdesc->tcp_hdr_ptr->psh == 0)
	{
	
		send_falg = 0;
		goto fin_syn_ack;
	}

	//这是一个SYN　包
	if(pdesc->tcp_hdr_ptr->fin == 0 && pdesc->tcp_hdr_ptr->syn == 1 && 
		pdesc->tcp_hdr_ptr->ack == 0 && pdesc->tcp_hdr_ptr->psh == 0)
	{
		send_falg = 0;
		tcp_hdr->syn = 1;
		goto fin_syn_ack;
	}
	//这是一个FIN_ACK
	if(pdesc->tcp_hdr_ptr->fin == 1 && pdesc->tcp_hdr_ptr->syn == 0 && 
		pdesc->tcp_hdr_ptr->ack == 1 && pdesc->tcp_hdr_ptr->psh == 0 )
	{
		send_falg = 0;
		goto fin_syn_ack;
	}
	
	return ERROR_FAIL;
	
fin_syn_ack:
	if(send_falg != 0)
	{
		tcp_hdr->ack_seq = htonl(ntohl(pdesc->tcp_hdr_ptr->seq)+ data_len);
	}
	else
	{
		tcp_hdr->ack_seq = htonl(ntohl(pdesc->tcp_hdr_ptr->seq)+ 1);
	}
	return FIN_SYN_ACK;

}

int reply_packet_ethhdr_data(netlib_pkt_desc_s *pdesc, struct ethhdr *eth_hdr)
{
	eth_hdr->h_proto = pdesc->eth_hdr_ptr->h_proto;
	memcpy(eth_hdr->h_dest, pdesc->eth_hdr_ptr->h_source, ETH_ALEN);
	memcpy(eth_hdr->h_source, pdesc->eth_hdr_ptr->h_dest, ETH_ALEN);

	return ERROR_OK;
}

int reply_packet_buffer_data(char * data_hdr)
{
	char date_gmt[35] = {0};
	struct tm *ptr;
	time_t timep;

	time(&timep);
	ptr = localtime(&timep);
	strftime(date_gmt,35,"%a, %d %h %Y %T GMT",ptr);
	WRITE_DATA_BUFFER(data_hdr, g_reHttp_config.Location, date_gmt);

	return strlen(data_hdr);
}


int reply_send_packet(netlib_pkt_desc_s * pdesc, netlib_task_info_s *task_info, netlib_net_iface_s *iface, char *packet, int pkt_len)
{

	//netlib_pkt_desc_s * pdesc = netlib_pdesc_alloc_desc(task_info);	
	
	netlib_pdesc_alloc_buff(task_info, NETLIB_MPIPE_L_PKT_SIZE, pdesc);
	if (NULL == pdesc->pkt_base_ptr)
	{   
		netlib_pdesc_free_pkt(task_info, pdesc);
		return ERROR_FAIL;
	}

	/*拷贝数据到缓冲区*/
	memcpy((void*)pdesc->pkt_base_ptr, (void*)packet, pkt_len);	
	pdesc->pkt_length = pkt_len;

	/*设置数据报文的描述符, 避免后续报文发送不出去*/
	pdesc->idesc.va = (int_reg_t)pdesc->pkt_base_ptr;
	pdesc->idesc.l2_size = pkt_len;    
	netlib_send_packet(task_info,iface,pdesc);
	
	return ERROR_OK;

}

int reply_packet_transparent_forward(netlib_pkt_desc_s *pdesc)
{
	unsigned short *port = NULL;
	unsigned short h_poto = 0;
	
	/*过滤帧类型*/
	if(pdesc->eth_hdr_ptr->h_proto == htons(ETH_P_8021Q))
	{
		h_poto = *((unsigned short *)pdesc->eth_hdr_ptr + 8);
	}
	else
	{
		h_poto = pdesc->eth_hdr_ptr->h_proto;
	}
	
	if ( h_poto != htons(ETH_P_IP) )
	{
		return ERROR_FAIL;
	}

	port = (unsigned short *)pdesc->l4_hdr_ptr;
	if(port == NULL)
	{
		return ERROR_FAIL;
	}

	//包过滤目的端口为snmp,dns端口号则报文透传
	if(port[1] == htons(PROTO_DNS) || port[1] == htons(PROTO_SNMP))
	{
		return ERROR_FAIL;
	}

	if( ntohl(pdesc->ipv4_hdr_ptr->daddr) ==  g_reHttp_config.destIp && 
		ntohs(port[1]) == g_reHttp_config.destPort)
	{
		PRINTF("ip daddr %x is 302 rePKT\n",ntohl(pdesc->ipv4_hdr_ptr->daddr));		
		return ERROR_FAIL;
	}

	return ERROR_OK;

}

void show_packet_info_hexlog(unsigned char* str, int len)
{
	unsigned char* p = str;
	int out_len = len;
	char x[17];
	int nn;
	int j;
	if (len <= 0)
		return;
	printf("----------------hex dump start-----------------\n");
	for (nn = 0; nn < out_len; nn++) {
		printf("%02X ", p[nn]);
		if ((p[nn] & 0xff) < 0x20 || (p[nn] & 0xff) >= 0x7f)
			x[nn % 16] = '.';
		else
			x[nn % 16] = p[nn];
		if (nn % 16 == 15 || nn == out_len - 1) {
			if (nn == out_len - 1 && nn % 16 != 15)
				for (j = 0; j < 16 - (nn % 16); j++)
					printf("   ");
			printf("\t\t %s\n", x);
			memset(x, 0, 17);
		}
	}
	printf("\n----------------hex dump end-------------------\n");
}

int http_redirector(netlib_pkt_desc_s *pdesc, netlib_pkt_desc_s *reHttp_pdesc, netlib_task_info_s *task_info, netlib_net_iface_s *iface)
{
	struct iphdr *ipv4_hdr;
	struct tcphdr *tcp_hdr;
    struct ethhdr *eth_hdr;
	char *data_hdr;

	int eth_hdr_len = 0;
	int ipv4_hdr_len = 0;
	int tcp_hdr_len = 0;
	int data_len = 0;
	int pkt_len = 0;
	int ret;
	char packet[NETLIB_MPIPE_L_PKT_SIZE]={0}; 
	unsigned short *port = NULL;
	
	ret = reply_packet_transparent_forward(pdesc);
	if(ret == ERROR_FAIL)
	{
		goto transparent_forward;
	}
	
	//过滤http请求包，不是直接截断丢包。现在只处理http get请求包	
	port = (unsigned short *)pdesc->l4_hdr_ptr;
	if(port[1] != htons(HTTP_80_PORT) && port[1] != htons(HTTP_443_PORT))
	{
		goto truncation_drop;
	}
	
	eth_hdr_len = pdesc->eth_hdr_ptr->h_proto == htons(ETH_P_8021Q) ? ETH_HLEN + 4: ETH_HLEN;
	ipv4_hdr_len = pdesc->ipv4_hdr_ptr->ihl *4;
	tcp_hdr_len = pdesc->tcp_hdr_ptr->doff * 4;
	
	eth_hdr = (struct ethhdr *)(packet);
	ipv4_hdr = (struct iphdr *)(packet + eth_hdr_len);
	tcp_hdr = (struct tcphdr *)(packet + eth_hdr_len + ipv4_hdr_len);
	data_hdr = packet + eth_hdr_len + ipv4_hdr_len + tcp_hdr_len;


	reply_packet_ethhdr_data(pdesc, eth_hdr);

	reply_packet_iphdr_data(pdesc, ipv4_hdr);

	ret = reply_packet_tcphdr_data(pdesc, tcp_hdr);
	if( ret == ERROR_FAIL)
	{
		goto truncation_drop;
	}
	if( ret == FIN_SYN_ACK)
	{//需要回的ack
		goto fin_syn_ack;
	}

	data_len = reply_packet_buffer_data(data_hdr);

	ip_tcp_csum(pdesc, ipv4_hdr, tcp_hdr, data_len);

	pkt_len = eth_hdr_len + ipv4_hdr_len + tcp_hdr_len + data_len;

	ret = reply_send_packet(reHttp_pdesc, task_info, iface, packet, pkt_len);
	if(ret == ERROR_FAIL)
	{
		goto truncation_drop;
	}
	
	PRINTF(" FLOW_ACTION_MATCH_OK 302 packet\n");
	return FLOW_ACTION_MATCH_OK;	
fin_syn_ack:
	ip_tcp_csum(pdesc, ipv4_hdr, tcp_hdr, 0);
	pkt_len = eth_hdr_len + ipv4_hdr_len + tcp_hdr_len;
	ret = reply_send_packet(reHttp_pdesc, task_info, iface, packet, pkt_len);
	if(ret == ERROR_FAIL)
	{
		goto truncation_drop;
	}
	netlib_free_packet(task_info, pdesc);
	
	PRINTF(" FLOW_ACTION_MATCH_OK fin_syn_ack\n");
	return FLOW_ACTION_MATCH_OK;
truncation_drop:
	
	//PRINTF("truncation_drop\n");
	return FLOW_ACTION_FORBID;
transparent_forward:
	
	//PRINTF("transparent_forward\n");
	return FLOW_ACTION_PERMIT;
}


int reHttp_init(void)
{
	int size;

	//这里只初始化一次,之后是否移到ipipe main
	//主要是新建一个报文发送对列
	//tmc_alloc_map();
	strncpy(g_reHttp_config.Location, "http://10.9.100.210:8099/ipipe_web_2/login/loginForm",
			strlen("http://10.9.100.210:8099/ipipe_web_2/login/loginForm"));

	g_reHttp_config.destIp = 0x0a0964d2;
	g_reHttp_config.destPort = 8099;
	//本模块的全局变量开关
	//g_portal_module_switch = MODULE_SWITCH_ON;

    size = sizeof(reHttp_fragment_hash_t)*HTTP_FRAGMENT_HASH_LEN;
    g_reHttp_fragment_list = (reHttp_fragment_hash_t*)malloc(size);
    if( NULL == g_reHttp_fragment_list )
    {
        //return ERROR_FAIL;
    }
    memset(g_reHttp_fragment_list, 0, size);


	return ERROR_OK;
}

