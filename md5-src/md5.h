/*md5.h 
*/ 
#ifndef _MD5_H_ 
#define _MD5_H_ 

/* MD5 context. */ 
typedef struct { 
/* state (ABCD) */ 
/*
四个
32bits
数，用于存放最终计算得到的消息摘要。当消息长度〉
512bits
时，也用于存
放每个
512bits
的中间结果
*/ 
unsigned long state[4]; 
/* number of bits, modulo 2^64 (lsb first) */ 
/*
存储原始信息的
bits
数长度
,
不包括填充的
bits
，最长为
2^64 bits
，因为
2^64
是一个
64
位数的最大值
*/ 
unsigned long count[2]; 
/* input buffer */ 
/*
存放输入的信息的缓冲区，
512bits*/ 
unsigned char buffer[64]; 
} MD5_CTX; 
void MD5Init(MD5_CTX *); 
void MD5Update(MD5_CTX *, unsigned char *, unsigned int); 
void MD5Final(unsigned char [16], MD5_CTX *); 
#endif /* _MD5_H_ */ 
/////////////////////////////////////////////////////////////////////////// 
/*md5.cpp 
*/ 
/* Constants for MD5Transform routine. */ 
/*md5
转换用到的常量，算法本身规定的
*/ 
#define S11 7 
#define S12 12 
#define S13 17 
#define S14 22 
#define S21 5 
#define S22 9 
#define S23 14 
#define S24 20 
#define S31 4 
#define S32 11 
#define S33 16 
#define S34 23 
#define S41 6 
#define S42 10 
#define S43 15 
#define S44 21 
static void MD5Transform(unsigned long [4], unsigned char [64]); 
static void Encode(unsigned char *, unsigned long *, unsigned int); 
static void Decode(unsigned long *, unsigned char *, unsigned int); 
/*用于
bits
填充的缓冲区，为什么要
64
个字节呢？因为当欲加密的信息的
bits
数被
512
除其
余数为
448
时，
需要填充的
bits
的最大值为
512=64*8 
。
*/ 
static unsigned char PADDING[64] = { 
0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
}; 
/*接下来的这几个宏定义是
md5
算法规定的，就是对信息进行
md5
加密都要做的运算。
据说有经验的高手跟踪程序时根据这几个特殊的操作就可以断定是不是用的
md5 
*/ 
/* F, G
, H and I are basic MD5 functions. 
*/ 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z))) 
#define G(x, y, z) (((x) & (z)) | ((y) & (~z))) 
#define H(x, y, z) ((x) ^ (y) ^ (z)) 
#define I(x, y, z) ((y) ^ ((x) | (~z))) 
/* ROTATE_LEFT rotates x left n bits. 
*/ 
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n)))) 
/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4. 
Rotation is separate from addition to prevent recomputation. 
*/ 
#define FF(a, b, c, d, x, s, ac) {\
(a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac);\
(a) = ROTATE_LEFT ((a), (s));\
(a) += (b);\
} 
#define GG(a, b, c, d, x, s, ac) {\
(a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac);\
(a) = ROTATE_LEFT ((a), (s));\
(a) += (b);\
} 
#define HH(a, b, c, d, x, s, ac) {\
(a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac);\
(a) = ROTATE_LEFT ((a), (s));\
(a) += (b);\
} 
#define II(a, b, c, d, x, s, ac) {\
(a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac);\
(a) = ROTATE_LEFT ((a), (s));\
(a) += (b);\
} 
/* MD5 initialization. Begins an MD5 operation, writing a new context. */  
  