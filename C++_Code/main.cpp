#include "Transportation.h"
#include "Permissible_means.h"
#include "MinHeap.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <math.h>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <mysql.h>
#define PI 3.1415926
using namespace std;

#pragma comment(lib, "libmysql.lib")

bool cmp1(transportation a, transportation b)	//������ʡǮ����ʡʱ����
{
	return a.g < b.g;
}

bool cmp2(transportation a, transportation b)	//��������������
{
	if (a.change_trans_times == b.change_trans_times)
		return a.g < b.g;
	return a.change_trans_times < b.change_trans_times;
}

MYSQL ceshi;
MYSQL_RES *result;	//�����
MYSQL_ROW row;	//�������һ�е���Ϣ
map<string, int> city;	//������ӳ��Ϊ���
vector<string> have_railway_stops[6];	//�ݱ���6������ӵ�еĸ���վ����
vector<string> have_plane_stops[6];	//�ݱ���6������ӵ�еķɻ�������
map<string, vector<string> > railplane_to_sub;	//����վ�����ĵ���վ
bool city_visited[6];
double res_city_lon[6];
double res_city_lat[6];
int allow_trans_numbers;	//��ʱ��������ת����ͨ��ʽ�ĳ�������

map<string, int> railway_stops;	//������վ��ӳ��Ϊ���
map<int, railway_means> railway_list[620];	//�����ڽӱ�洢����վ��ͼ��Ϣ
bool rail_stop_visited[620];
double res_rail_lon[620];	//���ȱ�
double res_rail_lat[620];	//γ�ȱ�
int res_rail_city[620];	//����վ�������б�

map<string, int> plane_stops; //���ɻ�����ӳ��Ϊ���
map<int, plane_means> plane_list[100];	//�����ڽӱ�洢�ɻ�����ͼ��Ϣ
bool plane_stop_visited[100];
double res_plane_lon[100];	//���ȱ�
double res_plane_lat[100];	//γ�ȱ�
int res_plane_city[100];	//�ɻ����������б�

map<string, int> subway_stops[6];	//��6�����еĵ���վ��ӳ��Ϊ���
map<int, subway_means> subway_list[6][400];	//�����ڽӱ�洢6�����е���վ��ͼ��Ϣ
bool subway_stop_visited[6][400];
//double res_subway_lon[6][400];	//���ȱ�
//double res_subway_lat[6][600];	//γ�ȱ�
//int res_subway_city[600];	//����վ�������б�

void create_map();
void create_railway_map();
void create_plane_map();
void create_subway_map();

string search_min_time(string start, string start_city, string end, string end_city, string time, string plan);//�������ŷ���
string search_min_price(string start, string start_city, string end, string end_city, string time, string plan);//������ʡǮ����
transportation search_subway_min_time(transportation* node, string start, string end, string nearname, int time, int cur_city_flag, string end_city);
transportation search_subway_min_price(transportation * node, string start, string end, string nearname, int time, int cur_city_flag, string end_city);

//void search_min_price(string start, string end, string time);	//��ʡǮ����
//void search_railway_min_price(string start, string end, string time);	//����������������ʡǮ����
//void search_plane_min_price(string start, string end, string time);	//�ɻ�������������ʡǮ����

//void search_min_trouble(string start, string end, string time);	//�����ʷ���
//void search_railway_min_trouble(string start, string end, string time);	//�������������������ʷ���
//void search_plane_min_trouble(string start, string end, string time);	//�ɻ����������������ʷ���

vector<string> split(string strs, char ch);	//�ַ����ָ��
string trans_to_str_time(double i_time);
void get_lon_and_lat_and_city();
double cal_h(double lon1, double lat1, double lon2, double lat2, string trans_means);

int main()
{
	create_map();

	string start, start_city, end, end_city, time, plan;
	while (1)
	{
		cout << "\n������ ���վ�������С��յ�վ���յ���С�Ԥ�Ƴ���ʱ��͹滮������" << endl;
		cin >> start >> start_city >> end >> end_city >> time >> plan;
		if (plan == "��ʡǮ")
			search_min_price(start, start_city, end, end_city, time, plan);
		else
			search_min_time(start, start_city, end, end_city, time, plan);
	}

	return 0;
}

void create_map()
{
	mysql_init(&ceshi);   //��ʼ��MYSQL����
	//mysql_options(&ceshi, MYSQL_SET_CHARSET_NAME, "utf8");	//linux��
	mysql_options(&ceshi, MYSQL_SET_CHARSET_NAME, "gbk");	//windows�����ö�ȡʱ��gbk��������ַ�������������ʾ����
	//129.211.3.97 123456
	if (mysql_real_connect(&ceshi, "localhost", "root", "zyc666233", "test", 3306, NULL, 0))  //���ӵ�mysql
		cout << "\n----MySQL������----" << endl;

	create_railway_map();
	create_plane_map();
	get_lon_and_lat_and_city();
	create_subway_map();

	mysql_free_result(result);     //�ͷŽ������ռ�õ��ڴ�
	mysql_close(&ceshi);          //�ر���mysql������
}

void create_railway_map()
{
	int num;	//������Ž��������
	if (!mysql_query(&ceshi, "SELECT * FROM ����"))   //����ѯ�ɹ�����0��ʧ�ܷ��������
		cout << "\n----�������ѯ�ɹ�----" << endl;
	result = mysql_store_result(&ceshi);    //����ѯ���Ľ�������浽result��
	num = mysql_num_fields(result);        //�������������ŵ�num��

	string temp[50];	//�������ݵ�����Ϣ
	int flag = 0;	//��¼��ͬ��վ��ı��
	vector<string> str1;
	vector<string> str2;
	vector<string> str_times;
	int start_time, arrival_time;
	//railway_stops.clear();	//��ո���վ����Ϣ
	//ofstream destFile("����վ����.txt", ios::out); //���ı�ģʽ��out.txt��д
	while ((row = mysql_fetch_row(result)))  //�������һ�У�����ֹѭ��
	{
		for (int i = 0; i < num; i++)         //������е�ÿһ��
			temp[i] = row[i];	//row��MYSQL_ROW���������Ե�������ʹ�ã�iΪ����

		str1 = split(temp[1], '_');
		if (railway_stops.insert(map<string, int>::value_type(str1[0], flag)).second == true)
		{
			//find_railway_stops[flag] = str1[0];
			//cout << str1[0] << " " << flag << endl;
			//destFile << flag << "," << str1[0] << "վ" << endl;
			flag++;
		}
		for (int j = 2; j < num; j++)
		{
			if (temp[j] == "")	break;
			str1 = split(temp[j - 1], '_');	//name_lastarrivaltime_nextstarttime_stoptime
			str2 = split(temp[j], '_');
			if (railway_stops.insert(map<string, int>::value_type(str2[0], flag)).second == true)	//��ֹͬһվ���ظ�ӳ��
			{
				//find_railway_stops[flag] = str2[0];
				//cout << str2[0] << " " << flag << endl;
				//destFile << flag << "," << str2[0] << "վ" << endl;
				flag++;
			}
			//str1[0]Ϊ����վ��str2[0]ΪĿ��վ
			int flag1 = railway_stops[str1[0]];
			int flag2 = railway_stops[str2[0]];
			if (railway_list[flag1].count(flag2) == 0)
			{
				railway_means res;
				//res.flag = flag2;
				res.railways[res.railnumbers].represent_number = temp[0];	//����
				res.railways[res.railnumbers].starting_time = str1[2];
				res.railways[res.railnumbers].arrival_time = str2[1];
				res.railways[res.railnumbers].transportation_name = "����";
				str_times = split(str1[2], ':');
				start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				str_times = split(str2[1], ':');
				arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arrival_time < start_time)	//˵�������������
				{
					res.railways[res.railnumbers].spendtimes = 1440 - start_time + arrival_time;
				}
				else //����ʱ��
				{
					res.railways[res.railnumbers].spendtimes = arrival_time - start_time;
				}

				if (temp[0][0] == 'G')	//����Ʊ��
					res.railways[res.railnumbers].price = (int)(1.5 * res.railways[res.railnumbers].spendtimes);
				else if (temp[0][0] == 'C')	//�Ǽ�Ʊ��
					res.railways[res.railnumbers].price = (int)(1.75 * res.railways[res.railnumbers].spendtimes);
				else //����Ʊ��
					res.railways[res.railnumbers].price = (int)(0.9 * res.railways[res.railnumbers].spendtimes);

				res.railways[res.railnumbers].stop_name = str2[0];
				res.railnumbers++;
				railway_list[flag1].insert(map<int, railway_means>::value_type(flag2, res));
				//cout << times;
			}
			else
			{
				int railnumbers = railway_list[flag1][flag2].railnumbers;
				railway_list[flag1][flag2].railways[railnumbers].represent_number = temp[0];
				railway_list[flag1][flag2].railways[railnumbers].starting_time = str1[2];
				railway_list[flag1][flag2].railways[railnumbers].arrival_time = str2[1];
				railway_list[flag1][flag2].railways[railnumbers].transportation_name = "����";
				str_times = split(str1[2], ':');
				start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				str_times = split(str2[1], ':');
				arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arrival_time < start_time)	//˵�������������
				{
					railway_list[flag1][flag2].railways[railnumbers].spendtimes = 1440 - start_time + arrival_time;
				}
				else //����ʱ��
				{
					railway_list[flag1][flag2].railways[railnumbers].spendtimes = arrival_time - start_time;
				}

				if (temp[0][0] == 'G')	//����Ʊ��
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(1.5 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);
				else if (temp[0][0] == 'C')	//�Ǽ�Ʊ��
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(1.75 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);
				else  //����Ʊ��
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(0.9 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);

				railway_list[flag1][flag2].railways[railnumbers].stop_name = str2[0];
				railway_list[flag1][flag2].railnumbers++;
			}
		}
	}
	//cout << flag - 1;
	//destFile.close();
	cout << "\n���������ɹ�" << endl;
}

