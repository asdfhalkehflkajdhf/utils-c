#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "random.h"

unsigned int randseed;

int main(int argc, const char *argv[])
{
	random_u32_init(&randseed);
	int i;
	for(i=0; i<100; i++)
	{
		printf("%u\n", random_u32(&randseed));
	}
    return 0;
}
