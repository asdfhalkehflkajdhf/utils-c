#include <zlib.h>
 
#include <stdio.h>
 
#include <sys/types.h>
 
#include <sys/stat.h>
 
#include <fcntl.h>
 
#include <sys/mman.h>
 #include <bzlib.h>
#define WF_FAIL -1
#define WF_OK 0
//gcc   zpip2-big.c -lbz2
char buf[1024*1024*100];
int Compress(const char * DestName,const char *SrcName)
{
    FILE * fp_in = NULL;int len = 0;

    int re = WF_OK;
    
    if( NULL == (fp_in = fopen(SrcName,"rb")))
    {
        return WF_FAIL;
    }

    /////////////////////////////////////////////
    BZFILE * out = BZ2_bzopen(DestName,"wb6f");
    
    if(out == NULL)
    {
        return WF_FAIL;
    }

    for(;;)
    {
        len = fread(buf,1,sizeof(buf),fp_in);
        
        if(ferror(fp_in))
        {
            re = WF_FAIL;
            break;
        }
        
        if(len == 0) break;

        if(BZ2_bzwrite(out, buf, (unsigned)len) != len)
        {
            re = WF_FAIL;
        }
    }

    BZ2_bzclose(out);

    fclose(fp_in);

    return re;
 }

 int UnCompress(const char * DestName,const char *SrcName)
{
    FILE * fp_out = NULL;
	int re = WF_OK;
    
    BZFILE *in;int len = 0;char buf[16384];

    in = BZ2_bzopen(SrcName,"rb");

    if(in == NULL)
    {
        return WF_FAIL;
    }

    if(NULL == (fp_out = fopen(DestName,"wb")))
    {
        BZ2_bzclose(in);
        return WF_FAIL;
    }
    
    for (;;)
    {
        len = BZ2_bzread(in,buf,sizeof(buf));

        if(len < 0)
        {
            re = WF_FAIL;
            break;
        }

        if(len == 0) break;

        if(fwrite(buf,1,(unsigned)len,fp_out)!=len)
        {
            re = WF_FAIL;
            break;
        }
    }

    fclose(fp_out);
    BZ2_bzclose(in);

    return re;
}
 //[gz/zip/tar/tar.Z/tar.gz]
int main(int argc, char **argv)
 
{
 
	if(argc !=3)
	{
		printf("usage: <src> <dest>\n");
		printf("support：[tar/gz/bz2/zip/tar.Z/taz/tar.xz/txz/tar.lzma]\n");
		return 0;
	}
	if(Compress(argv[2], argv[1])){
		printf("zip error %s\n", argv[1]);
	}
 
 
    return -1;
 
}