#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mem_pool.h"

typedef struct mem_black_head
{
	mem_pool_o *mem_pool_h;
	unsigned int index;
	unsigned int b_count;
}mem_black_h;

#define MEM_OBJ_HEAD_OFFSET (sizeof(mem_black_h))
#define MEM_OBJ_HEAD_GET(ptr) ((mem_black_h *)((void *)ptr-MEM_OBJ_HEAD_OFFSET))
inline void DIRTY_SET(void *ptr, int num)
{
	mem_black_h *ptr_h=MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	memset(&mem_pool->used_index[ptr_h->index], 1, num);
	return ;
}
inline void DIRTY_CLR(void *ptr, int num)
{
	mem_black_h *ptr_h=MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	memset(&mem_pool->used_index[ptr_h->index], 0, num);
	return ;
}

inline void MEM_OBJ_HEAD_SET(void *ptr, int num)
{
	mem_black_h *ptr_h=MEM_OBJ_HEAD_GET(ptr);
	
	ptr_h->b_count = num;
	//DIRTY_SET(ptr, num);

	return;
}


inline void* MEM_OBJ_ALLOC(void *buff, mem_pool_o *mem_pool_h, unsigned int index, int objsize)
{
	void *ptr=NULL;
	if(mem_pool_h->create_type == LIST){
		ptr = malloc(MEM_OBJ_HEAD_OFFSET+(objsize));
	}else if(buff != NULL){
		ptr = buff + index*(MEM_OBJ_HEAD_OFFSET+(objsize));
	}else{
		return NULL;
	}
	if(ptr==NULL){
		return NULL;
	}
	mem_black_h *ptr_h = (mem_black_h *)ptr;
	ptr_h->b_count = 0;
	ptr_h->mem_pool_h = mem_pool_h;
	ptr_h->index = index;
	return ptr+MEM_OBJ_HEAD_OFFSET;
}