void create_plane_map()
{
	int num;	//������Ž��������
	if (!mysql_query(&ceshi, "SELECT * FROM �ɻ�"))   //����ѯ�ɹ�����0��ʧ�ܷ��������
		cout << "\n----�ɻ����ѯ�ɹ�----" << endl;
	result = mysql_store_result(&ceshi);    //����ѯ���Ľ�������浽result��
	num = mysql_num_fields(result);        //�������������ŵ�num��

	string temp[10];	//�������ݵ�����Ϣ
	int flag = 0;	//��¼��ͬ��վ��ı��
	vector<string> str1;
	vector<string> str2;
	vector<string> str_times;
	int start_time, arrival_time;
	//stops.clear();
	//ofstream destFile("��������.txt", ios::out); //���ı�ģʽ�򿪱�д
	while ((row = mysql_fetch_row(result)))  //�������һ�У�����ֹѭ��
	{
		for (int i = 0; i < num; i++)         //������е�ÿһ��
			temp[i] = row[i];	//row��MYSQL_ROW���������Ե�������ʹ�ã�iΪ����
		if (plane_stops.insert(map<string, int>::value_type(temp[1], flag)).second == true)	//��ֹͬһվ���ظ�ӳ��
		{
			//cout << temp[1] << " " << flag << endl;
			//destFile << flag << "," << temp[1] << endl;
			flag++;
		}
		if (plane_stops.insert(map<string, int>::value_type(temp[3], flag)).second == true)	//��ֹͬһվ���ظ�ӳ��
		{
			//cout << temp[3] << " " << flag << endl;
			//destFile << flag << "," << temp[3] << endl;
			flag++;
		}
		//temp[1]Ϊ����վ��temp[3]ΪĿ��վ
		int flag1 = plane_stops[temp[1]];
		int flag2 = plane_stops[temp[3]];
		if (plane_list[flag1].count(flag2) == 0)
		{
			plane_means res;
			//res.flag = flag2;
			res.planes[res.planenumbers].represent_number = temp[0];
			res.planes[res.planenumbers].starting_time = temp[2];
			res.planes[res.planenumbers].stop_name = temp[3];
			res.planes[res.planenumbers].arrival_time = temp[4];
			res.planes[res.planenumbers].transportation_name = "�ɻ�";
			str_times = split(temp[2], ':');
			start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			str_times = split(temp[4], ':');
			arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arrival_time < start_time)	//˵�������������
			{
				res.planes[res.planenumbers].spendtimes = 1440 - start_time + arrival_time;
			}
			else //����ʱ��
			{
				res.planes[res.planenumbers].spendtimes = arrival_time - start_time;
			}
			res.planes[res.planenumbers].price = atoi(temp[5].c_str());	//¼��Ʊ��
			res.planenumbers++;
			plane_list[flag1].insert(map<int, plane_means>::value_type(flag2, res));
		}
		else
		{
			int planenumbers = plane_list[flag1][flag2].planenumbers;
			plane_list[flag1][flag2].planes[planenumbers].represent_number = temp[0];
			plane_list[flag1][flag2].planes[planenumbers].starting_time = temp[2];
			plane_list[flag1][flag2].planes[planenumbers].stop_name = temp[3];
			plane_list[flag1][flag2].planes[planenumbers].arrival_time = temp[4];
			plane_list[flag1][flag2].planes[planenumbers].transportation_name = "�ɻ�";
			str_times = split(temp[2], ':');
			start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			str_times = split(temp[4], ':');
			arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arrival_time < start_time)	//˵�������������
			{
				plane_list[flag1][flag2].planes[planenumbers].spendtimes = 1440 - start_time + arrival_time;
			}
			else //����ʱ��
			{
				plane_list[flag1][flag2].planes[planenumbers].spendtimes = arrival_time - start_time;
			}
			plane_list[flag1][flag2].planes[planenumbers].price = atoi(temp[5].c_str());
			plane_list[flag1][flag2].planenumbers++;
		}
		//cout << temp[0] << endl;
	}
	//destFile.close();
	cout << "\n�ɻ������ɹ�" << endl;
}

