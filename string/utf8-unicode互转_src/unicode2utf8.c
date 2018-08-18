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
	key为需要转码的数据，
	output为输出数据内容，长度最长为6个字节
	
	返回值为输出内容长度
*/
static int unicode2utf8(unsigned int key, unsigned char *output)
{
	unsigned int src_t = key;
	int i;
	int k=-1;
	for(i=0; i<6; i++){
		if( src_t >= g_usc2u_t[i].ucs_min
			&& src_t <= g_usc2u_t[i].ucs_max ){
			k=i;
			break;
		}
	}
	if(k==-1){
		return 0;
	}
	memset(output, 0, k+1);
	for(i=k;i>0;i--){
		output[i]=src_t & g_usc2u_t[k].other;
		src_t = src_t >>6;
		setbit(output[i], 7);
		//clrbit(output[i+1], 6);
	}
	output[0]=(char)src_t;
	output[0]=output[0]| g_usc2u_t[k].head;

	return k+1;
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