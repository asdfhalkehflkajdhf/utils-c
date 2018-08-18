#include<stdio.h>
#include <string.h>

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define RL(x, y) (((x) << (y)) | ((x) >> (32 - (y))))  //x向左循环移y位

#define PP(x) (x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)|(x>>24)  //将x高低位互换,例如PP(aabbccdd)=ddccbbaa

#define FF(a, b, c, d, x, s, ac) a = b + (RL((a + F(b,c,d) + x + ac),s))
#define GG(a, b, c, d, x, s, ac) a = b + (RL((a + G(b,c,d) + x + ac),s))
#define HH(a, b, c, d, x, s, ac) a = b + (RL((a + H(b,c,d) + x + ac),s))
#define II(a, b, c, d, x, s, ac) a = b + (RL((a + I(b,c,d) + x + ac),s))

char *filename;   //文件名
FILE *fp;

typedef struct { 
unsigned state[4]; 
unsigned len[2]; 
unsigned char buffer[64]; 
} MD5_CTX; 


void MD5Init (MD5_CTX *context, int len) {   
	//context->len[0] = context->len[1] = 0; 
	context->len[1]=len/0x20000000;     //flen单位是bit
    context->len[0]=(len%0x20000000)*8;

	/* Load magic initialization constants.*/ 
    memset(context->buffer,0,64);   //初始化x数组为0

	context->state[0] = 0x67452301; 
	context->state[1] = 0xefcdab89; 
	context->state[2] = 0x98badcfe; 
	context->state[3] = 0x10325476; 
} 

