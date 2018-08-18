/*****************************************************************************
Copyright: 2016-2026, liuzc
File name: lwca (轻量配置解析 Light weight configuration analysis)
Description: 用于详细说明此程序文件完成的主要功能， 与其他模块或函数的接口， 输出
值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系

Author: liuzc
Version: 1.0
Date: 完
History: 
	2016/11/22 创建文件 
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "utils.h"
#include "include/list.h"

#define LWCA_TRUE 1
#define LWCA_FALSE 0

#define LWCA_MODE_FILE 1
#define LWCA_MODE_STR 0

#define LWCA_SEARCH_HASH_NUM 1000

typedef char  LWCA_NODE; 
//配置文件解析头 
typedef struct list_head LWCA_LIST_HEAD;



typedef struct LWCA_RUNING_INFO_NODE_{
	
	LWCA_NODE *lwca_hash[LWCA_SEARCH_HASH_NUM]; //查找hash  
	
	int cur_layer;//层 
	int cur_align; //对齐 
	char *cur_k_start;
	char * cur_k_end;

	char *cur_v_start;
	char * cur_v_end;	//指向第一个\0 
	char *cur_tmp;//当前指向的头 
	
	int buffer_size;
	char *buffer;
	
}LWCA_RUNING_INFO_NODE;


typedef struct LWCA_LIST_NODE_{
	int layer;		//层级 
	int indicia;	//标号 
	char *key;
	char *value;
	
	LWCA_LIST_HEAD list;//同级list 
	LWCA_LIST_HEAD next;//下级list 
	LWCA_LIST_HEAD hash_list;

	int list_num;	
		
	struct LWCA_LIST_NODE_ **hash;
	int hash_key;

	LWCA_RUNING_INFO_NODE *root;
}LWCA_LIST_NODE;



/*************************************************
Function: 		Lwca_F_Init
Description:	初始化配置文件解析信息 
Called By:		Lwca_Init

Input:			配置文件路径 
Output:			文件解析信息结构
Return:			文件解析信息结构
Others:
*************************************************/
static LWCA_RUNING_INFO_NODE *Lwca_F_Init(const char const *file_path)
{
	int res;
	int fileSize;
	char *buffer;
	FILE *tfd;
	
	LWCA_RUNING_INFO_NODE *root;
	
	//return 0:yes  -1:no
	res = is_file(file_path);
	if(0 != res){
		printf("[ERROR] [%s] is not file!\n",file_path);
		return NULL;
	}
	
	fileSize = get_file_size(file_path);
	if(0 >= fileSize){
		printf("[ERROR] [%s] is null!\n",file_path);
		return NULL;
	}	

	tfd = fopen(file_path,"r");
	if(tfd < 0){
		printf("[ERROR] [%s] is not open(%d)!\n",file_path, tfd);
		return NULL;
	}
	
	buffer = (char *)malloc(fileSize+1);
	memset(buffer, 0, fileSize+1);
	
	while(res !=  fileSize){
		int i = fread(buffer+res, fileSize-res, 1,tfd);
		res = res+ i;
		if(i<0)break;
		if(i==0){res = fileSize;}
	}
	
	if(res < fileSize){
		fclose(tfd);
		free(buffer);
		printf("[ERROR] [%s] is not read!\n",file_path);
		return NULL;
	}
	
	root = (LWCA_RUNING_INFO_NODE *)malloc(sizeof(LWCA_RUNING_INFO_NODE));
	if(root == NULL){
		fclose(tfd);
		free(buffer);
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	

	int i=0;
	for(i=0; i<LWCA_SEARCH_HASH_NUM; ++i){
		root->lwca_hash[i]=NULL;
	}
	
	root->cur_layer = 0;
	
	root->cur_align = 0;
	root->cur_k_start = NULL;
	root->cur_k_end = NULL;
	root->cur_v_start = NULL;
	root->cur_v_end = NULL;
	root->buffer_size = fileSize;
	root->buffer = buffer;
	root->cur_tmp = buffer;
	
	return root;
}
/*************************************************
Function: 		Lwca_S_Init
Description:	初始化配置字符串解析信息 
Called By:		Lwca_Init

Input:			字符串 
Output:			字符串解析信息结构
Return:			字符串解析信息结构
Others:
*************************************************/
static LWCA_RUNING_INFO_NODE *Lwca_S_Init(const char const *string)
{
	int res;
	int fileSize;
	char *buffer;
	int tfd;
	
	LWCA_RUNING_INFO_NODE *root;
	
	fileSize = strlen(string);
	if(0 >= fileSize){
		printf("[ERROR] string is null!\n");
		return NULL;
	}	

	
	buffer = (char *)malloc(fileSize);
	if(buffer == NULL){
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	
	strcpy(buffer, string);
	
	root = (LWCA_RUNING_INFO_NODE *)malloc(sizeof(LWCA_RUNING_INFO_NODE));
	if(root == NULL){
		close(tfd);
		free(buffer);
		printf("[ERROR] malloc false!\n");
		return NULL;
	}
	
	
	int i=0;
	for(i=0; i<LWCA_SEARCH_HASH_NUM; ++i){
		root->lwca_hash[i]=NULL;
	}
	
	root->cur_layer = 0;
	root->cur_align = 0;
	root->cur_k_start = NULL;
	root->cur_k_end = NULL;
	root->cur_v_start = NULL;
	root->cur_v_end = NULL;
	root->buffer_size = fileSize;
	root->buffer = buffer;
	root->cur_tmp = buffer;
		
	return root;
}
//返回下一行头指针 
char *utils_find_newline_noset(char *string)
{
	int i=0;
	while(*(string+i)!='\0' && *(string+i) != '\n'){
		i++;
	}
	
	if(*(string+i) == '\0'){
		return NULL;
	}


	return (string+i+1);
}
//返回下一行头指针 
char *utils_find_newline(char *string)
{
	int i=0;
	while(*(string+i)!='\0' && *(string+i) != '\n'){
		i++;
	}
	
	if(*(string+i) == '\0'){
		return NULL;
	}
	
	*(string+i)='\0';
	if(*(string+i-1) == '\r'){
		*(string+i-1)='\0';		
	}
	
	return (string+i+1);
}
//去空格 \r 返回第一个\0 
char * utils_move_right_SPACE_R(char *string)
{
	int i=1;
	while(*(string-i)==' ' || *(string-i) == '\r'){
		*(string-i)='\0';
		i++;
	}
	return string-i+1;
}
/*
返回值：
NULL 表示结束
非空：当前表示扫描的最的一个地址 
*/ 

char *utils_find_k_head(char *string, LWCA_RUNING_INFO_NODE *root)
{

	int sum=0;
	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				return NULL;
			}
			if(string[i] == '\t'){
				sum = sum +4;
			}else{
				++sum;
			}
			++i;
		}
		//是一空行 
		if(*(string+i) == '\n'){
			++i;
			sum=0;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			i=i+2;
			sum=0;
		}else if(*(string+i) == '#'){
			string = utils_find_newline(string+i);
			i=0;
			sum=0; 
		}else{
			root->cur_align=sum;
			root->cur_k_start=string+i;	
			break;
		}
		
	}

	root->cur_align=sum;
	return root->cur_k_start;
}
/*
返回值：
NULL 表示结束
非空：当前表示扫描的最的一个地址 
*/ 
char *utils_find_v_head(char *string, LWCA_RUNING_INFO_NODE *root)
{

	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				root->cur_v_start = NULL;
				return NULL;
			}

			++i;
		}
		//是一空行 
		if(*(string+i) == '\n'){
			root->cur_v_start = NULL;
			break;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			root->cur_v_start = NULL;
			break;
		}else if(*(string+i) == '#'){
			root->cur_v_start = NULL;
			string = utils_find_newline(string+i);
			break;
		}else{
			root->cur_v_start=string+i;
			break;
		}
		
	}

	return string+i;
}
/*
返回值：
NULL 表示结束
非空：当前表示扫描的最的一个地址 
*/ 
char *utils_find_k_end(char *string, LWCA_RUNING_INFO_NODE *root )
{
	int i=0;
	char *end;
	int j=0;

	while( *(string+i) != ':' ){
		if(*(string+i) == '\n' || *(string+i)=='\0' ){
			//一行内没有key value  分割符号 
			return NULL;
		}
		i++;
	}
	*(string+i)='\0'; 

	end = utils_move_right_SPACE_R(string+i);



	root->cur_k_end=end;
		
	return (string+i+1);
}


