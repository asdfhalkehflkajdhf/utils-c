#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mem_pool.h"

typedef struct mem_node_head
{
	mem_pool_o *mem_pool_h;
	struct list_head list;
}mem_node_h;
typedef struct mem_pool_list
{
	struct list_head list;
}mem_pool_l;

#define MEM_BLC_HEAD_OFFSET (sizeof(mem_pool_l))
#define MEM_OBJ_HEAD_OFFSET (sizeof(mem_node_h))

#define MEM_OBJ_HEAD_GET(ptr) ((mem_node_h *)((void *)ptr-MEM_OBJ_HEAD_OFFSET))



inline mem_node_h* MEM_OBJ_ALLOC_GET_HEAD(void *buff, mem_pool_o *mem_pool_h, unsigned int index, int objsize)
{
	mem_node_h *ptr=NULL;
	ptr = (mem_node_h *)(buff + index*(MEM_OBJ_HEAD_OFFSET+(objsize)));
	if(ptr==NULL){
		return NULL;
	}
	ptr->mem_pool_h = mem_pool_h;
	list_add_tail(&ptr->list, &mem_pool_h->unused_list);
	return ptr;
}


/*
return: pool buff ptr
*/
void *mem_p_blc_calloc_get_buff(mem_pool_o *mem_pool, unsigned int objcount)
{
	void *buff=NULL;
	mem_pool_l *newpool=NULL;
	int block_size;
	block_size = objcount*(mem_pool->objsize + MEM_OBJ_HEAD_OFFSET) + MEM_BLC_HEAD_OFFSET;

	buff = calloc(1, block_size);
	if(buff == NULL){
		return NULL;
	}
	newpool = (mem_pool_l *)buff;

	list_add_tail(&newpool->list, &mem_pool->pool_list);
	return buff+MEM_BLC_HEAD_OFFSET;
}

/*
	mem_p_obj*  是基于对象结构的api
*/
mem_pool_o *mem_p_obj_create(int objsize, int objcount, int inc, char *name)
{
	mem_pool_o *mem_pool=NULL;
	if(objsize<=0 || objcount<=0 || name == NULL){
		return NULL;
	}
	mem_pool = (mem_pool_o *)calloc(1, sizeof(mem_pool_o));
	if(mem_pool == NULL){
		return NULL;
	}
	mem_pool->inc_count = inc;
	mem_pool->objsize = objsize;
	mem_pool->unused_count = objcount;
	mem_pool->used_count = 0;
	mem_pool->objcount = objcount;
	INIT_LIST_HEAD(&mem_pool->pool_list);
	INIT_LIST_HEAD(&mem_pool->unused_list);
	pthread_spin_init(&mem_pool->lock, PTHREAD_PROCESS_PRIVATE);
	mem_pool->name = strdup(name);
	if(mem_pool->name == NULL){
		printf("mem pool name error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}
	void *pool_buff = mem_p_blc_calloc_get_buff(mem_pool, mem_pool->objcount);
	if(pool_buff == NULL){
		printf("mem pool name error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}
	int i;
	mem_node_h *node=NULL;
	for(i=0; i<objcount; i++){
		node=MEM_OBJ_ALLOC_GET_HEAD(pool_buff, mem_pool, i, objsize);
		list_add_tail(&node->list, &mem_pool->unused_list);
	}

	return mem_pool;
}
/*
	最好是在所以malloc free 之后再执行
*/
void mem_p_obj_destroy(mem_pool_o *mem_pool)
{
	mem_pool_l *node;
	mem_pool_l *tem_node;
	if(mem_pool == NULL){
		return ;
	}
	if(mem_pool->name!=NULL){
		free(mem_pool->name);
	}

	list_for_each_entry_safe(node, tem_node, &mem_pool->pool_list, list){
		list_del(&node->list);
		free(node);
	}

    pthread_spin_destroy(&mem_pool->lock);
	free(mem_pool);
	return ;
}

static int mem_p_obj_realloc(mem_pool_o *mem_pool)
{
	if(mem_pool->inc_count == 0){
		return -1;
	}
	void *pool_buff = mem_p_blc_calloc_get_buff(mem_pool, mem_pool->inc_count);
	if(pool_buff == NULL){
		return -1;
	}
	int i;
	mem_node_h *node=NULL;
	for(i=0; i<mem_pool->inc_count; i++){
		node=MEM_OBJ_ALLOC_GET_HEAD(pool_buff, mem_pool, i, mem_pool->objsize);
		list_add_tail(&node->list, &mem_pool->unused_list);
	}
	mem_pool->unused_count+=mem_pool->inc_count;

	return 0;
}

void *mem_p_obj_malloc(mem_pool_o *mem_pool)
{
	if(mem_pool->unused_count == 0){
		if(mem_pool->inc_count == 0 || -1 == mem_p_obj_realloc(mem_pool)){
			return NULL;
		}
	}
	mem_pool->unused_count--;
	mem_pool->used_count++;
	mem_node_h *node=NULL;
	mem_node_h *tem_node;

	list_for_each_exit_safe(node,tem_node, &mem_pool->unused_list, list){
		list_del(&node->list);
		break;
	}
	return node?(void *)node+MEM_OBJ_HEAD_OFFSET:(void *)node;
}
void mem_p_obj_free(void *ptr)
{
	if(ptr == NULL){
		return ;
	}
	mem_node_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;

	mem_pool->unused_count++;
	mem_pool->used_count--;

	list_add_tail(&ptr_h->list, &mem_pool->unused_list);
	return ;
}


void *mem_p_obj_malloc_safe(mem_pool_o *mem_pool)
{
	void *res=NULL;
	pthread_spin_lock(&mem_pool->lock);
	res = mem_p_obj_malloc(mem_pool);
	pthread_spin_unlock(&mem_pool->lock);
	return res;
}
void mem_p_obj_free_safe(void *ptr)
{
	mem_node_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	pthread_spin_lock(&mem_pool->lock);
	mem_p_obj_free(ptr);
	pthread_spin_unlock(&mem_pool->lock);
	return ;
}




