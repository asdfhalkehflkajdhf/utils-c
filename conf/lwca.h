/*****************************************************************************
Copyright: 2016-2026, liuzc
File name: lwca.h (轻量配置解析 Light weight configuration analysis)
Description: 公共使用接口 

Author: liuzc
Version: 1.0
Date: 完
History: 
	2016/11/22 创建文件 
*****************************************************************************/


#ifndef _LWCA_H__
#define _LWCA_H__


#ifdef __cplusplus
extern "C" {
#endif 

#define LWCA_MODE_FILE 1
#define LWCA_MODE_STR 0

#define LWCA_SEARCH_HASH_NUM 1000

typedef char  LWCA_NODE; 

/*************************************************
Function: 		Lwca_Init
Description:	用户使用返回解析root根结点 
Called By:		

Input:			@mode:文件或是字符串 
				@arg; 参数
				
Output:			
			
				
Return:			LWCA_NODE 根结点 指针， 解析失败返回NULL 

Others:			无 
*************************************************/
LWCA_NODE *Lwca_Init(int mode, char *arg);
/*************************************************
Function: 		Lwca_Destroy
Description:	用户使用销毁root根结点 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:

Others:			无 
*************************************************/
void Lwca_Destroy(LWCA_NODE *root);

/*************************************************
Function: 		Lwca_Traverse_Node
Description:	遍历以当前结点为根的所有子结点信息 
Called By:		

Input:			LWCA_NODE 结点指针 
Output:			无
Return:			无
Others:			无 
*************************************************/
void Lwca_Traverse_Node(LWCA_NODE *root);


/*************************************************
Function: 		Lwca_Get_ListIndex_By_Node
Description:	用户使用root根结点返回当前结点的index 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			index 

Others:			无 
*************************************************/
int Lwca_Get_ListIndex_By_Node(LWCA_NODE *node);
/*************************************************
Function: 		Lwca_Get_Key_By_Node
Description:	用户使用root根结点返回当前结点的key 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			指向key 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Key_By_Node(LWCA_NODE *node);
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	用户使用root根结点返回当前结点的value 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			指向value 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Value_By_Node(LWCA_NODE *node);
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	从 根节点 返回key串的 value 值（key可以为复串） 
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				@key可以为复串				
Output:			
	
Return:			指向value 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Value_By_Key(LWCA_NODE *root, char *key);
/*************************************************
Function: 		Lwca_Get_Node_By_Key
Description:	从 根节点 返回key串的 node结点 指针（key可以为复串） 
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				@key可以为复串				
Output:			
	
Return:			指向node 值的指针 

Others:			无 
*************************************************/
LWCA_NODE *Lwca_Get_Node_By_Key(LWCA_NODE *root, char *key);
/*************************************************
Function: 		Lwca_Get_Value_By_ListIndex
Description:	从 根节点 返回key串的 value值（） 
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				@index 为在当前结点的索引				
Output:			
	
Return:			指向vlaue 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Value_By_ListIndex(LWCA_NODE *root, int index);
/*************************************************
Function: 		Lwca_Get_Node_By_ListIndex
Description:	从 根节点 返回key串的 node 指针（） 
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				@index 为在当前结点的索引				
Output:			
	
Return:			指向node 值的指针 

Others:			无 
*************************************************/
LWCA_NODE *Lwca_Get_Node_By_ListIndex(LWCA_NODE *root, int index);
/*************************************************
Function: 		Lwca_Get_Node_ListNum_By_Node
Description:	//返回当前node节点的list个数  
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				
Output:			
	
Return:			//返回当前node节点的list个数 

Others:			无 
*************************************************/
int Lwca_Get_Node_ListNum_By_Node(LWCA_NODE *root);
/*************************************************
Function: 		Lwca_Get_Value_By_LayerKey
Description:	//hash查找 ,只有在所有key值不能时会有用 
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				@layer
				@key 只能是单串 
Output:			
	
Return:			//返回当前key节点的value 

Others:			无 
*************************************************/
char *Lwca_Get_Value_By_LayerKey(LWCA_NODE *root, int layer, char *key);



// ****************************************************************************

#ifdef __cplusplus
}
#endif  // cplusplus 

#endif  //  

// ****************************************************************************