/*
返回值：
NULL 表示结束
非空：当前表示扫描的最的一个地址 
*/ 
char *utils_find_v_end(char *string, LWCA_RUNING_INFO_NODE *root)
{
	int i=0;
	char *end;
	int j=0;
	

	switch(*string){
		case '"':{
			i=1;
			while(*(string + i) != '"'){
				++i;
			}
			if(*(string + i+1) == '\n' || *(string + i+1) == '\r')break;
			*(string + i+1)='\0';
			break;
		}
		case '{':{
			int k=1;
			i=1;
			while(k !=0 ){
				if( *(string + i) == '{' ){
					++k;
				}
				if( *(string + i) == '}' ){
					--k;
				}
				++i;
			}
			if(*(string + i) == '\n' || *(string + i) == '\r')break;
			*(string + i)='\0';
			break;
		}
		
		
		case '[':{
			break;
		}
		default:{
			i=0;
			while(1){
				
				if(*(string+i)=='#' ){
					break;
				}
				if( string[i] == '\n' ){
					break;
				}
				if(*(string+i)=='\0'){
					return NULL;
				}
				++i;
			}
			
			break;
		}
		
	}
	
	
	//去左空格 
	end = utils_move_right_SPACE_R(string+i);
	root->cur_v_end=end;

	//换新行 
	return utils_find_newline(string+i);;
}

