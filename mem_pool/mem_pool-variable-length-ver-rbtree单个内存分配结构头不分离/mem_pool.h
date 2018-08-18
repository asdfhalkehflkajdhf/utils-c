
#ifndef _MEM_POOL_H_INCLUDED_
#define _MEM_POOL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif 

#include "rbtree/rbtree_augmented.h"
#include "list.h"

typedef struct mem_pool_obj
{
	void  *unused_hash;//未使用块信息表
	struct rb_root rb_root;
	struct list_head  pool_list;//buff内存块表
	unsigned int     inc_count;//可否自增加
	unsigned int     objcount;//最小块个数和对象个数
	unsigned int     objsize;//最小可用块或是固定对象结构大小
	//unsigned int     used_count;        
	//unsigned int     unused_count;        
	char             *name;//pool名
	pthread_spinlock_t lock;
}mem_pool_o;

// ****************************************************************************
mem_pool_o *mem_p_obj_create(int objsize, int objcount, int inc, char *name);
void mem_p_obj_destroy(mem_pool_o *mem_pool);


void *mem_p_obj_malloc(mem_pool_o *mem_pool, int size);
//void *mem_p_obj_calloc(mem_pool_o *mem_pool, unsigned int num);
void mem_p_obj_free(void *ptr);


void *mem_p_obj_malloc_safe(mem_pool_o *mem_pool, int size);
//void *mem_p_obj_calloc_safe(mem_pool_o *mem_pool, unsigned int num);
void mem_p_obj_free_safe(void *ptr);


#ifdef __cplusplus
}
#endif  // cplusplus 

#endif  //  

// ****************************************************************************
