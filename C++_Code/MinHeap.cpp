#include "MinHeap.h"


MinHeap::MinHeap()
{
}

MinHeap::~MinHeap()
{
}

transportation MinHeap::getAndRemoveMin()
{
	if (queue.empty())
		return transportation();

	transportation head = queue.top();
	queue.pop();
	string str = head.represent_number + "_" + head.stop_name;
	index.erase(str);

	return head;
}

transportation* MinHeap::find(string pnt)
{
	if (index.count(pnt) == 0)
		return nullptr;

	return &index[pnt];
}

void MinHeap::add(transportation data)
{
	queue.push(data);
	string str = data.represent_number + "_" + data.stop_name;
	index[str] = data;
}

bool MinHeap::isEmpty()
{
	return queue.empty();
}