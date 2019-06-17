#pragma once
#include<map>
#include<queue>
#include<string>
#include"Transportation.h"
using namespace std;

struct cmp	//��Ϊ���ȳ����ж�Ϊ!cmp�����Է�����ʵ����Сֵ����
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
	priority_queue<transportation, vector<transportation>, cmp> queue;	//���ȶ���ģ��С����
	transportation getAndRemoveMin();	//��ȡ��ɾ���Ѷ�ֵ
	transportation* find(string pnt);
	void add(transportation data);
	bool isEmpty();
};

