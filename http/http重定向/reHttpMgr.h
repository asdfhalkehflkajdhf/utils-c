#ifndef RE_HTTP_MGR_H_
#define RE_HTTP_MGR_H_

#define ERROR_OK 0
#define ERROR_FAIL -1

#define LOCATION_URL_DATA_LEN 1024
#define HTTP_RESPONSE_PKT_LEN 1024

#define PROTO_DNS 53
#define PROTO_SNMP 161


enum  module_switch
{
    MODULE_SWITCH_OFF,          
    MODULE_SWITCH_ON,
    MODULE_SWITCH_MAX,
};

typedef struct reHttp
{
	unsigned int destIp;	//重定向指定目的地址
	unsigned short destPort;	//重定向指定的端口号
	char Location[LOCATION_URL_DATA_LEN];	//重定向指定的URL
}reHttp_t;

#define HTTP_RESPONSE_HEAD	"HTTP/1.1 302 Moved Temporarily"
#define HTTP_SERVER			"Server: Apache-Coyote/1.1"
#define HTTP_LOCATION		"Location: %s"
#define HTTP_CONTENT_TYPE	"Content-Type: text/html;charset=UTF-8"
#define HTTP_CONTENT_LENTGH	"Content-Length: 0"
#define HTTP_DATE			"Date: %s"


extern unsigned int g_portal_module_switch;


extern int reHttp_init(void);
extern int http_redirector(netlib_pkt_desc_s *pdesc, netlib_pkt_desc_s *reHttp_pdesc, netlib_task_info_s *task_info, netlib_net_iface_s *iface);


#endif