void create_subway_map()
{
	int num;	//������Ž��������
	map<string, int>::iterator it;	//���ڱ���city
	int allow_city;	//Ŀǰ�����6�����еĵ�����Ϣ
	for (it = city.begin(), allow_city = 0; it != city.end() && allow_city < allow_trans_numbers; it++)
	{
		int city_flag = it->second;//���б��
		if (city_flag < allow_trans_numbers)
		{
			allow_city++;
			string str = "SELECT * FROM " + it->first;
			if (!mysql_query(&ceshi, str.c_str()))   //����ѯ�ɹ�����0��ʧ�ܷ��������
				cout << "\n----" + it->first + "�������ѯ�ɹ�----" << endl;
			result = mysql_store_result(&ceshi);    //����ѯ���Ľ�������浽result��
			num = mysql_num_fields(result);        //�������������ŵ�num��

			string temp[50];	//�������ݵ�����Ϣ
			int flag = 0;	//��¼��ͬ��վ��ı��
			vector<string> str1;
			vector<string> str2;
			vector<string> str3;
			vector<string> str_times;
			int start_time, arrival_time;
			//railway_stops.clear();	//��յ���վ����Ϣ
			//string str = it->first + "����վ����.txt";
			//ofstream destFile(str.c_str(), ios::out); //���ı�ģʽ��out.txt��д
			while ((row = mysql_fetch_row(result)))  //�������һ�У�����ֹѭ��
			{
				for (int i = 0; i < num; i++)         //������е�ÿһ��
					temp[i] = row[i];	//row��MYSQL_ROW���������Ե�������ʹ�ã�iΪ����

				str1 = split(temp[1], '_');
				if (str1[0].find('*') != string::npos)	//˵���õ���վ��������վ
				{
					str3 = split(str1[0], '*');
					for (int i = 1; i < str3.size(); i++)
					{
						vector<string>::iterator ite = find(railplane_to_sub[str3[i]].begin(), railplane_to_sub[str3[i]].end(), str3[0]);
						if (ite == railplane_to_sub[str3[i]].end()) //�õ���վδ¼��
							railplane_to_sub[str3[i]].push_back(str3[0]);
					}
					str1[0] = str3[0];
				}
				if (subway_stops[city_flag].insert(map<string, int>::value_type(str1[0], flag)).second == true)
				{
					//find_railway_stops[flag] = str1[0];
					//cout << str1[0] << " " << flag << endl;
					//destFile << flag << "," << str1[0] << "վ" << endl;
					flag++;
				}
				for (int j = 2; j < num; j++)
				{
					if (temp[j] == "")	break;

					str1 = split(temp[j - 1], '_');	//name_lastarrivaltime_nextstarttime_stoptime
					if (str1[0].find('*') != string::npos)	//˵���õ���վ��������վ
					{
						str3 = split(str1[0], '*');
						str1[0] = str3[0];
					}
					str2 = split(temp[j], '_');
					if (str2[0].find('*') != string::npos)	//˵���õ���վ��������վ
					{
						str3 = split(str2[0], '*');
						for (int i = 1; i < str3.size(); i++)
						{
							vector<string>::iterator ite = find(railplane_to_sub[str3[i]].begin(), railplane_to_sub[str3[i]].end(), str3[0]);
							if (ite == railplane_to_sub[str3[i]].end())
								railplane_to_sub[str3[i]].push_back(str3[0]);
						}
						str2[0] = str3[0];
					}
					if (subway_stops[city_flag].insert(map<string, int>::value_type(str2[0], flag)).second == true)	//��ֹͬһվ���ظ�ӳ��
					{
						//find_railway_stops[flag] = str2[0];
						//cout << str2[0] << " " << flag << endl;
						//destFile << flag << "," << str2[0] << "վ" << endl;
						flag++;
					}
					//str1[0]Ϊ����վ��str2[0]ΪĿ��վ
					int flag1 = subway_stops[city_flag][str1[0]];
					int flag2 = subway_stops[city_flag][str2[0]];
					if (subway_list[city_flag][flag1].count(flag2) == 0)
					{
						subway_means res;
						//res.flag = flag2;
						res.subways[res.subnumbers].represent_number = temp[0];	//������·
						res.subways[res.subnumbers].starting_time = str1[1]; //��೵����ʱ��
						res.subways[res.subnumbers].arrival_time = str1[2];	//ĩ�೵����ʱ��
						res.subways[res.subnumbers].transportation_name = "����";
						str_times = split(str1[1], ':');
						start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						str_times = split(str2[1], ':');
						arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						if (arrival_time < start_time) //˵����һվ�����׷�ʱ������ǰһվ
							res.subways[res.subnumbers].spendtimes = 3; //���� 3min ����ʱ��
						else
							res.subways[res.subnumbers].spendtimes = arrival_time - start_time;
						res.subways[res.subnumbers].stop_name = str2[0];
						res.subways[res.subnumbers].price = 0; //����������վ��Ĭ�ϲ���Ǯ
						res.subnumbers++;
						subway_list[city_flag][flag1].insert(map<int, subway_means>::value_type(flag2, res));
						//cout << times;
					}
					else
					{
						int subnumbers = subway_list[city_flag][flag1][flag2].subnumbers;
						subway_list[city_flag][flag1][flag2].subways[subnumbers].represent_number = temp[0];
						subway_list[city_flag][flag1][flag2].subways[subnumbers].starting_time = str1[1];
						subway_list[city_flag][flag1][flag2].subways[subnumbers].arrival_time = str2[1];
						subway_list[city_flag][flag1][flag2].subways[subnumbers].transportation_name = "����";
						str_times = split(str1[1], ':');
						start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						str_times = split(str2[1], ':');
						arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						if (arrival_time < start_time) //˵����һվ�����׷�ʱ������ǰһվ
							subway_list[city_flag][flag1][flag2].subways[subnumbers].spendtimes = 3;  //���� 3min ����ʱ��
						else
							subway_list[city_flag][flag1][flag2].subways[subnumbers].spendtimes = arrival_time - start_time;
						subway_list[city_flag][flag1][flag2].subways[subnumbers].stop_name = str2[0];
						subway_list[city_flag][flag1][flag2].subways[subnumbers].price = 0;
						subway_list[city_flag][flag1][flag2].subnumbers++;
					}
				}
				//cout << temp[0] << endl;
			}
			//cout << flag - 1;
			//destFile.close();
		}
	}
	cout << "\n���������ɹ�" << endl;
}

