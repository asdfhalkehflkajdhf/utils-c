#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <errno.h>
/*
	多路优化复制
*/
void memcpyM(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
	//找对齐Coefficient of alignment
	//int C_of_a = sizeof(void *);
	int C_of_a = sizeof(long int);
	//块复制
	int block = count/C_of_a;
	//剩余
	int surplus = count%C_of_a;
	int double_b = block/8;
	int double_s = block%8;
	switch (surplus ) {
		case 7:     *(to+6)=*(from+6);to++;from++; ;
		case 6:     *(to+5)=*(from+5);to++;from++; ;
		case 5:     *(to+4)=*(from+4);to++;from++; ;
		case 4:     *(to+3)=*(from+3);to++;from++; ;
		case 3:     *(to+2)=*(from+2);to++;from++; ;
		case 2:     *(to+1)=*(from+1);to++;from++; ;
		case 1:     *(to+0)=*(from+0);to++;from++; ;
    } 
	switch (double_s ) {
		case 7:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 6:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 5:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 4:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 3:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 2:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 1:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
    } 
	while(--double_b>0){
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
	}
	
}
/*
	单路优化复制
*/
void memcpyO(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
	//找对齐Coefficient of alignment
	//int C_of_a = sizeof(void *);
	int C_of_a = sizeof(long int);
	//块复制
	int block = count/C_of_a;
	//剩余
	int surplus = count%C_of_a;
	switch (surplus ) {
		case 0:     *(to+7)=*(from+7);to++;from++; ;
		case 7:     *(to+6)=*(from+6);to++;from++; ;
		case 6:     *(to+5)=*(from+5);to++;from++; ;
		case 5:     *(to+4)=*(from+4);to++;from++; ;
		case 4:     *(to+3)=*(from+3);to++;from++; ;
		case 3:     *(to+2)=*(from+2);to++;from++; ;
		case 2:     *(to+1)=*(from+1);to++;from++; ;
		case 1:     *(to+0)=*(from+0);to++;from++; ;
    } 
	//
	while(--block>0){
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
	}
	
}

/*
	常用复制函数
*/
void memcpy1(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
    do{
        *to++ = *from ++ ;
    }while ( --count != 0 ) ;
}
/*
file:///F:/utils/memcpy%E4%BC%98%E5%8C%96-src/memcpy%E7%9A%84%E4%BC%98%E5%8C%96%20-%20jeason%E7%9A%84%E6%97%A5%E5%BF%97%20-%20%E7%BD%91%E6%98%93%E5%8D%9A%E5%AE%A2.htm
源码
*/
void memcpy4(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
#define DEF(x) int*to##x=(int*)to+x;int*from##x=(int*)from+x 
#define EXEC(x) *to##x=*from##x;to##x+=8;from##x+=8;
    DEF(0) ;DEF(1) ;DEF(2) ;DEF(3) ;DEF(4) ;DEF(5) ;DEF(6) ;DEF(7) ;
    int rem = count % 32 ;
    int n = (count)/32;
    switch( (count / 4)%8 ) {
    case 0: do{ EXEC(7) ;
    case 7:     EXEC(6) ;
    case 6:     EXEC(5) ;
    case 5:     EXEC(4) ;
    case 4:     EXEC(3) ;
    case 3:     EXEC(2) ;
    case 2:     EXEC(1) ;
    case 1:     EXEC(0) ;
            }while(--n>0);
    }    
    to = (char*)to0 ;
    from = (char*)from0 ;
    while ( rem -- != 0 ) {
        *to ++ = *from ++ ;
    }
#undef DEF
#undef EXEC
}
int main(int argc, char **argv)
{
	if(argc != 2){
		printf("input test buff len\n");
		return 0;
	}
	struct timeval tval;
//(uint64_t)(tval.tv_sec)*1000000ull + (uint64_t)tval.tv_usec
	int len = atoi(argv[1]);
	char *buffto=(char *)malloc( len);
	char *bufffrom=(char *)malloc( len);
	long int usec_y;
	long int usec_4;
	long int usec_O;
	long int usec_M;
	int sec =0;
	gettimeofday(&tval, NULL);
	sec = tval.tv_sec+60;
	while(tval.tv_sec<sec){
		//gettimeofday(&tval, NULL);
		usec_y = tval.tv_usec;
		//memcpy4(buffto, bufffrom, 1024);
		memcpy(buffto, bufffrom, len);
		gettimeofday(&tval, NULL);
		usec_y = tval.tv_usec - usec_y;
		
		//gettimeofday(&tval, NULL);
		usec_4 = tval.tv_usec;
		//memcpy4(buffto, bufffrom, 1024);
		memcpy4(buffto, bufffrom, len);
		gettimeofday(&tval, NULL);
		usec_4 = tval.tv_usec - usec_4;
		
		//gettimeofday(&tval, NULL);
		usec_O = tval.tv_usec;
		//memcpy4(buffto, bufffrom, 1024);
		memcpyO(buffto, bufffrom, len);
		gettimeofday(&tval, NULL);
		usec_O = tval.tv_usec - usec_O;
		
		//gettimeofday(&tval, NULL);
		usec_M = tval.tv_usec;
		//memcpy4(buffto, bufffrom, 1024);
		memcpyM(buffto, bufffrom, len);
		gettimeofday(&tval, NULL);
		usec_M = tval.tv_usec - usec_M;
	}
	printf("memcpy  %dus %d\n", usec_y, len);
	printf("memcpy4 %dus %d\n", usec_4, len);
	printf("memcpyO %dus %d\n", usec_O, len);
	printf("memcpyM %dus %d\n", usec_M, len);
	
	free(buffto);
	free(bufffrom);
	return 0;
}