int utils_find_headspace_num(char *string)
{
	int sum=0;
	int i=0;

	while(1){

		while(*(string+i)!='\0' && (string[i] == ' ' ||  string[i] == '\t')){
			if(*(string+i)=='\0'){
				return sum;
			}
			if(string[i] == '\t'){
				sum = sum +4;
			}else{
				++sum;
			}
			++i;
		}
		//是一空行 
		if(*(string+i) == '\n'){
			++i;
			sum=0;
		}else  if(*(string+i)=='\r' && *(string+i+1) == '\n'){
			i=i+2;
			sum=0;
		}else if(*(string+i) == '#'){
			string = utils_find_newline_noset(string+i);
			i=0;
			sum=0; 
		}else{
			break;
		}
		
	}

	return sum;
} 


void utils_traverse(LWCA_LIST_HEAD *head)
{
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		int i=0;
		for(i=0; i<node->layer; ++i){
			printf("\t");
		}
		printf("%d|%d|%d [%s]:[%s]\n", node->layer, node->indicia, node->list_num, node->key, node->value);
		
		utils_traverse(&node->next);
	}
	
}
/*************************************************
Function: 		Lwca_Traverse_Node
Description:	遍历以当前结点为根的所有子结点信息 
Called By:		

Input:			LWCA_NODE 结点指针 
Output:			无
Return:			无
Others:			无 
*************************************************/
void Lwca_Traverse_Node(LWCA_NODE *root)
{
	if(root == NULL){
		printf("Lwca_Traverse_Node parameter is empty\n");
		return; 
	}
	LWCA_LIST_NODE *root2 = (LWCA_LIST_NODE *)root;
	utils_traverse(&root2->next);
}

void utils_serach_hash_set(int layer, char *key, LWCA_RUNING_INFO_NODE *root, LWCA_LIST_NODE *new_node)
{
	char k[100]={0};
	sprintf(k, "%05d%s", layer-1, key);
	
	unsigned t = str_hash(k)%1000;

	
	if(root->lwca_hash[t]){
		LWCA_LIST_NODE *node=NULL;
		node = (LWCA_LIST_NODE *)root->lwca_hash[t];
		LWCA_LIST_HEAD *head = 	&node->hash_list;
		//printf("%d|%s %d\n", new_node->layer, new_node->key, t);
		list_add_tail(&new_node->hash_list, head);
	}else{
		//printf("%d|%s %d\n", new_node->layer, new_node->key, t);
		root->lwca_hash[t] = (LWCA_NODE *)new_node;
	}

}

