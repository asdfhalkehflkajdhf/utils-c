#ifndef __P_H_
#define __P_H_

#include <stddef.h>

/*c hearders*/
#ifdef __cplusplus
extern "C" {
#endif

#define OK 0
#define FAIL -1

#define USER_NAME_LEN 32
#define PASS_WORD_LEN 64
#define SUBDIR_LEN 512
#define PATH_LEN 512
#define FILE_NAME_LEN 128


#define DATA_BUFFER_SIZE 1024*10

typedef struct certification_
{
	char username[USER_NAME_LEN];
	char password[PASS_WORD_LEN];
	int connect_time;
}certification;
typedef struct data_head_
{
	int filesize;
	char subdir[512];
	char filename[128];
}data_head;
typedef struct data_push_
{
	int buff_len;
	char buff[DATA_BUFFER_SIZE];
}data_push;

typedef struct pkt_head_
{
	int type;
	union
	{
		certification certific;
		data_head h;
		data_push p;
	}data;
}pkt_head;


#define PKT_HEAD_SIZE 4
#define DATA_HEAD_SIZE sizeof(data_head)
#define DATA_PUSH_SIZE sizeof(data_push)
#define CERTIFICA_SIZE sizeof(certification)


enum
{
	CERTIFICATION,
	DISCONNECT,
	DATA_HEAD,
	DATA_PUSH,
	MAX
};



#ifdef __cplusplus
}
#endif

#endif /* __P_H_ */
