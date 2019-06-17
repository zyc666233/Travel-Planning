#pragma once
#include "Transportation.h"

class railway_means
{
public:
	railway_means();
	~railway_means();
	transportation railways[300]; //假定一个高铁站一天发出最多300车次
	//int flag;
	int railnumbers;	//一个高铁站发出车次的数量
};

class plane_means
{
public:
	plane_means();
	~plane_means();
	transportation planes[300];  //假定一个飞机场一天起飞最多300架次
	//int flag;
	int planenumbers;	//一个飞机场起飞的航班数量
};

class subway_means
{
public:
	subway_means();
	~subway_means();
	transportation subways[20];	//一个地铁站最多经过20条不同线路
	int subnumbers;	//一个地铁站经过的不同线路数量
};

