#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

typedef struct session_node
{
	struct list_head list;
	int status;
}session_node_t;

typedef struct session_hash
{
	struct list_head h_head;	
}session_hash_t;

session_hash_t g_cw_fs_hash;



int main(int argc, const char *argv[])
{
	INIT_LIST_HEAD(&g_cw_fs_hash.h_head);
	session_node_t *new_node;
	session_node_t *temp_node;
	session_node_t *node;
	int i;
	
	for(i=0; i<10; i++)
	{
		new_node = (session_node_t *)malloc(sizeof(session_node_t));
		if(new_node != NULL)
		{
			new_node->status = i;
			
			list_add_head(&new_node->list,&g_cw_fs_hash.h_head);
		}
	}
	
	for(i=0; i<5;i++)
	{
		list_for_each_entry_safe(node, temp_node, &g_cw_fs_hash.h_head, list)
		{
			list_del(&node->list);
			free(node);
			break;
		}
	}
/*show all list*/
	list_for_each_entry_safe(node, temp_node, &g_cw_fs_hash.h_head, list)
	{
		printf("%d \t", node->status);
	}
	printf("\n");


	
	list_for_each_entry_safe(node, temp_node, &g_cw_fs_hash.h_head, list)
	{
		list_del(&node->list);
		free(node);
	}	

    return 0;
}