void *utils_serach_hash_get(int layer, char *key, LWCA_RUNING_INFO_NODE *root)
{
	char k[100]={0};
	sprintf(k, "%05d%s", layer-1, key);

	unsigned t = str_hash(k)%1000;
	
	printf("%x\n", root->lwca_hash[t]);
	if( root->lwca_hash[t] ){
		LWCA_LIST_NODE *node=NULL;
		node = (LWCA_LIST_NODE *)root->lwca_hash[t];
		printf("%d | %s | %d\n", node->layer, node->key, t);
		if(layer == node->layer && strcmp(key, node->key) == 0){
			return (void *)node->value;
		}
		
		LWCA_LIST_HEAD *head = 	&node->hash_list;
		LWCA_LIST_NODE *temp_node;
		/*show all list*/
		list_for_each_entry_safe(node, temp_node, head, hash_list)
		{	
			printf("%d|%s %d\n", node->layer, node->key, t);
			if(layer == node->layer && strcmp(key, node->key) == 0){
				return (void *)node->value;
			}
		}
	}
	return NULL;	
	
}
LWCA_LIST_NODE *utils_serach_node_get(char *key, LWCA_LIST_HEAD *head)
{
	if(head == NULL || key == NULL)return NULL;
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(strcmp(key, node->key) == 0){
			return node;
		}
	}
	return NULL;
}
void utils_destroy(LWCA_LIST_HEAD *head)
{
	
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*del all list*/
again:
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->next.next == &node->next){
			list_del(&node->list);
			free(node);
		}else if(node->next.next != &node->next){
			utils_destroy(&node->next);		
		}
	}
	if( node->list.next != &node->list ){
 		goto again;
	}
	
}
/*************************************************
Function: 		Lwca_Destroy
Description:	用户使用销毁root根结点 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:

Others:			无 
*************************************************/
void Lwca_Destroy(LWCA_NODE *root)
{
	if(root == NULL){
		printf("Lwca_Destroy parameter is empty\n");
		return; 
	}

	LWCA_RUNING_INFO_NODE *root2 = ((LWCA_LIST_NODE *)root)->root;

	if(root2){

		if(root2->buffer){
			free(root2->buffer);
		}
		free(root2);	
	}
	
	utils_destroy(&((LWCA_LIST_NODE *)root)->next);
	free(root);
	return; 
}
/*************************************************
Function: 		utils_analysis
Description:	配置内容解析，生成树型结构 
Called By:		Lwca_Init

Input:			@layer:当前所层级
				@root; 配置文件解析信息结构
				@head; 当前结点的父结点的树型结构链表头 
				
Output:			@head;  当前结点添加到链表
				@listNum; 统计list 链表子结点个数 
				
Return:			LWCA_TRUE; 解析成功
				LWCA_FALSE; 解析失败 
Others:			无 
*************************************************/
int utils_analysis(int layer,  LWCA_RUNING_INFO_NODE *root, LWCA_LIST_HEAD *head, int *listNum)
{
	int k=0;
	int cur_align;
	int frist_align;
	int i;
	int indicia=0;
	LWCA_LIST_NODE *new_node;

	while(root->cur_tmp !=NULL){
		
		if(k==0){
			frist_align = utils_find_headspace_num(root->cur_tmp);
			cur_align = frist_align;
			k=1;	
		}else{
			cur_align = utils_find_headspace_num(root->cur_tmp);			
		}

		if(cur_align > frist_align){
		//子结点 
				int res;
				res = utils_analysis(layer+1, root, &new_node->next, &new_node->list_num);
				if(res == LWCA_FALSE)return LWCA_FALSE;
				if(root->cur_tmp == NULL)return LWCA_TRUE;

		}else if(cur_align < frist_align){
		//返回父结点； 
			//当层数不为0时才执行 
			if(layer != 0){
				return  LWCA_TRUE;
			}else{
			//当层数为0,屏蔽这两个参数对比 
				frist_align = cur_align;
			}
		}			


		//发现key头部 
		root->cur_tmp = utils_find_k_head(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//结束处理 
			return LWCA_TRUE;
		}

		root->cur_tmp = utils_find_k_end(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//结束处理 ,有key没值 
			return LWCA_TRUE;
		}

		//发现value头 
		root->cur_tmp = utils_find_v_head(root->cur_tmp, root);
		if(root->cur_tmp == NULL){
			//结束处理 	有key没值 
			return LWCA_TRUE;
		}
		root->cur_tmp = utils_find_v_end(root->cur_tmp, root);
		//添加key value

		//申请添加结点 
		new_node = (LWCA_LIST_NODE *)malloc(sizeof(LWCA_LIST_NODE));
		if(new_node != NULL)
		{
			
			INIT_LIST_HEAD(&new_node->next);
			INIT_LIST_HEAD(&new_node->hash_list);
			
			new_node->next.prev=&new_node->next;
			new_node->next.next=&new_node->next;
			
			new_node->layer=layer;
			new_node->indicia=indicia++;
			new_node->key = root->cur_k_start;
			new_node->value = root->cur_v_start;
			new_node->root = root;
			utils_serach_hash_set(layer, new_node->key, root, new_node);

			new_node->list_num=0;
			list_add_tail(&new_node->list,head);

		}
		
		*listNum = indicia;

		int tlen=0;
		switch(*(new_node->value)){
			case '"'://是串，去引号 	
				tlen = strlen(new_node->value);
				*(new_node->value+tlen-1) = '\0';				
				new_node->value = new_node->value+1;

				break;
			case '{'://是域，再次分析 
				tlen = strlen(new_node->value);
				*(new_node->value+tlen-1) = '\0';				
				new_node->value = new_node->value+1;

				break;
			default:
				break;
		}

	}	

	return  LWCA_TRUE;

	
}
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
LWCA_NODE *Lwca_Init(int mode, char *arg)
{
	LWCA_LIST_NODE *node_root;
	LWCA_RUNING_INFO_NODE *root;
	int res;
	
	node_root = (LWCA_LIST_NODE *)malloc(sizeof(LWCA_LIST_NODE));
	memset(node_root, 0, sizeof(LWCA_LIST_NODE));
	INIT_LIST_HEAD(&node_root->next);
	INIT_LIST_HEAD(&node_root->list);

	if(mode == LWCA_MODE_FILE){
		root = Lwca_F_Init(arg);
		node_root->root = root;
		if(root == NULL){
			printf("lwca init false\n");
			return NULL;
		}
		res = utils_analysis(0, root, &node_root->next, &node_root->list_num);
		if(res == LWCA_FALSE){
			Lwca_Destroy((LWCA_NODE *)node_root);
			return NULL;
		}
	}else if(mode == LWCA_MODE_STR){
		root = Lwca_S_Init(arg);
		node_root->root = root;
		if(root == NULL){
			printf("lwca init false\n");
			return NULL;
		}
		res = utils_analysis(0, root, &node_root->next, &node_root->list_num);
		if(res == LWCA_FALSE){
			Lwca_Destroy((LWCA_NODE *)node_root);
			return NULL;
		}
	}else{
		printf("mode para error\n");
		return NULL;
	}
	
	return (LWCA_NODE *)node_root;
}