/*
	mem_p_obj*  是基于对象结构的api
*/
mem_pool_o *mem_p_obj_create(int type, int objsize, int objcount, int inc, char *name)
{
	mem_pool_o *mem_pool=NULL;
	if(objsize<=0 || objcount<=0 || name == NULL){
		return NULL;
	}
	mem_pool = (mem_pool_o *)malloc(sizeof(mem_pool_o));
	if(mem_pool == NULL){
		return NULL;
	}
	memset(mem_pool, 0, sizeof(mem_pool_o));
	mem_pool->objcount = objcount;
	pthread_spin_init(&mem_pool->lock, PTHREAD_PROCESS_PRIVATE);
	mem_pool->name = strdup(name);
	if(mem_pool->name == NULL){
		printf("mem pool name error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}
	
	mem_pool->used_index= (unsigned char*)malloc(sizeof(unsigned char *) * (objcount));
	mem_pool->unused_list = (void **)malloc(sizeof(void*) * (objcount));
	mem_pool->obj_p_index= (void **)malloc(sizeof(void*) * (objcount));
	if(mem_pool->used_index == NULL 
		|| mem_pool->unused_list == NULL
		|| mem_pool->obj_p_index == NULL ){
		printf("malloc error\n");
		mem_p_obj_destroy(mem_pool);
		return NULL;
	}
	memset(mem_pool->used_index, 0, sizeof(unsigned char *) * (objcount));
	mem_pool->create_type = type;
	unsigned int i;
	if(type == LIST){
		for(i=0; i<objcount; i++){
			mem_pool->obj_p_index[i]=(void *)MEM_OBJ_ALLOC(NULL, mem_pool, i, objsize);
			mem_pool->unused_list[i]=mem_pool->obj_p_index[i];
			if(mem_pool->obj_p_index[i] == NULL){
				printf("obj malloc error\n");
				mem_p_obj_destroy(mem_pool);
				return NULL;
			}
		}
	}else if(type==BLOCK){
		mem_pool->obj_p_index[0]=(void *)malloc((objcount) *(objsize+MEM_OBJ_HEAD_OFFSET));
		for(i=0; i<objcount; i++){
			mem_pool->unused_list[i]=(void *)MEM_OBJ_ALLOC(mem_pool->obj_p_index[0], mem_pool, i, objsize);
		}
	}
	
	mem_pool->inc_count = inc;
	mem_pool->objsize = objsize;
	mem_pool->unused_count = objcount;
	mem_pool->used_count = 0;
	mem_pool->unused_head = 0;
	mem_pool->unused_tail = 0;
	
	return mem_pool;
}
/*
	最好是在所以malloc free 之后再执行
*/
static void mem_p_obj_destroy_sub(mem_pool_o *mem_pool)
{
	if(mem_pool->used_index != NULL){
		free(mem_pool->used_index);
	}
	if(mem_pool->obj_p_index != NULL){
		void *t=NULL;
		if(mem_pool->create_type == LIST){
			int i;
			for(i=0; i<mem_pool->objcount; i++){
				t=MEM_OBJ_HEAD_GET(mem_pool->obj_p_index[i]);
				if(t != NULL){
					free(t);
				}
				t=NULL;
			}
		}else if(mem_pool->create_type == BLOCK){
			t=mem_pool->obj_p_index[0];
			if(t != NULL){
				free(t);
			}
		}
		
		free(mem_pool->obj_p_index);
	}
	if(mem_pool->unused_list != NULL){
		free(mem_pool->unused_list);
	}
	return ;
}
void mem_p_obj_destroy(mem_pool_o *mem_pool)
{
	if(mem_pool == NULL){
		return ;
	}
	if(mem_pool->name!=NULL){
		free(mem_pool->name);
	}
	mem_p_obj_destroy_sub(mem_pool);

    pthread_spin_destroy(&mem_pool->lock);
	free(mem_pool);
	return ;
}

static int mem_p_obj_realloc(mem_pool_o *mem_pool)
{
	void **unused_list;
	void **obj_p_index;
	unsigned char	 *used_index;
	unsigned int oldobjcount = mem_pool->objcount;

	unsigned int newobjcount = mem_pool->objcount+mem_pool->inc_count;
	used_index= (unsigned char*)malloc(sizeof(unsigned char *) * (newobjcount));
	unused_list = (void **)malloc(sizeof(void*) * (newobjcount));
	obj_p_index= (void **)malloc(sizeof(void*) * (newobjcount));
	if(used_index == NULL 
		|| unused_list == NULL
		|| obj_p_index == NULL ){
		printf("mem_p_obj_realloc malloc error\n");
		return -1;
	}
	memcpy((void *)unused_list, (void *)mem_pool->unused_list, sizeof(unsigned char *) * (oldobjcount));
	memcpy((void *)used_index, (void *)mem_pool->used_index, sizeof(void *) * (oldobjcount));
	memcpy((void *)obj_p_index, (void *)mem_pool->obj_p_index, sizeof(void *) * (oldobjcount));
	mem_pool->objcount = newobjcount;

	unsigned int i;
	unsigned int unused_cur_h = mem_pool->unused_head % newobjcount;
	unsigned int unused_cur_t = mem_pool->unused_tail% newobjcount;
	//void *d=unused_list+sizeof(void *)*(unused_cur_h+mem_pool->inc_count-1);
	//void *s=unused_list+sizeof(void *)*(unused_cur_h);
	for(i= unused_cur_h; i< newobjcount; i++){
		unused_list[i+mem_pool->inc_count-1]=unused_list[i];
	}
	//memcpy(d, s, sizeof(void *)*mem_pool->inc_count);
//printf("%p %p\n", d, s);
	int j=unused_cur_h;

	if(mem_pool->create_type == LIST){
		for(i=oldobjcount; i<newobjcount; i++){
			obj_p_index[i]=(void *)MEM_OBJ_ALLOC(NULL, mem_pool, i, mem_pool->objsize);
			unused_list[j++]=obj_p_index[i];
			if(obj_p_index[i] == NULL){
				printf("mem_p_obj_realloc malloc error\n");
				return -1;
			}
			
		}
	}else if(mem_pool->create_type==BLOCK){
		obj_p_index[0]=(void *)malloc((newobjcount) *(mem_pool->objsize+MEM_OBJ_HEAD_OFFSET));
		if(obj_p_index[0] == NULL){
			return -1;
		}
		for(i=oldobjcount; i<newobjcount; i++){
			unused_list[j++]=(void *)MEM_OBJ_ALLOC(obj_p_index[0], mem_pool, i, mem_pool->objsize);
		}
	}

	if(unused_cur_t >=unused_cur_h){
		mem_pool->unused_tail += mem_pool->inc_count;
	}

	mem_p_obj_destroy_sub(mem_pool);

	mem_pool->unused_list=unused_list;
	mem_pool->used_index=used_index;
	mem_pool->obj_p_index=obj_p_index;

	mem_pool->unused_count+=mem_pool->inc_count;

	return 0;
}

void *mem_p_obj_malloc(mem_pool_o *mem_pool)
{
	unsigned int unused_cur;
	if(mem_pool->unused_count == 0){
		if(mem_pool->inc_count == 0 || -1 == mem_p_obj_realloc(mem_pool)){
			return NULL;
		}
	}
	mem_pool->unused_count--;
	mem_pool->used_count++;

	unused_cur = (mem_pool->unused_head) % mem_pool->objcount;

	MEM_OBJ_HEAD_SET(mem_pool->unused_list[unused_cur],1);
	mem_pool->unused_head++;
	return mem_pool->unused_list[unused_cur];
}
void *mem_p_obj_calloc(mem_pool_o *mem_pool, unsigned int num)
{
	unsigned int unused_cur;
	if(mem_pool->unused_count < num){
		if(mem_pool->inc_count == 0 || -1 == mem_p_obj_realloc(mem_pool)){
			return NULL;
		}
	}
	void **dest = (void **)malloc(sizeof(void *)* num);
	if(dest == NULL){
		return NULL;
	}
	mem_pool->unused_count-=num;
	mem_pool->used_count+=num;

	unused_cur = (mem_pool->unused_head) % mem_pool->objcount;

	MEM_OBJ_HEAD_SET(mem_pool->unused_list[unused_cur],num);
	
	mem_pool->unused_head+=num;	

	int i;
	for(i=0; i<num; i++){
		dest[i] = mem_pool->unused_list[unused_cur % mem_pool->objcount];
		unused_cur--;
	}

	return (void *)dest;
}
void mem_p_obj_free(void *ptr)
{
	if(ptr == NULL){
		return ;
	}
	mem_black_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;

	mem_pool->unused_count+=ptr_h->b_count;
	mem_pool->used_count-=ptr_h->b_count;

	int used_cur = mem_pool->unused_tail % mem_pool->objcount;
	mem_pool->unused_tail+=ptr_h->b_count;
	printf("f %d %d %d\n", used_cur, mem_pool->unused_tail, mem_pool->objcount);
	if(ptr_h->b_count != 1){
		int i;
		void **dest=(void **)ptr;
		for(i=0; i<=ptr_h->b_count; i++){
			mem_pool->unused_list[used_cur % mem_pool->objcount]=dest[i];
			used_cur++;
		}
		free(dest);
	}else{
		mem_pool->unused_list[used_cur % mem_pool->objcount]=ptr;
	}
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
void *mem_p_obj_calloc_safe(mem_pool_o *mem_pool, unsigned int num)
{
	void *res=NULL;
	pthread_spin_lock(&mem_pool->lock);
	res = mem_p_obj_calloc(mem_pool, num);
	pthread_spin_unlock(&mem_pool->lock);
	return res;
}
void mem_p_obj_free_safe(void *ptr)
{
	mem_black_h *ptr_h = MEM_OBJ_HEAD_GET(ptr);
	mem_pool_o *mem_pool=ptr_h->mem_pool_h;
	pthread_spin_lock(&mem_pool->lock);
	mem_p_obj_free(ptr);
	pthread_spin_unlock(&mem_pool->lock);
	return ;
}




