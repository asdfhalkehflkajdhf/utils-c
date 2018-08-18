#include <stdio.h>

#include "../lwca.h"
#include "../utils.h"



int main(int argc, char ** args)
{
	
	
	LWCA_NODE *root;
	root = Lwca_Init(LWCA_MODE_FILE, "conf.txt");
// Lwca_Destroy(LWCA_HEAD *root);

	Lwca_Traverse_Node(root);
	

	printf("%05d\n\n",-1);

	int i=0;
	for(i=0;i<1000;++i){
		
	}
	/*
	//=======================================
	//从 根节点 返回key串的 value 值 （key可以为复串） 
	char *a = Lwca_Get_Value_By_Key(root, "h");
	printf("%s\n", a);
	//=======================================
	*/


	/*
	//=======================================
	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "d:e");
	//返回当前node节点的 value值 
	char *a=Lwca_Get_Value_By_Node(node);
	printf("%x %s\n", node, a);
	//=======================================
	*/
	
	
	/*
	//=======================================
	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "d");
	LWCA_NODE *node2 = Lwca_Get_SubNode_By_Node(node, "e");

	//返回当前node节点的 value值 
	char *a=Lwca_Get_Value_By_Node(node2);
	printf("%x %s\n", node2, a);
	//=======================================
	*/


	/*
	//=======================================
	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "d");		
	//返回当前node节点的 listindex下标vlaue 
	char *a = Lwca_Get_Value_By_ListIndex(node, 0);
	printf(" %s\n",  a);
	//=======================================
	*/

	
	/*
	//=======================================
	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "d");	
	LWCA_NODE *node2 = Lwca_Get_Node_By_ListIndex(node, 1);
	
	char *a=Lwca_Get_Value_By_Node(node2);
	printf("%x %s\n", node2, a);
	//=======================================
	*/	

	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "c");	
	LWCA_NODE *node2 = Lwca_Get_Node_By_ListIndex(node, 0);
	
	char *a=Lwca_Get_Value_By_Node(node);
	printf("%x %s\n", node2, a);
	/*
	//=======================================
	//从 根节点 返回的 key node 节点 （key可以为复串） 
	LWCA_NODE *node = Lwca_Get_Node_By_Key(root, "a");
	int a =Lwca_Get_Node_ListNum_By_Node(node);
	printf("%d\n", a);	
	//=======================================
	*/	



	Lwca_Destroy(root);
	return 0;
}