/*************************************************
Function: 		Lwca_Get_ListIndex_By_Node
Description:	用户使用root根结点返回当前结点的index 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			index 

Others:			无 
*************************************************/
int Lwca_Get_ListIndex_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return -1;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	return node2->indicia;
}
/*************************************************
Function: 		Lwca_Get_Key_By_Node
Description:	用户使用root根结点返回当前结点的key 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			指向key 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Key_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return NULL;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	
	return (char *)node2->key;
}
/*************************************************
Function: 		Lwca_Get_Value_By_Node
Description:	用户使用root根结点返回当前结点的value 值 
Called By:		

Input:			LWCA_NODE 根结点 指针
				
Output:			
	
Return:			指向value 值的指针 

Others:			无 
*************************************************/
char *Lwca_Get_Value_By_Node(LWCA_NODE *node)
{
	if(node == NULL )return NULL;
	
	LWCA_LIST_NODE *node2=(LWCA_LIST_NODE *)node;
	
	return (char *)node2->value;
}
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
char *Lwca_Get_Value_By_Key(LWCA_NODE *root, char *key)
{
	struct str_split split;
	int i;

	LWCA_LIST_NODE *node=NULL;
	LWCA_LIST_NODE *root2 = ((LWCA_LIST_NODE *)root);
	if(root == NULL){
		printf("Lwca_Get_Value_By_Key %s parameter is empty\n", split.str_array[i]);
		return; 
	}	
	str_split_func(&split, key, ':');	
	LWCA_LIST_HEAD *head=&root2->next;
	for(i=0; i<split.count; ++i)
	{
		node = utils_serach_node_get(split.str_array[i], head);
		if(node){
			head = &node->next;
		}else{
			break;
		}
	}
	
	//打到返回value 
	if( i == split.count ){
		str_split_free(&split);
		return Lwca_Get_Value_By_Node((LWCA_NODE *)node);
	}
	str_split_free(&split);	
	return NULL;
	
}
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
LWCA_NODE *Lwca_Get_Node_By_Key(LWCA_NODE *root, char *key)
{
	struct str_split split;
	int i;
	if(root == NULL || key == NULL){
		printf("Lwca_Get_Node_By_Key %s parameter is empty\n", split.str_array[i]);
		return NULL; 
	}	
	
	LWCA_LIST_NODE *node=NULL;
	LWCA_LIST_NODE *root2 = ((LWCA_LIST_NODE *)root);

	str_split_func(&split, key, ':');	
	LWCA_LIST_HEAD *head=&root2->next;
	for(i=0; i<split.count; ++i)
	{
		node = utils_serach_node_get(split.str_array[i], head);
		if(node){
			head = &node->next;
		}else{
			break;
		}
	}
	
	//打到返回value 
	if( i == split.count ){
		str_split_free(&split);
		return (LWCA_NODE *)node;
	}
	str_split_free(&split);	
	return NULL;

}
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
char *Lwca_Get_Value_By_ListIndex(LWCA_NODE *root, int index)
{
	if(root == NULL )return NULL;
	LWCA_LIST_HEAD *head=&(((LWCA_LIST_NODE *)root)->next);
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->indicia == index){
			return node->value;
		}
	}
	return NULL;
	
}
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
LWCA_NODE *Lwca_Get_Node_By_ListIndex(LWCA_NODE *root, int index)
{
	if(root == NULL )return NULL;
	LWCA_LIST_HEAD *head=&(((LWCA_LIST_NODE *)root)->next);
	LWCA_LIST_NODE *temp_node;
	LWCA_LIST_NODE *node;
	/*show all list*/
	list_for_each_entry_safe(node, temp_node, head, list)
	{	
		if(node->indicia == index){
			return (LWCA_NODE *)node;
		}
	}
	return NULL;
	
}
/*************************************************
Function: 		Lwca_Get_Node_ListNum_By_Node
Description:	//返回当前node节点的list个数  
Called By:		

Input:			@root; LWCA_NODE 根结点 指针
				
Output:			
	
Return:			//返回当前node节点的list个数 

Others:			无 
*************************************************/
int Lwca_Get_Node_ListNum_By_Node(LWCA_NODE *root)
{
	if(root == NULL )return 0;
	return ((LWCA_LIST_NODE *)root)->list_num;
}
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
char *Lwca_Get_Value_By_LayerKey(LWCA_NODE *root, int layer, char *key)
{
	return (char *)utils_serach_hash_get(layer, key, ((LWCA_LIST_NODE *)root)->root);
}