string search_min_time(string start, string start_city, string end, string end_city, string time, string plan)
{
	memset(rail_stop_visited, false, sizeof(rail_stop_visited));
	memset(city_visited, false, sizeof(city_visited));
	memset(plane_stop_visited, false, sizeof(plane_stop_visited));
	memset(subway_stop_visited, false, sizeof(subway_stop_visited));
	vector<transportation> laststops; //��Ž��
	MinHeap heap = MinHeap();
	transportation *res;	//�ڵ�ָ��
	transportation laststop;	//������һ��վ����Ϣ�����ڷ������·��
	//map<int, railway_means>::iterator it;	//���ڱ���railway_list
	vector<string> str_times;
	str_times = split(time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��Ԥ�Ƴ���ʱ�任�������
	/*��ȡ����վ��end���ڳ��еľ�γ��lon_end, lat_end*/
	double lon_end = res_city_lon[city[end_city]];
	double lat_end = res_city_lat[city[end_city]];
	res = new transportation();

	int city_flag = city[start_city];
	vector<string> subname;
	for (vector<string>::iterator it = have_railway_stops[city_flag].begin(); it != have_railway_stops[city_flag].end(); it++)
	{
		/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[railway_stops[*it]];
		double lat_cur = res_rail_lat[railway_stops[*it]];
		double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it]; //�͸���վ����ĵ���վ�б�
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_time(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 10; //����ӵ���վ���е�����վҪ 10min
			cur_res.price = 0; //���в���Ǯ
			cur_res.g = temp.g + cur_res.spendtimes;
			cur_res.h = h;
			cur_res.transportation_name = "����";
			cur_res.represent_number = "����";
			cur_res.change_trans_times = res->change_trans_times; //����ͨ��ʽ
			cur_res.starting_time = trans_to_str_time(temp.g + r_time);
			cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}

	for (vector<string>::iterator it = have_plane_stops[city_flag].begin(); it != have_plane_stops[city_flag].end(); it++)
	{
		/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[plane_stops[*it]];
		double lat_cur = res_rail_lat[plane_stops[*it]];
		double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it];	//�ɻ�������ĵ���վ���б�
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_time(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 30; //����ӵ���վ���е�����վҪ 30min
			cur_res.price = 0; //���в���Ǯ
			cur_res.g = temp.g + cur_res.spendtimes;
			cur_res.h = h;
			cur_res.transportation_name = "����";
			cur_res.represent_number = "����";
			cur_res.change_trans_times = res->change_trans_times; //����ͨ��ʽ
			cur_res.starting_time = trans_to_str_time(temp.g + r_time);
			cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}
	city_visited[city_flag] = true; //��ʼ�����ѷ���

	/*A*�����㷨ִ��*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}

	int result_flag = 0; //��¼�������Ŀ���·�����������������ʼ���
	while (!finish && !heap.isEmpty() && result_flag < 2)
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//ȡ��fֵ��С�ĵ�
		if (railway_stops.count(res->stop_name))	//�����������
		{
			//�������վ�����ڳ��У��˳�����
			if (res_rail_city[railway_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //˵���ڸ�վ�㻻�˹���
					{
						rail_stop_visited[railway_stops[pathData->last->stop_name]] = false;
						rail_stop_visited[railway_stops[pathData->stop_name]] = true;
						break;
					}
					pathData = pathData->last;
				}
				//rail_stop_visited[railway_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
				laststop = search_subway_min_time(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				continue;
				//break;
			}
			rail_stop_visited[railway_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
			string arr_chici = res->represent_number;	//���浽�ﵱǰ��վ�ĳ���

			str_times = split(res->arrival_time, ':');
			//int arr_time = (int)res->g + 60 * atoi(time.c_str()); //�������վ��ʱ��
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//����ó��ε��ﵱǰ��վ��ʱ��
			int cur_stop = railway_stops[res->stop_name];	//��ǰվ����
			//�����ӵ�ǰ��վ�����ܹ����������վ��
			for (map<int, railway_means>::iterator it = railway_list[cur_stop].begin(); it != railway_list[cur_stop].end(); it++)
			{
				/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
				double lon_cur = res_rail_lon[it->first];
				double lat_cur = res_rail_lat[it->first];
				double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

				transportation temp;
				if (rail_stop_visited[it->first] == true)	//��Ŀ��վ���ѱ����ʹ�
					continue;

				int num = it->second.railnumbers;	//�ܹ�����Ŀ��վ��ĸ�������
				for (int i = 0; i < num; i++)
				{
					temp = it->second.railways[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//�ӵ�վ���ٴη��������ʱ��

					if (leave_time < arr_time)	//������ֿ�������
					{
						wait_time = 1440 - arr_time + leave_time;
					}
					else //��ȡ�³��������ĳ���ʱ��͵�վʱ��Ĳ�ֵ
					{
						wait_time = leave_time - arr_time;
					}

					if (temp.represent_number != arr_chici) //�����Ҫ���˳��λ�ͨ��ʽ
					{
						if (wait_time < 15) //���ǻ��˵�ʵ���������Ҫ15min����ʱ��
							continue;
						temp.change_trans_times += 1; //������
					}
					//if(wait_time > )
					temp.g = temp.spendtimes + wait_time + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
					//��ѯ�Ƿ��Ѿ��ڶ���
					if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else
					{
						temp.h = h;
						temp.last = res;
						temp.transportation_name = "����";
						heap.add(temp);
					}
				}
			}
			//���Ǵ���һ�������Ŀ��վ�㻻�ɻ������
			int cur_city_flag = res_rail_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //˵����Ŀ�ĸ���վ���ڳ����е�����Ϣ
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_plane_stops[cur_city_flag].begin(); j != have_plane_stops[cur_city_flag].end(); j++)
					{
						/*��ȡj�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
						double lon_cur = res_plane_lon[plane_stops[*j]];
						double lat_cur = res_plane_lat[plane_stops[*j]];
						double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
						if (railplane_to_sub.count(*j))
							continue;
						subname = railplane_to_sub[*j];
						for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
						{
							memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
							transportation temp = search_subway_min_time(res, *i, *t, *j, r_time, cur_city_flag, end_city);
							res = new transportation();
							*res = temp;
							transportation cur_res;
							cur_res.stop_name = *j;
							cur_res.spendtimes = 30; //����ӵ���վ���е�����վҪ 30min
							cur_res.price = 0; //���в���Ǯ
							cur_res.g = temp.g + cur_res.spendtimes;
							cur_res.h = h;
							cur_res.transportation_name = "����";
							cur_res.represent_number = "����";
							cur_res.change_trans_times = res->change_trans_times + 1; //����ͨ��ʽ
							cur_res.starting_time = trans_to_str_time(temp.g + r_time);
							cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
							cur_res.last = res;
							heap.add(cur_res);
						}
					}
				}
			}
		}

		if (plane_stops.count(res->stop_name))	//����ɻ�����
		{
			//�������վ�����ڳ��У��˳�����
			if (res_plane_city[plane_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //˵���ڸ�վ�㻻�˹���
					{
						plane_stop_visited[plane_stops[pathData->last->stop_name]] = false;
						break;
					}
					pathData = pathData->last;
				}
				//plane_stop_visited[plane_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
				laststop = search_subway_min_time(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				//break;
				continue;
			}
			plane_stop_visited[plane_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
			string arr_flight_number = res->represent_number;	//���浽�ﵱǰ�����ĺ����
			str_times = split(res->arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//����ð�ηɻ����ﵱǰ������ʱ��
			int cur_stop = plane_stops[res->stop_name];
			//�����ӵ�ǰ���������ܹ���������л���
			for (map<int, plane_means>::iterator it = plane_list[cur_stop].begin(); it != plane_list[cur_stop].end(); it++)
			{
				/*��ȡit�����Ŀ�Ļ����ľ�γ��lon_cur,lat_cur*/
				double lon_cur = res_plane_lon[it->first];
				double lat_cur = res_plane_lat[it->first];
				double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

				transportation temp;
				if (plane_stop_visited[it->first] == true)	//��Ŀ�Ļ����ѱ����ʹ�
					continue;

				int num = it->second.planenumbers;	//�ܹ�����Ŀ�Ļ����ķɻ�����
				for (int i = 0; i < num; i++)
				{
					temp = it->second.planes[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//�ӵ�վ���ٴ���ɼ����ʱ��

					if (leave_time < arr_time)	//������ֿ�������
					{
						wait_time = 1440 - arr_time + leave_time;
					}
					else //��ȡ�³��������ĳ���ʱ��͵�վʱ��Ĳ�ֵ
					{
						wait_time = leave_time - arr_time;
					}

					if (temp.represent_number != arr_flight_number) //�����Ҫ����
					{
						if (wait_time < 30) //���ǻ��˵�ʵ���������Ҫ30min����ʱ��
							continue;
						temp.change_trans_times += 1; //������
					}

					temp.g = temp.spendtimes + wait_time + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//��ѯ�Ƿ��Ѿ��ڶ���
					if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //���ڶ��У������
					{
						temp.h = h;
						temp.last = res;
						heap.add(temp);
					}
				}

			}
			//���Ǵ���һ�������Ŀ��վ�㻻���������
			int cur_city_flag = res_plane_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //˵����Ŀ�ĵطɻ������ڳ����е�����Ϣ
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_railway_stops[cur_city_flag].begin(); j != have_railway_stops[cur_city_flag].end(); j++)
					{
						if (railplane_to_sub.count(*j))
							continue;
						/*��ȡj�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
						double lon_cur = res_rail_lon[railway_stops[*j]];
						double lat_cur = res_rail_lat[railway_stops[*j]];
						double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
						subname = railplane_to_sub[*j];
						for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
						{
							memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
							transportation temp = search_subway_min_time(res, *i, *t, *j, r_time, cur_city_flag, end_city);
							res = new transportation();
							*res = temp;
							transportation cur_res;
							cur_res.stop_name = *j;
							cur_res.spendtimes = 10; //����ӵ���վ���е�����վҪ 10min
							cur_res.price = 0; //���в���Ǯ
							cur_res.g = temp.g + cur_res.spendtimes;
							cur_res.h = h;
							cur_res.transportation_name = "����";
							cur_res.represent_number = "����";
							cur_res.change_trans_times = res->change_trans_times + 1; //����ͨ��ʽ
							cur_res.starting_time = trans_to_str_time(temp.g + r_time);
							cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
							cur_res.last = res;
							heap.add(cur_res);
						}
					}
				}
			}
		}
	}


	if (fin_flag == -1)
	{
		cout << "��ѡ��ĳ���վ������Ԥ�Ƴ���ʱ��θ���û�г��Σ��볢��ѡ������ʱ���" << endl;
	}
	else if (fin_flag == 0)
	{
		cout << "��Ǹ��δ�ܲ�ѯ�����ʵĹ滮�������볢�Բ�ͬʱ���վ�㲢����" << endl;
	}
	else
	{
		string result;
		if (plan == "��ʡʱ")
		{
			sort(laststops.begin(), laststops.end(), cmp1);
			ofstream destFile("��ʡʱ·���滮.txt", ios::out);
			transportation *pathData = &laststops[0];	//��ӡ·��
			transportation res_route, res_next_route;
			double real_spend_time = pathData->g;
			int change_times = pathData->change_trans_times;
			stack<transportation> route;
			int fl = 0;
			string cur_represent_number = "";
			for (pathData = &laststops[0]; pathData->last != nullptr; )
			{
				route.push(*pathData);
				pathData = pathData->last;
			}

			res_route = route.top();
			destFile << res_route.transportation_name << endl;	result = result + res_route.transportation_name + "*";
			destFile << res_route.starting_time << endl;	result = result + res_route.starting_time + "*";
			destFile << res_route.represent_number << endl;	result = result + res_route.represent_number + "*";
			destFile << res_route.last->stop_name << endl;	result = result + res_route.last->stop_name + "*";
			cout << endl << res_route.last->stop_name << "����" << res_route.starting_time << "_" << res_route.transportation_name
				<< "_" << res_route.represent_number << "_";
			while (!route.empty())
			{
				res_route = route.top();
				route.pop();
				if (route.empty())	break;
				res_next_route = route.top();
				if (res_route.represent_number != res_next_route.represent_number)
				{
					destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
					destFile << res_route.arrival_time << endl;	result = result + res_route.arrival_time + "_";
					cout << res_route.arrival_time << "����>" << res_route.stop_name << endl;

					destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
					destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
					destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
					destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
					cout << res_next_route.last->stop_name << "����" << res_next_route.starting_time << "_" << res_next_route.transportation_name
						<< "_" << res_next_route.represent_number << "_";
				}
			}
			destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
			destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
			cout << res_route.arrival_time << "����>" << res_route.stop_name << endl << endl;
			destFile << "����" << endl << "����" << endl << "����" << endl << "����" << endl;
			result = result + "����*����*����*����*";
			destFile << "�ܻ���ʱ�䣺" << trans_to_str_time(real_spend_time) << endl;
			result = result + "�ܻ���ʱ�䣺" + trans_to_str_time(real_spend_time) + "*";
			destFile << "ת��ת���ܴ�����" << change_times;
			result = result + "ת��ת���ܴ�����" + to_string(change_times) + "_";
			cout << "�г���ʱ����" << trans_to_str_time(real_spend_time) << endl;
			cout << "ת���ܴ�����" << change_times << endl;
			destFile.close();
			cout << "�����滮�ɹ�" << endl;

		}
		if (plan == "������")
		{
			sort(laststops.begin(), laststops.end(), cmp2);
			ofstream destFile("������·���滮.txt", ios::out);
			transportation *pathData = &laststops[0];	//��ӡ·��
			transportation res_route, res_next_route;
			double real_spend_time = pathData->g;
			int change_times = pathData->change_trans_times;
			stack<transportation> route;
			int fl = 0;
			string cur_represent_number = "";
			for (pathData = &laststops[0]; pathData->last != nullptr; )
			{
				route.push(*pathData);
				pathData = pathData->last;
			}

			res_route = route.top();
			destFile << res_route.transportation_name << endl;	result = result + res_route.transportation_name + "*";
			destFile << res_route.starting_time << endl;	result = result + res_route.starting_time + "*";
			destFile << res_route.represent_number << endl;	result = result + res_route.represent_number + "*";
			destFile << res_route.last->stop_name << endl;	result = result + res_route.last->stop_name + "*";
			cout << endl << res_route.last->stop_name << "����" << res_route.starting_time << "_" << res_route.transportation_name
				<< "_" << res_route.represent_number << "_";
			while (!route.empty())
			{
				res_route = route.top();
				route.pop();
				if (route.empty())	break;
				res_next_route = route.top();
				if (res_route.represent_number != res_next_route.represent_number)
				{
					destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
					destFile << res_route.arrival_time << endl;	result = result + res_route.arrival_time + "_";
					cout << res_route.arrival_time << "����>" << res_route.stop_name << endl;

					destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
					destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
					destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
					destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
					cout << res_next_route.last->stop_name << "����" << res_next_route.starting_time << "_" << res_next_route.transportation_name
						<< "_" << res_next_route.represent_number << "_";
				}
			}
			destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
			destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
			cout << res_route.arrival_time << "����>" << res_route.stop_name << endl << endl;
			destFile << "����" << endl << "����" << endl << "����" << endl << "����" << endl;
			result = result + "����*����*����*����*";
			destFile << "�ܻ���ʱ�䣺" << trans_to_str_time(real_spend_time) << endl;
			result = result + "�ܻ���ʱ�䣺" + trans_to_str_time(real_spend_time) + "*";
			destFile << "ת��ת���ܴ�����" << change_times;
			result = result + "ת��ת���ܴ�����" + to_string(change_times) + "_";
			cout << "�г���ʱ����" << trans_to_str_time(real_spend_time) << endl;
			cout << "ת���ܴ�����" << change_times << endl;
			destFile.close();
			cout << "�����滮�ɹ�" << endl;
		}

		return result;
	}
}

string search_min_price(string start, string start_city, string end, string end_city, string time, string plan)
{
	memset(rail_stop_visited, false, sizeof(rail_stop_visited));
	memset(city_visited, false, sizeof(city_visited));
	memset(plane_stop_visited, false, sizeof(plane_stop_visited));
	memset(subway_stop_visited, false, sizeof(subway_stop_visited));
	vector<transportation> laststops; //��Ž��
	MinHeap heap = MinHeap();
	transportation *res;	//�ڵ�ָ��
	transportation laststop;	//������һ��վ����Ϣ�����ڷ������·��
	//map<int, railway_means>::iterator it;	//���ڱ���railway_list
	vector<string> str_times;
	str_times = split(time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��Ԥ�Ƴ���ʱ�任�������
	/*��ȡ����վ��end���ڳ��еľ�γ��lon_end, lat_end*/
	double lon_end = res_city_lon[city[end_city]];
	double lat_end = res_city_lat[city[end_city]];
	res = new transportation();

	int city_flag = city[start_city];
	vector<string> subname;
	for (vector<string>::iterator it = have_railway_stops[city_flag].begin(); it != have_railway_stops[city_flag].end(); it++)
	{
		/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[railway_stops[*it]];
		double lat_cur = res_rail_lat[railway_stops[*it]];
		double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway"); //Ԥ�����������Ʊ��
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it]; //�͸���վ����ĵ���վ�б�
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_price(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 10; //����ӵ���վ���е�����վҪ 10min
			cur_res.price = 0; //���в���Ǯ
			cur_res.g = temp.g + cur_res.price; //����gֵ
			cur_res.h = h;
			cur_res.transportation_name = "����";
			cur_res.represent_number = "����";
			cur_res.change_trans_times = res->change_trans_times; //����ͨ��ʽ
			cur_res.starting_time = res->arrival_time;
			str_times = split(cur_res.starting_time, ':');
			cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}

	for (vector<string>::iterator it = have_plane_stops[city_flag].begin(); it != have_plane_stops[city_flag].end(); it++)
	{
		/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[plane_stops[*it]];
		double lat_cur = res_rail_lat[plane_stops[*it]];
		double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it];	//�ɻ�������ĵ���վ���б�
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_price(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 30; //����ӵ���վ���е�����վҪ 30min
			cur_res.price = 0; //���в���Ǯ
			cur_res.g = temp.g + cur_res.price;
			cur_res.h = h;
			cur_res.transportation_name = "����";
			cur_res.represent_number = "����";
			cur_res.change_trans_times = res->change_trans_times; //����ͨ��ʽ
			cur_res.starting_time = res->arrival_time;
			str_times = split(cur_res.starting_time, ':');
			cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}
	city_visited[city_flag] = true; //��ʼ�����ѷ���

	/*A*�����㷨ִ��*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}

	int result_flag = 0; //��¼�������Ŀ���·�����������������ʼ���
	while (!finish && !heap.isEmpty() && result_flag < 4)
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//ȡ��fֵ��С�ĵ�
		if (railway_stops.count(res->stop_name))	//�����������
		{
			//�������վ�����ڳ��У��˳�����
			if (res_rail_city[railway_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //˵���ڸ�վ�㻻�˹���
					{
						rail_stop_visited[railway_stops[pathData->last->stop_name]] = false;
						rail_stop_visited[railway_stops[pathData->stop_name]] = true;
						break;
					}
					pathData = pathData->last;
				}
				//rail_stop_visited[railway_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
				laststop = search_subway_min_price(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				continue;
				//break;
			}
			rail_stop_visited[railway_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
			string arr_chici = res->represent_number;	//���浽�ﵱǰ��վ�ĳ���

			str_times = split(res->arrival_time, ':');
			//int arr_time = (int)res->g + 60 * atoi(time.c_str()); //�������վ��ʱ��
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//����ó��ε��ﵱǰ��վ��ʱ��
			int cur_stop = railway_stops[res->stop_name];	//��ǰվ����
			//�����ӵ�ǰ��վ�����ܹ����������վ��
			for (map<int, railway_means>::iterator it = railway_list[cur_stop].begin(); it != railway_list[cur_stop].end(); it++)
			{
				/*��ȡit�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
				double lon_cur = res_rail_lon[it->first];
				double lat_cur = res_rail_lat[it->first];
				double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

				transportation temp;
				if (rail_stop_visited[it->first] == true)	//��Ŀ��վ���ѱ����ʹ�
					continue;

				int num = it->second.railnumbers;	//�ܹ�����Ŀ��վ��ĸ�������
				for (int i = 0; i < num; i++)
				{
					temp = it->second.railways[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//�ӵ�վ���ٴη��������ʱ��

					if (leave_time < arr_time)	//����ڵ���֮ǰ�ͷ����Ļ���������
					{
						continue;
					}

					wait_time = leave_time - arr_time;

					if (temp.represent_number != arr_chici) //�����Ҫ���˳��λ�ͨ��ʽ
					{
						if (wait_time < 15) //���ǻ��˵�ʵ���������Ҫ15min����ʱ��
							continue;
						temp.change_trans_times += 1; //������
					}
					//if(wait_time > )
					temp.g = temp.price + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
					//��ѯ�Ƿ��Ѿ��ڶ���
					if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //���ڶ��У������
					{
						temp.h = h;
						temp.last = res;
						temp.transportation_name = "����";
						heap.add(temp);
					}
				}
			}
			//���Ǵ���һ�������Ŀ��վ�㻻�ɻ������
			int cur_city_flag = res_rail_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //˵����Ŀ�ĸ���վ���ڳ����е�����Ϣ
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_plane_stops[cur_city_flag].begin(); j != have_plane_stops[cur_city_flag].end(); j++)
					{
						/*��ȡj�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
						double lon_cur = res_plane_lon[plane_stops[*j]];
						double lat_cur = res_plane_lat[plane_stops[*j]];
						double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
						if (railplane_to_sub.count(*j))
							continue;
						subname = railplane_to_sub[*j];
						for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
						{
							memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
							transportation temp = search_subway_min_price(res, *i, *t, *j, r_time, cur_city_flag, end_city);
							res = new transportation();
							*res = temp;
							transportation cur_res;
							cur_res.stop_name = *j;
							cur_res.spendtimes = 30; //����ӵ���վ���е�����վҪ 30min
							cur_res.price = 0; //���в���Ǯ
							cur_res.g = temp.g + cur_res.price;
							cur_res.h = h;
							cur_res.transportation_name = "����";
							cur_res.represent_number = "����";
							cur_res.change_trans_times = res->change_trans_times + 1; //����ͨ��ʽ
							cur_res.starting_time = res->arrival_time;
							str_times = split(cur_res.starting_time, ':');
							cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
							cur_res.last = res;
							heap.add(cur_res);
						}
					}
				}
			}
		}

		if (plane_stops.count(res->stop_name))	//����ɻ�����
		{
			//�������վ�����ڳ��У��˳�����
			if (res_plane_city[plane_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //˵���ڸ�վ�㻻�˹���
					{
						plane_stop_visited[plane_stops[pathData->last->stop_name]] = false;
						break;
					}
					pathData = pathData->last;
				}
				//plane_stop_visited[plane_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
				laststop = search_subway_min_price(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				//break;
				continue;
			}
			plane_stop_visited[plane_stops[res->stop_name]] = true;	//��վ���Ϊ�ѷ���
			string arr_flight_number = res->represent_number;	//���浽�ﵱǰ�����ĺ����
			str_times = split(res->arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//����ð�ηɻ����ﵱǰ������ʱ��
			int cur_stop = plane_stops[res->stop_name];
			//�����ӵ�ǰ���������ܹ���������л���
			for (map<int, plane_means>::iterator it = plane_list[cur_stop].begin(); it != plane_list[cur_stop].end(); it++)
			{
				/*��ȡit�����Ŀ�Ļ����ľ�γ��lon_cur,lat_cur*/
				double lon_cur = res_plane_lon[it->first];
				double lat_cur = res_plane_lat[it->first];
				double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

				transportation temp;
				if (plane_stop_visited[it->first] == true)	//��Ŀ�Ļ����ѱ����ʹ�
					continue;

				int num = it->second.planenumbers;	//�ܹ�����Ŀ�Ļ����ķɻ�����
				for (int i = 0; i < num; i++)
				{
					temp = it->second.planes[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//�ӵ�վ���ٴ���ɼ����ʱ��

					if (leave_time < arr_time)	//�ڵ���֮ǰ���ѷ�����������
					{
						continue;
					}

					wait_time = leave_time - arr_time;

					if (temp.represent_number != arr_flight_number) //�����Ҫ����
					{
						if (wait_time < 30) //���ǻ��˵�ʵ���������Ҫ30min����ʱ��
							continue;
						temp.change_trans_times += 1; //������
					}

					temp.g = temp.price + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//��ѯ�Ƿ��Ѿ��ڶ���
					if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //���ڶ��У������
					{
						temp.h = h;
						temp.last = res;
						heap.add(temp);
					}
				}

			}
			//���Ǵ���һ�������Ŀ��վ�㻻���������
			int cur_city_flag = res_plane_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //˵����Ŀ�ĵطɻ������ڳ����е�����Ϣ
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_railway_stops[cur_city_flag].begin(); j != have_railway_stops[cur_city_flag].end(); j++)
					{
						if (railplane_to_sub.count(*j))
							continue;
						/*��ȡj�����Ŀ��վ��ľ�γ��lon_cur,lat_cur*/
						double lon_cur = res_rail_lon[railway_stops[*j]];
						double lat_cur = res_rail_lat[railway_stops[*j]];
						double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
						subname = railplane_to_sub[*j];
						for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
						{
							memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
							transportation temp = search_subway_min_price(res, *i, *t, *j, r_time, cur_city_flag, end_city);
							res = new transportation();
							*res = temp;
							transportation cur_res;
							cur_res.stop_name = *j;
							cur_res.spendtimes = 10; //����ӵ���վ���е�����վҪ 10min
							cur_res.price = 0; //���в���Ǯ
							cur_res.g = temp.g + cur_res.price;
							cur_res.h = h;
							cur_res.transportation_name = "����";
							cur_res.represent_number = "����";
							cur_res.change_trans_times = res->change_trans_times + 1; //����ͨ��ʽ
							cur_res.starting_time = res->arrival_time;
							str_times = split(cur_res.starting_time, ':');
							cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
							cur_res.last = res;
							heap.add(cur_res);
						}
					}
				}
			}
		}
	}


	if (fin_flag == -1)
	{
		cout << "��ѡ��ĳ���վ������Ԥ�Ƴ���ʱ��θ���û�г��Σ��볢��ѡ������ʱ���" << endl;
		return string();
	}
	else if (fin_flag == 0)
	{
		cout << "��Ǹ��δ�ܲ�ѯ�����ʵĹ滮�������볢�Բ�ͬʱ���վ�㲢����" << endl;
		return string();
	}
	else
	{
		sort(laststops.begin(), laststops.end(), cmp1);
		transportation *pathData = &laststops[0];	//��ӡ·��
		transportation res_route, res_next_route;
		int real_price = (int)pathData->g;
		int change_times = pathData->change_trans_times;
		stack<transportation> route;
		string result = "";
		int fl = 0;
		string cur_represent_number = "";
		for (pathData = &laststops[0]; pathData->last != nullptr; )
		{
			route.push(*pathData);
			pathData = pathData->last;
		}
		ofstream destFile("��ʡǮ·���滮.txt", ios::out);
		res_route = route.top();
		destFile << res_route.transportation_name << endl;	result = result + res_route.transportation_name + "*";
		destFile << res_route.starting_time << endl;	result = result + res_route.starting_time + "*";
		destFile << res_route.represent_number << endl;	result = result + res_route.represent_number + "*";
		destFile << res_route.last->stop_name << endl;	result = result + res_route.last->stop_name + "*";
		cout << endl << res_route.last->stop_name << "����" << res_route.starting_time << "_" << res_route.transportation_name
			<< "_" << res_route.represent_number << "_";
		while (!route.empty())
		{
			res_route = route.top();
			route.pop();
			if (route.empty())	break;
			res_next_route = route.top();
			if (res_route.represent_number != res_next_route.represent_number)
			{
				destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
				destFile << res_route.arrival_time << endl;	result = result + res_route.arrival_time + "_";
				cout << res_route.arrival_time << "����>" << res_route.stop_name << endl;

				destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
				destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
				destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
				destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
				cout << res_next_route.last->stop_name << "����" << res_next_route.starting_time << "_" << res_next_route.transportation_name
					<< "_" << res_next_route.represent_number << "_";
			}
		}
		destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
		destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
		cout << res_route.arrival_time << "����>" << res_route.stop_name << endl << endl;
		destFile << "����" << endl << "����" << endl << "����" << endl << "����" << endl;
		result = result + "����*����*����*����*";
		destFile << "��Ʊ�ۣ�" << real_price << endl;
		result = result + "��Ʊ�ۣ�" + to_string(real_price) + "*";
		destFile << "ת��ת���ܴ�����" << change_times;
		result = result + "ת��ת���ܴ�����" + to_string(change_times) + "_";
		cout << "��Ʊ��" << real_price << endl;
		destFile.close();
		cout << "�����滮�ɹ�" << endl;
		return result;
	}
}

