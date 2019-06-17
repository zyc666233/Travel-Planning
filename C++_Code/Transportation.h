#pragma once
#include<string>
using namespace std;

class transportation
{
public:
	transportation();
	transportation(string stop_name, double g, double h, transportation *last, string represent_number);
	~transportation();
	string stop_name;	//驶向的目标车站或飞机场名或地铁站名
	string represent_number;	//车次或航班号或地铁线路
	string starting_time;	//在地铁表里表示首班车时间
	string arrival_time;	//在地铁表里表示末班车时间
	string transportation_name;	//到达目标站点的交通方式
	int spendtimes;
	int price;
	int change_trans_times;
	double g, h;
	transportation *last;	//记录上一条交通信息
};


