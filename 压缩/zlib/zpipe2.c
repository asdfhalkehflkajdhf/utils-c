#include <zlib.h>
 
#include <stdio.h>
 
#include <sys/types.h>
 
#include <sys/stat.h>
 
#include <fcntl.h>
 
#include <sys/mman.h>
 /*
 gcc zpipe2.c -lz
 */
 //[gz/zip/tar/tar.Z/tar.gz]
int main(int argc, char **argv)
 
{
 
    struct stat statf;
 
    gzFile gz_file;
	if(argc !=3)
	{
		printf("exit\n");
		return 0;
	}
 
    char * dstf = argv[2];
 
    char * srcf = argv[1];
 
    char * istart;
 
    int ifd;
 
    int rtn;
 
 
    if (stat(srcf, &statf) != 0)
	{
		printf("a\n");
        goto failed;
	}
    ifd = open(srcf, O_RDONLY);
 
    if (ifd == -1)
	{
		printf("b\n");
        goto failed;
	}
    istart = mmap(NULL, statf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
 
    if (istart != MAP_FAILED){
 
        gz_file = gzopen(dstf, "wb");
 
        if (gz_file != NULL){
 
            rtn = gzwrite(gz_file, istart, statf.st_size);
 
            gzclose(gz_file);
 
            if (rtn == statf.st_size){
 
                printf("compress the file %s ok\n", srcf);
 
                munmap(istart, statf.st_size);
 
                close(ifd);
 
                return 0;
 
            }
 
        }
 
    }
 
 
    close(ifd);
 
failed:
 
    printf("compress the file %s failed\n", srcf);
 
    return -1;
 
}