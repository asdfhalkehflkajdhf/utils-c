#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"

void *mem_p_show_list(mem_pool_o *mem_pool);

int OBJ_COUNT=5;
int main(int argc, const char *argv[])
{
	mem_pool_o *mem_pool=NULL;
	struct timeval tval;
	int sec =0;
	if(argc !=2){
		printf("usage NULL\n");
		return 0;
	}
	OBJ_COUNT = atoi(argv[1]);
	mem_pool = mem_p_obj_create(1024, OBJ_COUNT, 10, "asdf");
	if(mem_pool == NULL){
		printf("create fail\n");
		return 0;
	}
	char *b[OBJ_COUNT+5];
	int i=1;

	gettimeofday(&tval, NULL);
	sec = tval.tv_usec;
	for(i=0; i<OBJ_COUNT; i++)
	{
		b[i]=mem_p_obj_malloc(mem_pool, 800);
		/*
		if(b[i]==NULL)
		{
			printf("%d malloc error\n", i);
		}
		*/
	}
	gettimeofday(&tval, NULL);
	printf("%d \n", tval.tv_usec -sec);
	mem_p_obj_free(b[3]);
	mem_p_obj_free(b[4]);
	//mem_p_show_list(mem_pool);
	for(i=5; i<OBJ_COUNT; i++)
	{
		mem_p_obj_free(b[i]);
	}
	for(i=OBJ_COUNT; i<OBJ_COUNT+5; i++){
		b[i]=mem_p_obj_malloc(mem_pool, 500);
		if(b[i]==NULL){
			printf("%d malloc error 2\n", i);
		}
	}
	/*
	*/
	mem_p_obj_destroy(mem_pool);

	
	gettimeofday(&tval, NULL);
	sec = tval.tv_usec;
	for(i=0; i<OBJ_COUNT; i++){
		b[i]=malloc(1024);
		if(b[i]==NULL){
			printf("%d malloc error\n", i);
		}
	}
	gettimeofday(&tval, NULL);
	printf("%d\n", tval.tv_usec -sec);
	for(i=0; i<OBJ_COUNT; i++){
		if(b[i]!=NULL){
			free(b[i]);
		}
	}
    return 0;
}
