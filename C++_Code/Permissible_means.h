#pragma once
#include "Transportation.h"

class railway_means
{
public:
	railway_means();
	~railway_means();
	transportation railways[300]; //�ٶ�һ������վһ�췢�����300����
	//int flag;
	int railnumbers;	//һ������վ�������ε�����
};

class plane_means
{
public:
	plane_means();
	~plane_means();
	transportation planes[300];  //�ٶ�һ���ɻ���һ��������300�ܴ�
	//int flag;
	int planenumbers;	//һ���ɻ�����ɵĺ�������
};

class subway_means
{
public:
	subway_means();
	~subway_means();
	transportation subways[20];	//һ������վ��ྭ��20����ͬ��·
	int subnumbers;	//һ������վ�����Ĳ�ͬ��·����
};

