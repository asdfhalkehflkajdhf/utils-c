#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include <string.h>

#define LF          0x0a
#define CR          0x0d

#define CHECKLEN(tol, len) {\
    if (0 == (len)) {\
        return;\
    }\
    if ((tol) > (len)) {\
        return;\
    }\
}
#define CHECK(start, end) {\
    if (start >= end) {\
        return;\
    }\
}

int findUint8(const char* buf, int len, char c)
{
	int off = 0;
	for(; off<len; off++)
	{
		if (buf[off]==c)
		{
			return off;
		}
	}
	return -1;
}

int findTwoUint8(const char* buf, int len, char c, char nc, int *flag)
{
	int off = 0;
	for(; off<len; off++)
	{
		if (buf[off]==c)
		{
			*flag = 0;
            return off;
		}
        if (buf[off]==nc)
        {
            *flag = -1;
            return off;
        }
	}
	return -1;
}
int lineEndLFCR(const char* buf, int len, int *n_off)
{
	int index = 0;
    for( index = 0; index<len; ++index)
    {
        if (buf[index] == CR)
        {
			*n_off = index + 2;//buf[*n_off-1]==LF
			return (index);//buf[index]==CR
        }
    }
    return -1;
}

int lineEndLF(const char* buf, int len, int *n_off)
{
	int index = 0;
    for( index = 0; index<len; ++index)
    {
        if (buf[index] == CR)
        {
			*n_off = index+1;//buf[*n_off-1] = CR
            return index;//buf[index] == CR
        }
    }
    return -1;
}

int isLineEnd(const char* buf, int len,int *endhead)
{
	int index = 0;
    for( index = 0; index<len; ++index)
    {
        if (buf[index] == CR)
        {
        	index += 1;
			if (buf[index] == LF)
			{
				*endhead = 2;
				return (index + 1);
			}
			else
			{
				*endhead = 1;
				return index;
			}
		}
    }
	return -1;
}

void httpAnalyzeHead(const char* buf, int len, struct http * http_ptr,
					int endhead)
{
	int dissectHost = 0;
	int dissectUa = 0;
	int n_off = 0, dissectLen = 0, end = 0;
	if (strlen(http_ptr->host) !=0 )
	{
		return;
	}
	int (*findLineEnd)(const char* buf, int off, int *n_off) = NULL;
	if (endhead == 2)
	{
		findLineEnd = lineEndLFCR;
	}
	else
	{
		findLineEnd = lineEndLF;
	}
    while(dissectLen < len)
	{
        /*host不在uri里，则此处解析host*/
		if (strncasecmp((char *)&buf[dissectLen], "Host:", 5) == 0)//Host:
		{
			dissectLen += 5;
			/*实际的数据有可能出现多个空格,或者没空格，此处跳过空格*/
			while(buf[dissectLen] == ' ')
			{
				++dissectLen;
			}
			int hostLen = (*findLineEnd)(&buf[dissectLen], len-dissectLen, &n_off);
			if (hostLen<0)//找不到空格说明 消息太长产生了分段，或者被截断
			{
				end = (len-dissectLen)>1024 ? dissectLen+1024 : len;
				CHECK(dissectLen, end);
				bcopy(&buf[dissectLen], http_ptr->host, end-dissectLen);
				return;
			}
			end = dissectLen+(hostLen< 1024? hostLen: 1024);
			CHECK(dissectLen, end);
			bcopy(&buf[dissectLen], http_ptr->host, end-dissectLen);
			dissectLen += n_off;
			dissectHost = -1;
			if (!dissectHost && !dissectUa)
			{
				return;
			}
			continue;
		}
		else if(strncasecmp((char *)&buf[dissectLen], "User-Agent:", 11)==0)//User-Agent:
		{

			dissectLen+=11;
			/*跳过空格*/
			while(buf[dissectLen] == ' ')
			{
				++dissectLen;
			}
			int uaLen = (*findLineEnd)(&buf[dissectLen], len-dissectLen, &n_off);
			if (uaLen < 0)//找不到空格说明 消息太长产生了分段，或者被截断
			{
				end = (len-dissectLen)>1024?1024+dissectLen:len;
				CHECK(dissectLen, end)
				bcopy(&buf[dissectLen], http_ptr->ua, end-dissectLen);
				return;
			}
			end =  dissectLen+(uaLen< 1024? uaLen: 1024);
			CHECK(dissectLen, end);
			bcopy(&buf[dissectLen], http_ptr->ua, end-dissectLen);
			dissectLen += n_off;

			dissectUa = -1;
			if (!dissectHost && !dissectUa)
			{
				return;
			}
			continue;
		}
		if ((*findLineEnd)(&buf[dissectLen], len - dissectLen, &n_off) >=0)
		{
            if (n_off <= endhead)//消息头结束标志
            {
                break;
            }
            dissectLen += n_off;
        }
		else
		{
            return;
		}
	}
	return;
}

