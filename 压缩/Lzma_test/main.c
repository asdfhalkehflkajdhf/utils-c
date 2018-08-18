#include <stdio.h>
 
#include <sys/types.h>
 
#include <sys/stat.h>
 
#include <fcntl.h>
 
#include "../Lzma/LzmaUtil.h"

int main(int argc, char **argv)
{
 	if(argc !=3)
	{
		printf("usage: <src> <dest>\n");
		return 0;
	}
   LzmaCompress(argv[1], argv[2]);
   return 0;
}