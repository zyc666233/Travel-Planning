#pragma once
#include<map>
#include<queue>
#include<string>
#include"Transportation.h"
using namespace std;

struct cmp	//因为优先出列判定为!cmp，所以反向定义实现最小值优先
{
	bool operator()(transportation &a, transportation &b) const
	{
		return (a.g + a.h) > (b.g + b.h);
	}
};

class MinHeap
{
public:
	MinHeap();
	~MinHeap();

	map<string, transportation> index;
	priority_queue<transportation, vector<transportation>, cmp> queue;	//优先队列模拟小顶堆
	transportation getAndRemoveMin();	//获取并删除堆顶值
	transportation* find(string pnt);
	void add(transportation data);
	bool isEmpty();
};

