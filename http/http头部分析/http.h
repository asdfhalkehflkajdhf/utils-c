#ifndef HTTP_H_
#define HTTP_H_


struct http
{
	char                       url[1025];
	char                       host[1025];
	char 					   ua[1025];
};

//void dissect_http(const uint8_t* buf, int len, struct protocal_analyze_info * p_net_info);

#endif
