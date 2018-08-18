#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"


#define OBJ_COUNT 2000
int main(int argc, const char *argv[])
{
	mem_pool_o *mem_pool=NULL;
	struct timeval tval;
	int sec =0;

	mem_pool = mem_p_obj_create(1024, OBJ_COUNT, 10, "asdf");
	if(mem_pool == NULL){
		printf("create fail\n");
		return 0;
	}
	char *b[OBJ_COUNT+5];
	int i;
	gettimeofday(&tval, NULL);
	sec = tval.tv_usec;
	for(i=0; i<OBJ_COUNT; i++){
		b[i]=mem_p_obj_malloc(mem_pool);
		if(b[i]==NULL){
			printf("%d malloc error\n", i);
		}
	}
	gettimeofday(&tval, NULL);
	printf("%d\n", tval.tv_usec -sec);
	for(i=0; i<3; i++){
		mem_p_obj_free(b[i]);
	}
	for(i=OBJ_COUNT; i<OBJ_COUNT+5; i++){
		b[i]=mem_p_obj_malloc(mem_pool);
		if(b[i]==NULL){
			printf("%d malloc error 2\n", i);
		}
	}
	
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
