
#ifndef _MEM_POOL_H_INCLUDED_
#define _MEM_POOL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif 

#include "list.h"

typedef struct mem_pool_obj
{
	struct list_head  unused_list;
	struct list_head  pool_list;
	unsigned int     inc_count;
	unsigned int     objcount;
	unsigned int     objsize;
	unsigned int     used_count;        
	unsigned int     unused_count;        
	char             *name;        
	pthread_spinlock_t lock;
}mem_pool_o;

// ****************************************************************************
mem_pool_o *mem_p_obj_create(int objsize, int objcount, int inc, char *name);
void mem_p_obj_destroy(mem_pool_o *mem_pool);


void *mem_p_obj_malloc(mem_pool_o *mem_pool);
//void *mem_p_obj_calloc(mem_pool_o *mem_pool, unsigned int num);
void mem_p_obj_free(void *ptr);


void *mem_p_obj_malloc_safe(mem_pool_o *mem_pool);
//void *mem_p_obj_calloc_safe(mem_pool_o *mem_pool, unsigned int num);
void mem_p_obj_free_safe(void *ptr);


#ifdef __cplusplus
}
#endif  // cplusplus 

#endif  //  

// ****************************************************************************