void MD5(unsigned state[4], unsigned char *buffer){                 //MD5核心算法,供64轮

	unsigned a = state[0], b = state[1], c = state[2], d = state[3], *x=(unsigned *)buffer;
  /**//* Round 1 */
  FF (a, b, c, d, x[ 0],  7, 0xd76aa478); /**//* 1 */
  FF (d, a, b, c, x[ 1], 12, 0xe8c7b756); /**//* 2 */
  FF (c, d, a, b, x[ 2], 17, 0x242070db); /**//* 3 */
  FF (b, c, d, a, x[ 3], 22, 0xc1bdceee); /**//* 4 */
  FF (a, b, c, d, x[ 4],  7, 0xf57c0faf); /**//* 5 */
  FF (d, a, b, c, x[ 5], 12, 0x4787c62a); /**//* 6 */
  FF (c, d, a, b, x[ 6], 17, 0xa8304613); /**//* 7 */
  FF (b, c, d, a, x[ 7], 22, 0xfd469501); /**//* 8 */
  FF (a, b, c, d, x[ 8],  7, 0x698098d8); /**//* 9 */
  FF (d, a, b, c, x[ 9], 12, 0x8b44f7af); /**//* 10 */
  FF (c, d, a, b, x[10], 17, 0xffff5bb1); /**//* 11 */
  FF (b, c, d, a, x[11], 22, 0x895cd7be); /**//* 12 */
  FF (a, b, c, d, x[12],  7, 0x6b901122); /**//* 13 */
  FF (d, a, b, c, x[13], 12, 0xfd987193); /**//* 14 */
  FF (c, d, a, b, x[14], 17, 0xa679438e); /**//* 15 */
  FF (b, c, d, a, x[15], 22, 0x49b40821); /**//* 16 */

 /**//* Round 2 */
  GG (a, b, c, d, x[ 1],  5, 0xf61e2562); /**//* 17 */
  GG (d, a, b, c, x[ 6],  9, 0xc040b340); /**//* 18 */
  GG (c, d, a, b, x[11], 14, 0x265e5a51); /**//* 19 */
  GG (b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /**//* 20 */
  GG (a, b, c, d, x[ 5],  5, 0xd62f105d); /**//* 21 */
  GG (d, a, b, c, x[10],  9, 0x02441453); /**//* 22 */
  GG (c, d, a, b, x[15], 14, 0xd8a1e681); /**//* 23 */
  GG (b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /**//* 24 */
  GG (a, b, c, d, x[ 9],  5, 0x21e1cde6); /**//* 25 */
  GG (d, a, b, c, x[14],  9, 0xc33707d6); /**//* 26 */
  GG (c, d, a, b, x[ 3], 14, 0xf4d50d87); /**//* 27 */
  GG (b, c, d, a, x[ 8], 20, 0x455a14ed); /**//* 28 */
  GG (a, b, c, d, x[13],  5, 0xa9e3e905); /**//* 29 */
  GG (d, a, b, c, x[ 2],  9, 0xfcefa3f8); /**//* 30 */
  GG (c, d, a, b, x[ 7], 14, 0x676f02d9); /**//* 31 */
  GG (b, c, d, a, x[12], 20, 0x8d2a4c8a); /**//* 32 */

  /**//* Round 3 */
  HH (a, b, c, d, x[ 5],  4, 0xfffa3942); /**//* 33 */
  HH (d, a, b, c, x[ 8], 11, 0x8771f681); /**//* 34 */
  HH (c, d, a, b, x[11], 16, 0x6d9d6122); /**//* 35 */
  HH (b, c, d, a, x[14], 23, 0xfde5380c); /**//* 36 */
  HH (a, b, c, d, x[ 1],  4, 0xa4beea44); /**//* 37 */
  HH (d, a, b, c, x[ 4], 11, 0x4bdecfa9); /**//* 38 */
  HH (c, d, a, b, x[ 7], 16, 0xf6bb4b60); /**//* 39 */
  HH (b, c, d, a, x[10], 23, 0xbebfbc70); /**//* 40 */
  HH (a, b, c, d, x[13],  4, 0x289b7ec6); /**//* 41 */
  HH (d, a, b, c, x[ 0], 11, 0xeaa127fa); /**//* 42 */
  HH (c, d, a, b, x[ 3], 16, 0xd4ef3085); /**//* 43 */
  HH (b, c, d, a, x[ 6], 23, 0x04881d05); /**//* 44 */
  HH (a, b, c, d, x[ 9],  4, 0xd9d4d039); /**//* 45 */
  HH (d, a, b, c, x[12], 11, 0xe6db99e5); /**//* 46 */
  HH (c, d, a, b, x[15], 16, 0x1fa27cf8); /**//* 47 */
  HH (b, c, d, a, x[ 2], 23, 0xc4ac5665); /**//* 48 */

  /**//* Round 4 */
  II (a, b, c, d, x[ 0],  6, 0xf4292244); /**//* 49 */
  II (d, a, b, c, x[ 7], 10, 0x432aff97); /**//* 50 */
  II (c, d, a, b, x[14], 15, 0xab9423a7); /**//* 51 */
  II (b, c, d, a, x[ 5], 21, 0xfc93a039); /**//* 52 */
  II (a, b, c, d, x[12],  6, 0x655b59c3); /**//* 53 */
  II (d, a, b, c, x[ 3], 10, 0x8f0ccc92); /**//* 54 */
  II (c, d, a, b, x[10], 15, 0xffeff47d); /**//* 55 */
  II (b, c, d, a, x[ 1], 21, 0x85845dd1); /**//* 56 */
  II (a, b, c, d, x[ 8],  6, 0x6fa87e4f); /**//* 57 */
  II (d, a, b, c, x[15], 10, 0xfe2ce6e0); /**//* 58 */
  II (c, d, a, b, x[ 6], 15, 0xa3014314); /**//* 59 */
  II (b, c, d, a, x[13], 21, 0x4e0811a1); /**//* 60 */
  II (a, b, c, d, x[ 4],  6, 0xf7537e82); /**//* 61 */
  II (d, a, b, c, x[11], 10, 0xbd3af235); /**//* 62 */
  II (c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /**//* 63 */
  II (b, c, d, a, x[ 9], 21, 0xeb86d391); /**//* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

}


int MD5BuildFile(char *dest, char *filename, int flen)
{
	int i;
	int len = flen;
	MD5_CTX md5_ctx;

    if (!(fp=fopen(filename,"rb"))) {printf("Can not open this file!\n");return -1;}  //以二进制打开文件

	if(len ==0){
		if (!(fp=fopen(filename,"rb"))) {printf("Can not open this file!\n");return 0;}  //以二进制打开文件
		fseek(fp, 0, SEEK_END);  //文件指针转到文件末尾
		if((len=ftell(fp))==-1) {printf("Sorry! Can not calculate files which larger than 2 GB!\n");fclose(fp);return 0;}  //ftell函数返回long,最大为2GB,超出返回-1
		rewind(fp);  //文件指针复位到文件头
	}
	
	MD5Init(&md5_ctx, len);

    fread(md5_ctx.buffer,1,64,fp);  //以4字节为一组,读取16组数据
    for(i=0;i<len/64;i++){    //循环运算直至文件结束
		MD5(md5_ctx.state, md5_ctx.buffer);
		memset(md5_ctx.buffer,0,64);
		fread(md5_ctx.buffer,1,64,fp);  //以4字节为一组,读取16组数据
    }
    md5_ctx.buffer[len%64]=128;  //文件结束补1,补0操作,128二进制即10000000
    if(len%64>55){
		MD5(md5_ctx.state, md5_ctx.buffer);
		memset(md5_ctx.buffer,0,64);
	} 
    memcpy(md5_ctx.buffer,(void *)md5_ctx.len,8);    //文件末尾加入原文件的bit长度
	MD5(md5_ctx.state, md5_ctx.buffer);
    fclose(fp);
	
	
    snprintf(dest, 32, "%08x%08x%08x%08x",
		PP(md5_ctx.state[0]),PP(md5_ctx.state[1]),
		PP(md5_ctx.state[2]),PP(md5_ctx.state[3]));  //高低位逆反输出
	return 0;
}

int MD5BuildBuff(char *dest,char *buffer, int blen)
{
    char *p=buffer;    //用get函数,避免scanf以空格分割数据,
	int len = blen;
	int i;
	MD5_CTX md5_ctx;

	if(len ==0){
		len = strlen(buffer);
	}
	MD5Init(&md5_ctx, len);

    memcpy(md5_ctx.buffer,p,64);  //以4字节为一组,读取16组数据
	p+=64;
    for(i=0;i<len/64;i++){    //循环运算直至文件结束
		MD5(md5_ctx.state, md5_ctx.buffer);
		memset(md5_ctx.buffer,0,64);
		memcpy(md5_ctx.buffer,p,64);  //以4字节为一组,读取16组数据
		p+=64;
    }
    md5_ctx.buffer[len%64]=128;  //文件结束补1,补0操作,128二进制即10000000
    if(len%64>55){
		MD5(md5_ctx.state, md5_ctx.buffer);
		memset(md5_ctx.buffer,0,64);
	} 
    memcpy(md5_ctx.buffer,(void *)md5_ctx.len,8);    //文件末尾加入原文件的bit长度
	MD5(md5_ctx.state, md5_ctx.buffer);
	
	
    snprintf(dest, 32, "%08x%08x%08x%08x",
		PP(md5_ctx.state[0]),PP(md5_ctx.state[1]),
		PP(md5_ctx.state[2]),PP(md5_ctx.state[3]));  //高低位逆反输出
	return 0;
}


main(int argc, char **argv){
	if(argc !=3){
		printf("usage: error\n");
		return 0;
	}
	char destbuff[33]={0};
	int len;
	if(strcmp(argv[1], "-f")==0){
		MD5BuildFile(destbuff, argv[2], 0);
	}else if(strcmp(argv[1], "-b")==0){
		MD5BuildBuff(destbuff, argv[2], 0);
	}
	
	printf("%s\n", destbuff);
	return 0;
}