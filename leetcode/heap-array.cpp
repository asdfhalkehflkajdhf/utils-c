


#include <iostream>
#include <string>
#include <set>
#include <vector>

using namespace std;
template<typename T>
void show(T &arry){
	
	for(auto &k : arry){
		std::cout<<" "<<k;
	}
	std::cout<<std::endl;
}

//整理节点time:O(lgn)
template<typename T>
void MinHeapify(T &arry, int size, int element)
{
	int lchild=element*2+1;
	int rchild=lchild+1;//左右子树
	while(rchild<size)//子树均在范围内
	{
		if(arry[element] <= arry[lchild] && arry[element] <= arry[rchild])//如果比左右子树都小，完成整理
		{
			cout<<"end";
			return;
		}
		if(arry[lchild]<=arry[rchild])//如果左边最小
		{
			cout<<" | 双子树 l[index("<<lchild<<" , "<<element<<") value("<<arry[lchild]<<" , "<<arry[element]<<")]";
			swap(arry[element],arry[lchild]);//把左面的提到上面
			element=lchild;//循环时整理子树
		}
		else//否则右面最小
		{
			cout<<" | 双子树 r[index("<<rchild<<" , "<<element<<") value("<<arry[rchild]<<" , "<<arry[element]<<")]";
			swap(arry[element],arry[rchild]);//同理
			element=rchild;
		}
		lchild = element*2+1;
		rchild = lchild+1;//重新计算子树位置
	}
	if(lchild<size && arry[lchild]<arry[element])//只有左子树且子树小于自己
	{
		cout<<" | 单子树 l[index("<<lchild<<" , "<<element<<") value("<<arry[lchild]<<" , "<<arry[element]<<")]";
		swap(arry[lchild], arry[element]);
	}else{
		cout<<" | 无动作";
	}
	return;
}

//堆排序time:O(nlgn)
template<typename T>
void HeapSort(T &arry,int size)
{
	int i;
	cout<<"构造堆（小顶堆，下标小的值也小）"<<endl;
	for(i=size/2-1; i>=0; --i)//从子树开始整理树(从size/2开始是因为堆是完全二叉树，个数/2正好是最后一个可能是单子的点)
	{
		std::cout<<i<<" - ";
		MinHeapify(arry,size,i);
		show<T>(arry);
	}

	cout<<endl<<"调整堆";
	while(size>0)//每次从堆顶取走最值
	{
		cout<<endl<<"取最值 "<<arry[0]<<endl;
		swap(arry[size-1],arry[0]);//将堆的最后一个值，放到堆顶，调整堆，如果是外来数据，则不需要
		size--;//树大小减小
		MinHeapify(arry,size,0);//整理树
	}
	return;
}

//经常面试的问题，1千万个数，取top100。堆可以建一个100 + 1 大小的，每次都从1千万中取一个进行调整堆，每次从堆中删除最小的那个
//n个数据取top k，时间复杂度为 O（n lgk）


int main(int argc, char **argv)
{
	std::vector<int> arry={11,53,64,76,23,32,43,85};
	
	HeapSort<std::vector<int>>(arry, arry.size());
	
	show<std::vector<int>>(arry);
	
	return 0;
}

