#pragma once
#include<string>
using namespace std;

class transportation
{
public:
	transportation();
	transportation(string stop_name, double g, double h, transportation *last, string represent_number);
	~transportation();
	string stop_name;	//ʻ���Ŀ�공վ��ɻ����������վ��
	string represent_number;	//���λ򺽰�Ż������·
	string starting_time;	//�ڵ��������ʾ�װ೵ʱ��
	string arrival_time;	//�ڵ��������ʾĩ�೵ʱ��
	string transportation_name;	//����Ŀ��վ��Ľ�ͨ��ʽ
	int spendtimes;
	int price;
	int change_trans_times;
	double g, h;
	transportation *last;	//��¼��һ����ͨ��Ϣ
};


