#include<stdio.h>
#include<pthread.h>
#include<string.h>
pthread_key_t p_key;


#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/time.h>

/*
U-00000000 - U-0000007F:  0xxxxxxx
U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
typedef struct usc2u_table_
{
	unsigned int ucs_min;
	unsigned int ucs_max;
	unsigned char head;
	unsigned char other;
}usc2u_table;
usc2u_table g_usc2u_t[6]={
	{0x00000000, 0x0000007F,0x00, 0x07F},
	{0x00000080, 0x000007FF,0xC0, 0x03F},
	{0x00000800, 0x0000FFFF,0xE0, 0x03F},
	{0x00010000, 0x001FFFFF,0xF0, 0x03F},
	{0x00200000, 0x03FFFFFF,0xF8, 0x03F},
	{0x04000000, 0x7FFFFFFF,0xFC, 0x03F}
};


#define   setbit(x,y) (x|=(1<<y))  //将X的第Y位置1
#define   clrbit(x,y) (x&=~(1<<y)) //将X的第Y位清0

char out[6]={0};
/*
	key为需要转码的数据，长度最长为6个字节
	output为输出数据内容，
	
	return 0is ok;
		   -1 is fail;
*/
static int utf82unicode(unsigned char *key, int key_len, unsigned int *output)
{
	unsigned char src_t = 0;
	int i;
	
	if(key==NULL || output == NULL || key_len >6 || key_len<1){
		return -1;
	}
	if(key_len == 1){
		*output = *(unsigned int *)key;
		return 0;
	}else{
		*output = (unsigned int )key[0]&((1<<(8-key_len))-1);
		printf("%x\n",*output);
	}
	
	for(i=1; i<key_len; i++){
		*output = *output << 6;
		src_t =key[i];
		clrbit(src_t, 7);
		*output = *output | (unsigned int )src_t;
		printf("%x\n",*output);
	}
	return 0;
}
#if 0
unsigned int src1=0x6d4b;
unsigned int src2=0x8bd5;

unsigned char dest[12]={0};


int main()
{
	int ret=0;
	ret = unicode2utf8(src1, dest);
	ret += unicode2utf8(src1, &dest[ret]);

	
	printf("len=%d, %s\n",ret, dest);
	return 0;

}
#endif