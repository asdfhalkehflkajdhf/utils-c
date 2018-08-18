#include <zlib.h>
 
#include <stdio.h>
 
#include <sys/types.h>
 
#include <sys/stat.h>
 
#include <fcntl.h>
 
#include <sys/mman.h>
 
#define WF_FAIL -1
#define WF_OK 0

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
    gzFile out = gzopen(DestName,"wb6f");
    
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

        if(gzwrite(out, buf, (unsigned)len) != len)
        {
            re = WF_FAIL;
        }
    }

    gzclose(out);

    fclose(fp_in);

    return re;
 }

 int UnCompress(const char * DestName,const char *SrcName)
{
    FILE * fp_out = NULL;
	int re = WF_OK;
    
    gzFile in;int len = 0;char buf[16384];

    in = gzopen(SrcName,"rb");

    if(in == NULL)
    {
        return WF_FAIL;
    }

    if(NULL == (fp_out = fopen(DestName,"wb")))
    {
        gzclose(in);
        return WF_FAIL;
    }
    
    for (;;)
    {
        len = gzread(in,buf,sizeof(buf));

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
    gzclose(in);

    return re;
}
 //[gz/zip/tar/tar.Z/tar.gz]
int main(int argc, char **argv)
 
{
 
	if(argc !=3)
	{
		printf("exit src dest\n");
		return 0;
	}
	if(Compress(argv[2], argv[1])){
		printf("zip error %s\n", argv[1]);
	}
 
 
    return -1;
 
}