void httpAnalyzeReq(const char* buf, int len, struct http *http_ptr){
	int off=0, n_off=0, endhead =0;
	if(http_ptr == NULL){
		return;
	}

	if (buf[off] != '/')//host包含于uri中
	{
		off +=7;//跳过http://
		CHECKLEN(0, len-off);
		int flag = -1;

		/*判断是先找到'/', 还是' ', 找到'/',说明host在uri中,host结束的标志为'/'*/
		if ((n_off = findTwoUint8(&buf[off], len-off, '/', ' ', &flag)) >= 0)
		{
			if (flag == 0)
	        {
				bcopy(&buf[off], http_ptr->host, n_off> 1024 ? 1024 : n_off);
				off=n_off+1+off;
				if ((n_off = findUint8(&buf[off], len-off, ' ')) >= 0)
				{
					bcopy(buf, http_ptr->url, (n_off+off) > 1024 ? 1024 : n_off+off);
				}
				else
				{
					bcopy(&buf[off], http_ptr->url, len>1024 ? 1024 : len);
					return;
				}
				off= n_off+1+off;
	        }
			else
			{
				/*找不到host结束标志*/
				bcopy(buf, http_ptr->url, (n_off+off) > 1024 ? 1024 : n_off+off);
				off= n_off+1+off;
			}
		}
		else
		{
			return;
		}
	}
	else//host不在uri中
	{
		if ((n_off = findUint8(&buf[off], len-off, ' ')) >= 0)
		{
			bcopy(buf, http_ptr->url, n_off>1024 ? 1024 : n_off);
		}
		else
		{
			bcopy(buf, http_ptr->url, len > 1024 ? 1024 : len);
			return;
		}
		off= n_off+1+off;
	}
	CHECK(off, len);
	n_off = isLineEnd(&buf[off], len-off,&endhead);
	if(n_off >= 0)//确定是/r/n结尾还是/r
	{
		off+=n_off;
		CHECK(off, len);
		return httpAnalyzeHead(&buf[off], len-off, http_ptr,endhead);
	}
	return;
}


char buf[1024]="GET / HTTP/1.1\r\nAccept-Encoding: gzip,deflate,sdch\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36\r\nHost: www.csdn.net\r\nAccept-Language: zh-CN,zh;q=0.8\r\nCookie: pva=mx9gqp; uuid_tt_dd=-4853309685081224574_20131204; __message_district_code=110000; __message_sys_msg_id=2195; __utma=17226283.1422475218.1386124799.1386124799.1386124799.1; __utmc=17226283; __utmz=17226283.1386124799.1.1.utmcsr=google|utmccn=(organic)|utmcmd=organic|utmctr=http%20uri%20%E5%88%86%E6%9E%90; pvg=mx9h8r; __message_gu_msg_id=0; __message_cnel_msg_id=0; __message_in_school=0\r\nIf-Modified-Since: Wed, 04 Dec 2013 02:40:03 GMT";
 
struct http http_ptr;

 
int main(int argc, char** argv)
{

	int len = strlen(buf);
	memset(&http_ptr,0,sizeof(struct http));
	
	CHECKLEN(5, len);
	int off = 0;
	if(memcmp(buf,"GET ",4) == 0){
		off += 4;
	}
	else if(memcmp(buf,"POST ",5) == 0){
		off += 5;
	}
	else{
		return;
	}
	while(buf[off] == ' ' && off<=len)
	{
		++off;
	}
	CHECKLEN(0, len-off);
	//printf("httpAnalyzeReq \r\n");
	httpAnalyzeReq(&buf[off], len-off, &http_ptr);
	
	printf("url: %s\n",http_ptr.url);
	printf("host: %s\n",http_ptr.host);
	
}
