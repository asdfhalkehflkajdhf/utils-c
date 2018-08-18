#include<stdio.h>
#include<pthread.h>
#include<string.h>
pthread_key_t p_key;


#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/time.h>
#include <iconv.h>

sigjmp_buf jmp_env[4]={0};
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


char src[2048]="%u6d4b%u8bd5";

char dest[2048]={0};
#define   setbit(x,y) (x|=(1<<y))  //将X的第Y位置1
#define   clrbit(x,y) (x&=~(1<<y)) //将X的第Y位清0
char out[6]={0};
static int unicode2utf8(int key, unsigned char *output)
{
	unsigned int src_t = (unsigned int)key;
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

static int  HttpDecodeAscii(char *Src, int SrcLen, char *Dest)
{
	char *p = NULL;
	int dest_cur_len = 0;
	unsigned char convert_l=0;
	unsigned char convert[3]={0};
	int i;
	if(Src == NULL ||Dest == NULL){
		return 0;
	}

	for(i=0; i<SrcLen; i++){
		p=Src+i;
		if(*p == '+'){
			Dest[dest_cur_len++] = ' ';
		}else if(*p == '%'){
			if(*(p+1)=='u'){
				unsigned int unicode_k=0;
				printf("s ucs2u %x\n",unicode_k);
				memcpy(convert, p+2, 2);
				convert_l = (unsigned char)strtol(convert, NULL, 16);
				unicode_k = convert_l;
				unicode_k = unicode_k<<8;
				printf("%x\n", convert_l);
				memcpy(convert, p+4, 2);
				convert_l = (unsigned char)strtol(convert, NULL, 16);
				unicode_k = unicode_k | convert_l;
				printf("%x\n", convert_l);
				printf("ucs2u %x\n",unicode_k);
				int outlen = unicode2utf8(unicode_k, out);
				printf("%d\n", outlen);
				memcpy(Dest+dest_cur_len, out, outlen);
				dest_cur_len = dest_cur_len + outlen;
				i = i + 5;
			}else{
				memcpy(convert, p+1, 2);
				convert_l = (char)strtol(convert, NULL, 16);
				if((unsigned char)convert_l > 0x7F){
					unsigned int unicode_k=0;
					unicode_k = convert_l;
					unicode_k = unicode_k<<8;
					memcpy(convert, p+3, 2);
					convert_l = (char)strtol(convert, NULL, 16);
					unicode_k = unicode_k | convert_l;
					int outlen = unicode2utf8(unicode_k, out);
					memcpy(Dest+dest_cur_len, out, outlen);
					dest_cur_len = dest_cur_len + outlen;
					i = i + 4;
				}else{
					Dest[dest_cur_len++] = convert_l;
					i = i + 2;
				}
			}
		}else{
			Dest[dest_cur_len++] = *p;
		}
	}
	
	Dest[dest_cur_len] = '\0';
	return dest_cur_len;
}

void func1()
{
        int *tmp = (int*)pthread_getspecific(p_key);//同一线程内的各个函数间共享数据。
        printf("%d is runing in %s %d\n",*tmp,__func__, pthread_self());
 
}
void *thread_func(void *args)
{
		int sleep_t = *(int *)args * 3;
		int jmp_index = *(int *)args;
		
        if (sigsetjmp(jmp_env[jmp_index], 1))
        {
            printf("timeout %d\n", *(int *)args);
            goto out;
        }

        pthread_setspecific(p_key,args);
 
        int *tmp = (int*)pthread_getspecific(p_key);//获得线程的私有空间
        printf("%d is runing in %s %d\n",*tmp,__func__, pthread_self());
 
        *tmp = (*tmp)*100;//修改私有变量的值
        func1();
		
		printf("sleep %d %d\n", sleep_t, pthread_self());
		sleep(sleep_t);
		printf("goto jmp %d\n",pthread_self());
		siglongjmp(jmp_env[jmp_index], 1);

 out:

        return (void*)0;
}

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

int main()
{
int ret;
	ret = HttpDecodeAscii(src, strlen(src), dest);

	printf("len=%d, %s\n",ret, dest);
	printf("\n\nutf82unicode\n");
unsigned int temp=0;
	ret = utf82unicode(dest, 3, &temp);
	printf("unicode src %s\n",src);
	printf("unicode to utf8 ret = %d %x\n",ret, temp);
	return 0;
	pthread_t pa, pb;
        int a=2;
        int b=1;
        pthread_key_create(&p_key,NULL);
        pthread_create(&pa, NULL,thread_func,&a);
        pthread_create(&pb, NULL,thread_func,&b);
        pthread_join(pa, NULL);
        pthread_join(pb, NULL);
        return 0;
}