transportation search_subway_min_price(transportation * node, string start, string end, string nearname, int time, int cur_city_flag, string end_city)
{
	memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
	MinHeap heap = MinHeap();
	transportation *res;	//�ڵ�ָ��
	transportation laststop;	//������һ��վ����Ϣ�����ڷ������·��
	map<int, subway_means>::iterator it;	//���ڱ���subway_list
	vector<string> str_times;

	if (node == nullptr)
	{
		res = new transportation(start, 0, 0, nullptr, "���");
		res->price = 3; //Ĭ�ϵ���Ʊ��3Ԫ
		res->g = 3;
		res->change_trans_times = 0;
		res->arrival_time = trans_to_str_time(time);
	}
	else
	{
		res = new transportation();
		res->stop_name = start;
		res->spendtimes = 10; //����10min
		res->price = 0;
		res->last = node;
		res->g = node->g + res->price;
		res->h = 0;
		res->change_trans_times = node->change_trans_times; //����ͨ��ʽ�Ĵ���
		res->transportation_name = "����";
		res->represent_number = "����";
		res->starting_time = node->arrival_time;
		str_times = split(res->starting_time, ':');
		res->arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + res->spendtimes);
	}
	str_times = split(res->arrival_time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��������ʱ�任�������

	subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][start]] = true;	//��ʼվ����Ϊ�ѷ���

	//�����ӵ�ǰ��վ�����ܹ����������վ��
	for (it = subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].begin(); it != subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].end(); it++)
	{
		transportation temp;
		int num = it->second.subnumbers;	//����һ��վ��ĵ�������
		for (int i = 0; i < num; i++)
		{
			temp = it->second.subways[i];
			str_times = split(temp.starting_time, ':');
			int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��ȡ��������ʱ��
			str_times = split(temp.arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arr_time < start_time)	//˵����ĩ����ʱ���������
			{
				arr_time += 1440;
			}
			//����ʱ���ڳ�����������Ӫʱ���ڣ���Ϊ��������
			if (start_time <= r_time && arr_time >= r_time)
			{
				temp.g = temp.price + res->g;
				str_times = split(res->arrival_time, ':');
				double s_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + 2;
				temp.starting_time = trans_to_str_time(s_time);
				double a_time = s_time + temp.spendtimes;
				temp.arrival_time = trans_to_str_time(a_time);
				temp.change_trans_times = res->change_trans_times;
				temp.h = 0;	//����վ�����������������˻�ΪDijkstra�㷨
				temp.last = res;
				heap.add(temp);
			}
		}
	}
	/*�����㷨ִ��*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}
	while (!finish && !heap.isEmpty())
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//ȡ��fֵ��С�ĵ�
		if (res->stop_name == end)	//�ҵ�����վ�㣬�˳�����
		{
			laststop = *res;
			finish = true;
			fin_flag = 1;
			break;
		}
		string arr_checi = res->represent_number;	//���浽�ﵱǰ��վ����·
		subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][res->stop_name]] = true;	//��վ���Ϊ�ѷ���
		str_times = split(res->arrival_time, ':');
		int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��������ʱ�任�������
		int cur_stop = subway_stops[cur_city_flag][res->stop_name];
		//�����ӵ�ǰ��վ�����ܹ����������վ��
		for (map<int, subway_means>::iterator it = subway_list[cur_city_flag][cur_stop].begin(); it != subway_list[cur_city_flag][cur_stop].end(); it++)
		{
			transportation temp;
			if (subway_stop_visited[cur_city_flag][it->first] == true)	//��Ŀ��վ���ѱ����ʹ�
				continue;

			int num = it->second.subnumbers;	//�ܹ�����Ŀ��վ��ĸ�������
			for (int i = 0; i < num; i++)
			{
				temp = it->second.subways[i];
				str_times = split(temp.starting_time, ':');
				int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��ȡ��������ʱ��
				str_times = split(temp.arrival_time, ':');
				int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arr_time < start_time)	//˵����ĩ����ʱ���������
				{
					arr_time += 1440;
				}
				temp.price = res->price;
				temp.g = temp.price + res->g;
				temp.change_trans_times = res->change_trans_times;
				str_times = split(res->arrival_time, ':');
				int cur_start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (temp.represent_number != arr_checi) //��Ҫ����
				{
					cur_start_time += 3;	//����������߶��⻨�ѵ�3min
					temp.change_trans_times += 1; //����ͨ��ʽ
				}
				//res->arrival_time = trans_to_str_time(res->g + time);
				temp.starting_time = trans_to_str_time(cur_start_time);
				temp.arrival_time = trans_to_str_time(cur_start_time + temp.spendtimes);

				transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//��ѯ�Ƿ��Ѿ��ڶ���
				if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
				{
					if (Inquire->g > temp.g)
					{
						Inquire->g = temp.g;
						Inquire->last = res;
					}
				}
				else //���ڶ��У������
				{
					temp.h = 0;
					temp.last = res;
					heap.add(temp);
				}
			}
		}
	}
	return laststop;
}

transportation search_subway_min_time(transportation * node, string start, string end, string nearname, int time, int cur_city_flag, string end_city)
{
	memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
	MinHeap heap = MinHeap();
	transportation *res;	//�ڵ�ָ��
	transportation laststop;	//������һ��վ����Ϣ�����ڷ������·��
	map<int, subway_means>::iterator it;	//���ڱ���subway_list
	vector<string> str_times;

	if (node == nullptr)
	{
		res = new transportation(start, 0, 0, nullptr, "���");
		res->price = 3; //Ĭ�ϵ���Ʊ��3Ԫ
		res->change_trans_times = 0;
	}
	else
	{
		res = new transportation();
		res->stop_name = start;
		res->spendtimes = 10; //����10min
		res->last = node;
		res->g = node->g + res->spendtimes;
		res->h = 0;
		res->price = 0;
		res->change_trans_times = node->change_trans_times + 1; //����ͨ��ʽ�Ĵ���
		res->transportation_name = "����";
		res->represent_number = "����";
		double s_time = node->g + time;
		double a_time = res->g + time;
		res->starting_time = trans_to_str_time(s_time);
		res->arrival_time = trans_to_str_time(a_time);
	}

	int r_time = (int)res->g + time;	//��������ʱ�任�������

	subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][start]] = true;	//��ʼվ����Ϊ�ѷ���

	//�����ӵ�ǰ��վ�����ܹ����������վ��
	for (it = subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].begin(); it != subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].end(); it++)
	{
		transportation temp;
		int num = it->second.subnumbers;	//����һ��վ��ĵ�������
		for (int i = 0; i < num; i++)
		{
			temp = it->second.subways[i];
			str_times = split(temp.starting_time, ':');
			int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��ȡ��������ʱ��
			str_times = split(temp.arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arr_time < start_time)	//˵����ĩ����ʱ���������
			{
				arr_time += 1440;
			}
			//����ʱ���ڳ�����������Ӫʱ���ڣ���Ϊ��������
			if (start_time <= r_time && arr_time >= r_time)
			{
				temp.g = temp.spendtimes + res->g + 2; //�ȵ������� 2min
				double s_time = res->g + 2 + time;
				temp.starting_time = trans_to_str_time(s_time);
				double a_time = temp.g + time;
				temp.arrival_time = trans_to_str_time(a_time);
				temp.change_trans_times = res->change_trans_times;
				temp.h = 0;	//����վ�����������������˻�ΪDijkstra�㷨
				temp.last = res;
				heap.add(temp);
			}
		}
	}
	/*�����㷨ִ��*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}
	while (!finish && !heap.isEmpty())
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//ȡ��fֵ��С�ĵ�
		if (res->stop_name == end)	//�ҵ�����վ�㣬�˳�����
		{
			laststop = *res;
			finish = true;
			fin_flag = 1;
			break;
		}
		string arr_checi = res->represent_number;	//���浽�ﵱǰ��վ����·
		subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][res->stop_name]] = true;	//��վ���Ϊ�ѷ���
		int r_time = (int)res->g + time; //���浽�ﵱǰ��վ��ʱ��
		int cur_stop = subway_stops[cur_city_flag][res->stop_name];
		//�����ӵ�ǰ��վ�����ܹ����������վ��
		for (map<int, subway_means>::iterator it = subway_list[cur_city_flag][cur_stop].begin(); it != subway_list[cur_city_flag][cur_stop].end(); it++)
		{
			transportation temp;
			if (subway_stop_visited[cur_city_flag][it->first] == true)	//��Ŀ��վ���ѱ����ʹ�
				continue;

			int num = it->second.subnumbers;	//�ܹ�����Ŀ��վ��ĸ�������
			for (int i = 0; i < num; i++)
			{
				temp = it->second.subways[i];
				str_times = split(temp.starting_time, ':');
				int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//��ȡ��������ʱ��
				str_times = split(temp.arrival_time, ':');
				int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arr_time < start_time)	//˵����ĩ����ʱ���������
				{
					arr_time += 1440;
				}
				temp.g = temp.spendtimes + res->g;
				temp.change_trans_times = res->change_trans_times;
				temp.price = res->price;
				if (temp.represent_number != arr_checi) //��Ҫ����
				{
					temp.g += 3; //����������߶��⻨�ѵ�3min
					temp.change_trans_times += 1; //����ͨ��ʽ
				}
				res->arrival_time = trans_to_str_time(res->g + time);
				temp.starting_time = trans_to_str_time(temp.g - temp.spendtimes + time);
				temp.arrival_time = trans_to_str_time(temp.g + time);

				transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//��ѯ�Ƿ��Ѿ��ڶ���
				if (Inquire != nullptr)	//�ڶ��У�����gֵ����һ��վ����Ϣ
				{
					if (Inquire->g > temp.g)
					{
						Inquire->g = temp.g;
						Inquire->last = res;
					}
				}
				else //���ڶ��У������
				{
					temp.h = 0;
					temp.last = res;
					heap.add(temp);
				}
			}
		}
	}
	return laststop;
}

void get_lon_and_lat_and_city()
{
	int num;	//������Ž��������
	string temp[6];	//�������ݵ�����Ϣ
	int city_flag = 0;	//��¼��ͬ���б��
	//�ȶ�ȡ������Ϣ����֪�ɵĳ���
	if (!mysql_query(&ceshi, "SELECT * FROM �ɻ�����Ϣ��"))   //����ѯ�ɹ�����0��ʧ�ܷ��������
		cout << "\n----�ɻ�����Ϣ���ѯ�ɹ�----" << endl;
	result = mysql_store_result(&ceshi);    //����ѯ���Ľ�������浽result��
	num = mysql_num_fields(result);		//�������������ŵ�num��
	while ((row = mysql_fetch_row(result)))  //�������һ�У�����ֹѭ��
	{
		for (int i = 0; i < num; i++)         //������е�ÿһ��
			temp[i] = row[i];
		int flag = plane_stops[temp[0]];	//վ����
		res_plane_lon[flag] = atof(temp[2].c_str());
		res_plane_lat[flag] = atof(temp[1].c_str());
		if (city.insert(map<string, int>::value_type(temp[3], city_flag)).second == true)
		{
			city_flag++;
		}
		res_plane_city[flag] = city[temp[3]];	//��������������еı��
		res_city_lon[city[temp[3]]] = atof(temp[5].c_str());	//������г��еľ�γ����Ϣ
		res_city_lat[city[temp[3]]] = atof(temp[4].c_str());
		have_plane_stops[city[temp[3]]].push_back(temp[0]);	//�����������еĻ�������
	}
	allow_trans_numbers = city_flag;	//��������ĳ�������
	//cout << allow_trans_numbers << endl;

	if (!mysql_query(&ceshi, "SELECT * FROM ����վ��Ϣ��"))   //����ѯ�ɹ�����0��ʧ�ܷ��������
		cout << "\n----����վ��Ϣ���ѯ�ɹ�----" << endl;
	result = mysql_store_result(&ceshi);    //����ѯ���Ľ�������浽result��
	num = mysql_num_fields(result);		//�������������ŵ�num��
	while ((row = mysql_fetch_row(result)))  //�������һ�У�����ֹѭ��
	{
		for (int i = 0; i < num; i++)         //������е�ÿһ��
			temp[i] = row[i];
		int flag = railway_stops[temp[0]];
		res_rail_lon[flag] = atof(temp[2].c_str());
		res_rail_lat[flag] = atof(temp[1].c_str());
		if (city.insert(map<string, int>::value_type(temp[3], city_flag)).second == true)
		{
			city_flag++;
		}
		res_rail_city[flag] = city[temp[3]];	//���泵վ�������еı��
		if (city[temp[3]] < allow_trans_numbers)
			have_railway_stops[city[temp[3]]].push_back(temp[0]);	//�����������еĸ���վ����

		//cout << temp[0] << endl;
	}

	cout << "\nվ����Ϣ��ȡ�ɹ�" << endl;
}

vector<string> split(string strs, char ch)
{
	vector<string> res;
	if (strs == "")
		return res;
	//���ַ���ĩβҲ����ָ����������ȡ���һ��
	strs = strs + ch;
	size_t pos = strs.find(ch);
	string temp;
	while (pos != strs.npos)
	{
		temp = strs.substr(0, pos);
		res.push_back(temp);
		//ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(ch);
	}
	return res;
}

double cal_h(double lon1, double lat1, double lon2, double lat2, string trans_means)
{
	// �õ���㾭γ��,��ת��Ϊ�Ƕ�
	double startLon = (PI / 180) * lon1;
	double startLan = (PI / 180) * lat1;
	// �õ��յ㾭γ��,��ת��Ϊ�Ƕ�
	double endLon = (PI / 180) * lon2;
	double endtLan = (PI / 180) * lat2;

	// ����ƽ���뾶Ϊ6371km
	double earthR = 6371;

	// ���㹫ʽ
	double distence =
		acos(sin(startLan) * sin(endtLan) + cos(startLan) * cos(endtLan) * cos(endLon - startLon)) * earthR;

	if (trans_means == "railway")
		return (distence / 230) * 60; //���������ƽ�������ٶ�Ϊ230km/h

	else if (trans_means == "plane")
		return (distence / 390) * 60;
	return -1;
}

string trans_to_str_time(double i_time)
{
	if (i_time >= 1440)
		i_time -= 1440;
	int h = (int)i_time / 60;
	int min = (int)i_time - 60 * h;
	string str_h = to_string(h);
	string str_min = to_string(min);
	if (h < 10)
		str_h = "0" + str_h;
	if (min < 10)
		str_min = "0" + str_min;
	string str_time = str_h + ":" + str_min;
	return str_time;
}
