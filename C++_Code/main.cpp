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

bool cmp1(transportation a, transportation b)	//用于最省钱和最省时排序
{
	return a.g < b.g;
}

bool cmp2(transportation a, transportation b)	//用于最舒适排序
{
	if (a.change_trans_times == b.change_trans_times)
		return a.g < b.g;
	return a.change_trans_times < b.change_trans_times;
}

MYSQL ceshi;
MYSQL_RES *result;	//结果集
MYSQL_ROW row;	//保存表中一行的信息
map<string, int> city;	//城市名映射为编号
vector<string> have_railway_stops[6];	//暂保存6个城市拥有的高铁站名称
vector<string> have_plane_stops[6];	//暂保存6个城市拥有的飞机场名称
map<string, vector<string> > railplane_to_sub;	//高铁站附近的地铁站
bool city_visited[6];
double res_city_lon[6];
double res_city_lat[6];
int allow_trans_numbers;	//暂时允许经地铁转换交通方式的城市数量

map<string, int> railway_stops;	//将高铁站名映射为编号
map<int, railway_means> railway_list[620];	//构建邻接表存储高铁站的图信息
bool rail_stop_visited[620];
double res_rail_lon[620];	//经度表
double res_rail_lat[620];	//纬度表
int res_rail_city[620];	//高铁站所属城市表

map<string, int> plane_stops; //将飞机场名映射为编号
map<int, plane_means> plane_list[100];	//构建邻接表存储飞机场的图信息
bool plane_stop_visited[100];
double res_plane_lon[100];	//经度表
double res_plane_lat[100];	//纬度表
int res_plane_city[100];	//飞机场所属城市表

map<string, int> subway_stops[6];	//将6个城市的地铁站名映射为编号
map<int, subway_means> subway_list[6][400];	//构建邻接表存储6个城市地铁站的图信息
bool subway_stop_visited[6][400];
//double res_subway_lon[6][400];	//经度表
//double res_subway_lat[6][600];	//纬度表
//int res_subway_city[600];	//高铁站所属城市表

void create_map();
void create_railway_map();
void create_plane_map();
void create_subway_map();

string search_min_time(string start, string start_city, string end, string end_city, string time, string plan);//搜索最优方案
string search_min_price(string start, string start_city, string end, string end_city, string time, string plan);//搜索最省钱方案
transportation search_subway_min_time(transportation* node, string start, string end, string nearname, int time, int cur_city_flag, string end_city);
transportation search_subway_min_price(transportation * node, string start, string end, string nearname, int time, int cur_city_flag, string end_city);

//void search_min_price(string start, string end, string time);	//最省钱方案
//void search_railway_min_price(string start, string end, string time);	//高铁网络内搜索最省钱方案
//void search_plane_min_price(string start, string end, string time);	//飞机网络内搜索最省钱方案

//void search_min_trouble(string start, string end, string time);	//最舒适方案
//void search_railway_min_trouble(string start, string end, string time);	//高铁网络内搜索最舒适方案
//void search_plane_min_trouble(string start, string end, string time);	//飞机网络内搜索最舒适方案

vector<string> split(string strs, char ch);	//字符串分割函数
string trans_to_str_time(double i_time);
void get_lon_and_lat_and_city();
double cal_h(double lon1, double lat1, double lon2, double lat2, string trans_means);

int main()
{
	create_map();

	string start, start_city, end, end_city, time, plan;
	while (1)
	{
		cout << "\n请输入 起点站、起点城市、终点站、终点城市、预计出发时间和规划方案：" << endl;
		cin >> start >> start_city >> end >> end_city >> time >> plan;
		if (plan == "最省钱")
			search_min_price(start, start_city, end, end_city, time, plan);
		else
			search_min_time(start, start_city, end, end_city, time, plan);
	}

	return 0;
}

void create_map()
{
	mysql_init(&ceshi);   //初始化MYSQL变量
	//mysql_options(&ceshi, MYSQL_SET_CHARSET_NAME, "utf8");	//linux下
	mysql_options(&ceshi, MYSQL_SET_CHARSET_NAME, "gbk");	//windows下设置读取时以gbk编码解释字符流避免中文显示乱码
	//129.211.3.97 123456
	if (mysql_real_connect(&ceshi, "localhost", "root", "zyc666233", "test", 3306, NULL, 0))  //连接到mysql
		cout << "\n----MySQL已连接----" << endl;

	create_railway_map();
	create_plane_map();
	get_lon_and_lat_and_city();
	create_subway_map();

	mysql_free_result(result);     //释放结果集所占用的内存
	mysql_close(&ceshi);          //关闭与mysql的连接
}

void create_railway_map()
{
	int num;	//用来存放结果集列数
	if (!mysql_query(&ceshi, "SELECT * FROM 高铁"))   //若查询成功返回0，失败返回随机数
		cout << "\n----高铁表查询成功----" << endl;
	result = mysql_store_result(&ceshi);    //将查询到的结果集储存到result中
	num = mysql_num_fields(result);        //将结果集列数存放到num中

	string temp[50];	//复制数据的列信息
	int flag = 0;	//记录不同的站点的编号
	vector<string> str1;
	vector<string> str2;
	vector<string> str_times;
	int start_time, arrival_time;
	//railway_stops.clear();	//清空高铁站点信息
	//ofstream destFile("高铁站名称.txt", ios::out); //以文本模式打开out.txt备写
	while ((row = mysql_fetch_row(result)))  //遇到最后一行，则中止循环
	{
		for (int i = 0; i < num; i++)         //输出该行的每一列
			temp[i] = row[i];	//row是MYSQL_ROW变量，可以当做数组使用，i为列数

		str1 = split(temp[1], '_');
		if (railway_stops.insert(map<string, int>::value_type(str1[0], flag)).second == true)
		{
			//find_railway_stops[flag] = str1[0];
			//cout << str1[0] << " " << flag << endl;
			//destFile << flag << "," << str1[0] << "站" << endl;
			flag++;
		}
		for (int j = 2; j < num; j++)
		{
			if (temp[j] == "")	break;
			str1 = split(temp[j - 1], '_');	//name_lastarrivaltime_nextstarttime_stoptime
			str2 = split(temp[j], '_');
			if (railway_stops.insert(map<string, int>::value_type(str2[0], flag)).second == true)	//防止同一站名重复映射
			{
				//find_railway_stops[flag] = str2[0];
				//cout << str2[0] << " " << flag << endl;
				//destFile << flag << "," << str2[0] << "站" << endl;
				flag++;
			}
			//str1[0]为出发站，str2[0]为目的站
			int flag1 = railway_stops[str1[0]];
			int flag2 = railway_stops[str2[0]];
			if (railway_list[flag1].count(flag2) == 0)
			{
				railway_means res;
				//res.flag = flag2;
				res.railways[res.railnumbers].represent_number = temp[0];	//车次
				res.railways[res.railnumbers].starting_time = str1[2];
				res.railways[res.railnumbers].arrival_time = str2[1];
				res.railways[res.railnumbers].transportation_name = "高铁";
				str_times = split(str1[2], ':');
				start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				str_times = split(str2[1], ':');
				arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arrival_time < start_time)	//说明发生跨天情况
				{
					res.railways[res.railnumbers].spendtimes = 1440 - start_time + arrival_time;
				}
				else //计算时间
				{
					res.railways[res.railnumbers].spendtimes = arrival_time - start_time;
				}

				if (temp[0][0] == 'G')	//高铁票价
					res.railways[res.railnumbers].price = (int)(1.5 * res.railways[res.railnumbers].spendtimes);
				else if (temp[0][0] == 'C')	//城际票价
					res.railways[res.railnumbers].price = (int)(1.75 * res.railways[res.railnumbers].spendtimes);
				else //动车票价
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
				railway_list[flag1][flag2].railways[railnumbers].transportation_name = "高铁";
				str_times = split(str1[2], ':');
				start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				str_times = split(str2[1], ':');
				arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arrival_time < start_time)	//说明发生跨天情况
				{
					railway_list[flag1][flag2].railways[railnumbers].spendtimes = 1440 - start_time + arrival_time;
				}
				else //计算时间
				{
					railway_list[flag1][flag2].railways[railnumbers].spendtimes = arrival_time - start_time;
				}

				if (temp[0][0] == 'G')	//高铁票价
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(1.5 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);
				else if (temp[0][0] == 'C')	//城际票价
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(1.75 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);
				else  //动车票价
					railway_list[flag1][flag2].railways[railnumbers].price = (int)(0.9 * railway_list[flag1][flag2].railways[railnumbers].spendtimes);

				railway_list[flag1][flag2].railways[railnumbers].stop_name = str2[0];
				railway_list[flag1][flag2].railnumbers++;
			}
		}
	}
	//cout << flag - 1;
	//destFile.close();
	cout << "\n高铁表构建成功" << endl;
}

void create_plane_map()
{
	int num;	//用来存放结果集列数
	if (!mysql_query(&ceshi, "SELECT * FROM 飞机"))   //若查询成功返回0，失败返回随机数
		cout << "\n----飞机表查询成功----" << endl;
	result = mysql_store_result(&ceshi);    //将查询到的结果集储存到result中
	num = mysql_num_fields(result);        //将结果集列数存放到num中

	string temp[10];	//复制数据的列信息
	int flag = 0;	//记录不同的站点的编号
	vector<string> str1;
	vector<string> str2;
	vector<string> str_times;
	int start_time, arrival_time;
	//stops.clear();
	//ofstream destFile("机场名称.txt", ios::out); //以文本模式打开备写
	while ((row = mysql_fetch_row(result)))  //遇到最后一行，则中止循环
	{
		for (int i = 0; i < num; i++)         //输出该行的每一列
			temp[i] = row[i];	//row是MYSQL_ROW变量，可以当做数组使用，i为列数
		if (plane_stops.insert(map<string, int>::value_type(temp[1], flag)).second == true)	//防止同一站名重复映射
		{
			//cout << temp[1] << " " << flag << endl;
			//destFile << flag << "," << temp[1] << endl;
			flag++;
		}
		if (plane_stops.insert(map<string, int>::value_type(temp[3], flag)).second == true)	//防止同一站名重复映射
		{
			//cout << temp[3] << " " << flag << endl;
			//destFile << flag << "," << temp[3] << endl;
			flag++;
		}
		//temp[1]为出发站，temp[3]为目的站
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
			res.planes[res.planenumbers].transportation_name = "飞机";
			str_times = split(temp[2], ':');
			start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			str_times = split(temp[4], ':');
			arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arrival_time < start_time)	//说明发生跨天情况
			{
				res.planes[res.planenumbers].spendtimes = 1440 - start_time + arrival_time;
			}
			else //计算时间
			{
				res.planes[res.planenumbers].spendtimes = arrival_time - start_time;
			}
			res.planes[res.planenumbers].price = atoi(temp[5].c_str());	//录入票价
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
			plane_list[flag1][flag2].planes[planenumbers].transportation_name = "飞机";
			str_times = split(temp[2], ':');
			start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			str_times = split(temp[4], ':');
			arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arrival_time < start_time)	//说明发生跨天情况
			{
				plane_list[flag1][flag2].planes[planenumbers].spendtimes = 1440 - start_time + arrival_time;
			}
			else //计算时间
			{
				plane_list[flag1][flag2].planes[planenumbers].spendtimes = arrival_time - start_time;
			}
			plane_list[flag1][flag2].planes[planenumbers].price = atoi(temp[5].c_str());
			plane_list[flag1][flag2].planenumbers++;
		}
		//cout << temp[0] << endl;
	}
	//destFile.close();
	cout << "\n飞机表构建成功" << endl;
}

void create_subway_map()
{
	int num;	//用来存放结果集列数
	map<string, int>::iterator it;	//用于遍历city
	int allow_city;	//目前保存的6个城市的地铁信息
	for (it = city.begin(), allow_city = 0; it != city.end() && allow_city < allow_trans_numbers; it++)
	{
		int city_flag = it->second;//城市编号
		if (city_flag < allow_trans_numbers)
		{
			allow_city++;
			string str = "SELECT * FROM " + it->first;
			if (!mysql_query(&ceshi, str.c_str()))   //若查询成功返回0，失败返回随机数
				cout << "\n----" + it->first + "地铁表查询成功----" << endl;
			result = mysql_store_result(&ceshi);    //将查询到的结果集储存到result中
			num = mysql_num_fields(result);        //将结果集列数存放到num中

			string temp[50];	//复制数据的列信息
			int flag = 0;	//记录不同的站点的编号
			vector<string> str1;
			vector<string> str2;
			vector<string> str3;
			vector<string> str_times;
			int start_time, arrival_time;
			//railway_stops.clear();	//清空地铁站点信息
			//string str = it->first + "地铁站名称.txt";
			//ofstream destFile(str.c_str(), ios::out); //以文本模式打开out.txt备写
			while ((row = mysql_fetch_row(result)))  //遇到最后一行，则中止循环
			{
				for (int i = 0; i < num; i++)         //输出该行的每一列
					temp[i] = row[i];	//row是MYSQL_ROW变量，可以当做数组使用，i为列数

				str1 = split(temp[1], '_');
				if (str1[0].find('*') != string::npos)	//说明该地铁站靠近高铁站
				{
					str3 = split(str1[0], '*');
					for (int i = 1; i < str3.size(); i++)
					{
						vector<string>::iterator ite = find(railplane_to_sub[str3[i]].begin(), railplane_to_sub[str3[i]].end(), str3[0]);
						if (ite == railplane_to_sub[str3[i]].end()) //该地铁站未录入
							railplane_to_sub[str3[i]].push_back(str3[0]);
					}
					str1[0] = str3[0];
				}
				if (subway_stops[city_flag].insert(map<string, int>::value_type(str1[0], flag)).second == true)
				{
					//find_railway_stops[flag] = str1[0];
					//cout << str1[0] << " " << flag << endl;
					//destFile << flag << "," << str1[0] << "站" << endl;
					flag++;
				}
				for (int j = 2; j < num; j++)
				{
					if (temp[j] == "")	break;

					str1 = split(temp[j - 1], '_');	//name_lastarrivaltime_nextstarttime_stoptime
					if (str1[0].find('*') != string::npos)	//说明该地铁站靠近高铁站
					{
						str3 = split(str1[0], '*');
						str1[0] = str3[0];
					}
					str2 = split(temp[j], '_');
					if (str2[0].find('*') != string::npos)	//说明该地铁站靠近高铁站
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
					if (subway_stops[city_flag].insert(map<string, int>::value_type(str2[0], flag)).second == true)	//防止同一站名重复映射
					{
						//find_railway_stops[flag] = str2[0];
						//cout << str2[0] << " " << flag << endl;
						//destFile << flag << "," << str2[0] << "站" << endl;
						flag++;
					}
					//str1[0]为出发站，str2[0]为目的站
					int flag1 = subway_stops[city_flag][str1[0]];
					int flag2 = subway_stops[city_flag][str2[0]];
					if (subway_list[city_flag][flag1].count(flag2) == 0)
					{
						subway_means res;
						//res.flag = flag2;
						res.subways[res.subnumbers].represent_number = temp[0];	//地铁线路
						res.subways[res.subnumbers].starting_time = str1[1]; //早班车发车时间
						res.subways[res.subnumbers].arrival_time = str1[2];	//末班车发车时间
						res.subways[res.subnumbers].transportation_name = "地铁";
						str_times = split(str1[1], ':');
						start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						str_times = split(str2[1], ':');
						arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						if (arrival_time < start_time) //说明下一站车的首发时间早于前一站
							res.subways[res.subnumbers].spendtimes = 3; //估算 3min 运行时间
						else
							res.subways[res.subnumbers].spendtimes = arrival_time - start_time;
						res.subways[res.subnumbers].stop_name = str2[0];
						res.subways[res.subnumbers].price = 0; //地铁相邻两站间默认不花钱
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
						subway_list[city_flag][flag1][flag2].subways[subnumbers].transportation_name = "地铁";
						str_times = split(str1[1], ':');
						start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						str_times = split(str2[1], ':');
						arrival_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
						if (arrival_time < start_time) //说明下一站车的首发时间早于前一站
							subway_list[city_flag][flag1][flag2].subways[subnumbers].spendtimes = 3;  //估算 3min 运行时间
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
	cout << "\n地铁表构建成功" << endl;
}

string search_min_time(string start, string start_city, string end, string end_city, string time, string plan)
{
	memset(rail_stop_visited, false, sizeof(rail_stop_visited));
	memset(city_visited, false, sizeof(city_visited));
	memset(plane_stop_visited, false, sizeof(plane_stop_visited));
	memset(subway_stop_visited, false, sizeof(subway_stop_visited));
	vector<transportation> laststops; //存放结果
	MinHeap heap = MinHeap();
	transportation *res;	//节点指针
	transportation laststop;	//存放最后一个站点信息，用于反向输出路径
	//map<int, railway_means>::iterator it;	//用于遍历railway_list
	vector<string> str_times;
	str_times = split(time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//将预计出发时间换算成整数
	/*获取最终站点end所在城市的经纬度lon_end, lat_end*/
	double lon_end = res_city_lon[city[end_city]];
	double lat_end = res_city_lat[city[end_city]];
	res = new transportation();

	int city_flag = city[start_city];
	vector<string> subname;
	for (vector<string>::iterator it = have_railway_stops[city_flag].begin(); it != have_railway_stops[city_flag].end(); it++)
	{
		/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[railway_stops[*it]];
		double lat_cur = res_rail_lat[railway_stops[*it]];
		double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it]; //和高铁站最近的地铁站列表
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_time(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 10; //假设从地铁站步行到高铁站要 10min
			cur_res.price = 0; //步行不花钱
			cur_res.g = temp.g + cur_res.spendtimes;
			cur_res.h = h;
			cur_res.transportation_name = "步行";
			cur_res.represent_number = "——";
			cur_res.change_trans_times = res->change_trans_times; //换交通方式
			cur_res.starting_time = trans_to_str_time(temp.g + r_time);
			cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}

	for (vector<string>::iterator it = have_plane_stops[city_flag].begin(); it != have_plane_stops[city_flag].end(); it++)
	{
		/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[plane_stops[*it]];
		double lat_cur = res_rail_lat[plane_stops[*it]];
		double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it];	//飞机场最靠近的地铁站名列表
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_time(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
			cur_res.price = 0; //步行不花钱
			cur_res.g = temp.g + cur_res.spendtimes;
			cur_res.h = h;
			cur_res.transportation_name = "步行";
			cur_res.represent_number = "——";
			cur_res.change_trans_times = res->change_trans_times; //换交通方式
			cur_res.starting_time = trans_to_str_time(temp.g + r_time);
			cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}
	city_visited[city_flag] = true; //起始城市已访问

	/*A*搜索算法执行*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}

	int result_flag = 0; //记录搜索到的可行路径数量，用于最舒适计算
	while (!finish && !heap.isEmpty() && result_flag < 2)
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//取出f值最小的点
		if (railway_stops.count(res->stop_name))	//接入高铁网络
		{
			//到达结束站点所在城市，退出搜索
			if (res_rail_city[railway_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //说明在该站点换乘过车
					{
						rail_stop_visited[railway_stops[pathData->last->stop_name]] = false;
						rail_stop_visited[railway_stops[pathData->stop_name]] = true;
						break;
					}
					pathData = pathData->last;
				}
				//rail_stop_visited[railway_stops[res->stop_name]] = true;	//车站标记为已访问
				laststop = search_subway_min_time(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				continue;
				//break;
			}
			rail_stop_visited[railway_stops[res->stop_name]] = true;	//车站标记为已访问
			string arr_chici = res->represent_number;	//保存到达当前车站的车次

			str_times = split(res->arrival_time, ':');
			//int arr_time = (int)res->g + 60 * atoi(time.c_str()); //到达高铁站的时间
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//保存该车次到达当前车站的时间
			int cur_stop = railway_stops[res->stop_name];	//当前站点编号
			//遍历从当前车站出发能够到达的所有站点
			for (map<int, railway_means>::iterator it = railway_list[cur_stop].begin(); it != railway_list[cur_stop].end(); it++)
			{
				/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
				double lon_cur = res_rail_lon[it->first];
				double lat_cur = res_rail_lat[it->first];
				double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

				transportation temp;
				if (rail_stop_visited[it->first] == true)	//该目的站点已被访问过
					continue;

				int num = it->second.railnumbers;	//能够到达目的站点的高铁数量
				for (int i = 0; i < num; i++)
				{
					temp = it->second.railways[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//从到站到再次发车间隔的时间

					if (leave_time < arr_time)	//如果出现跨天的情况
					{
						wait_time = 1440 - arr_time + leave_time;
					}
					else //获取新乘坐高铁的出发时间和到站时间的差值
					{
						wait_time = leave_time - arr_time;
					}

					if (temp.represent_number != arr_chici) //如果需要换乘车次或交通方式
					{
						if (wait_time < 15) //考虑换乘的实际情况，需要15min换乘时间
							continue;
						temp.change_trans_times += 1; //换车次
					}
					//if(wait_time > )
					temp.g = temp.spendtimes + wait_time + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
					//查询是否已经在堆中
					if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
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
						temp.transportation_name = "高铁";
						heap.add(temp);
					}
				}
			}
			//考虑从下一个到达的目的站点换飞机的情况
			int cur_city_flag = res_rail_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //说明该目的高铁站所在城市有地铁信息
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_plane_stops[cur_city_flag].begin(); j != have_plane_stops[cur_city_flag].end(); j++)
					{
						/*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
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
							cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
							cur_res.price = 0; //步行不花钱
							cur_res.g = temp.g + cur_res.spendtimes;
							cur_res.h = h;
							cur_res.transportation_name = "步行";
							cur_res.represent_number = "——";
							cur_res.change_trans_times = res->change_trans_times + 1; //换交通方式
							cur_res.starting_time = trans_to_str_time(temp.g + r_time);
							cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
							cur_res.last = res;
							heap.add(cur_res);
						}
					}
				}
			}
		}

		if (plane_stops.count(res->stop_name))	//接入飞机网络
		{
			//到达结束站点所在城市，退出搜索
			if (res_plane_city[plane_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //说明在该站点换乘过车
					{
						plane_stop_visited[plane_stops[pathData->last->stop_name]] = false;
						break;
					}
					pathData = pathData->last;
				}
				//plane_stop_visited[plane_stops[res->stop_name]] = true;	//车站标记为已访问
				laststop = search_subway_min_time(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				//break;
				continue;
			}
			plane_stop_visited[plane_stops[res->stop_name]] = true;	//车站标记为已访问
			string arr_flight_number = res->represent_number;	//保存到达当前机场的航班次
			str_times = split(res->arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//保存该班次飞机到达当前机场的时间
			int cur_stop = plane_stops[res->stop_name];
			//遍历从当前机场出发能够到达的所有机场
			for (map<int, plane_means>::iterator it = plane_list[cur_stop].begin(); it != plane_list[cur_stop].end(); it++)
			{
				/*获取it代表的目的机场的经纬度lon_cur,lat_cur*/
				double lon_cur = res_plane_lon[it->first];
				double lat_cur = res_plane_lat[it->first];
				double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

				transportation temp;
				if (plane_stop_visited[it->first] == true)	//该目的机场已被访问过
					continue;

				int num = it->second.planenumbers;	//能够到达目的机场的飞机数量
				for (int i = 0; i < num; i++)
				{
					temp = it->second.planes[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//从到站到再次起飞间隔的时间

					if (leave_time < arr_time)	//如果出现跨天的情况
					{
						wait_time = 1440 - arr_time + leave_time;
					}
					else //获取新乘坐高铁的出发时间和到站时间的差值
					{
						wait_time = leave_time - arr_time;
					}

					if (temp.represent_number != arr_flight_number) //如果需要换乘
					{
						if (wait_time < 30) //考虑换乘的实际情况，需要30min换乘时间
							continue;
						temp.change_trans_times += 1; //换航班
					}

					temp.g = temp.spendtimes + wait_time + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
					if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //不在堆中，则加入
					{
						temp.h = h;
						temp.last = res;
						heap.add(temp);
					}
				}

			}
			//考虑从下一个到达的目的站点换高铁的情况
			int cur_city_flag = res_plane_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //说明该目的地飞机场所在城市有地铁信息
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_railway_stops[cur_city_flag].begin(); j != have_railway_stops[cur_city_flag].end(); j++)
					{
						if (railplane_to_sub.count(*j))
							continue;
						/*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
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
							cur_res.spendtimes = 10; //假设从地铁站步行到机场站要 10min
							cur_res.price = 0; //步行不花钱
							cur_res.g = temp.g + cur_res.spendtimes;
							cur_res.h = h;
							cur_res.transportation_name = "步行";
							cur_res.represent_number = "——";
							cur_res.change_trans_times = res->change_trans_times + 1; //换交通方式
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
		cout << "您选择的出发站点在您预计出行时间段附近没有车次，请尝试选择其它时间段" << endl;
	}
	else if (fin_flag == 0)
	{
		cout << "抱歉，未能查询到合适的规划方案，请尝试不同时间和站点并重试" << endl;
	}
	else
	{
		string result;
		if (plan == "最省时")
		{
			sort(laststops.begin(), laststops.end(), cmp1);
			ofstream destFile("最省时路径规划.txt", ios::out);
			transportation *pathData = &laststops[0];	//打印路径
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
			cout << endl << res_route.last->stop_name << "——" << res_route.starting_time << "_" << res_route.transportation_name
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
					cout << res_route.arrival_time << "——>" << res_route.stop_name << endl;

					destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
					destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
					destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
					destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
					cout << res_next_route.last->stop_name << "——" << res_next_route.starting_time << "_" << res_next_route.transportation_name
						<< "_" << res_next_route.represent_number << "_";
				}
			}
			destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
			destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
			cout << res_route.arrival_time << "——>" << res_route.stop_name << endl << endl;
			destFile << "——" << endl << "——" << endl << "——" << endl << "——" << endl;
			result = result + "——*——*——*——*";
			destFile << "总花费时间：" << trans_to_str_time(real_spend_time) << endl;
			result = result + "总花费时间：" + trans_to_str_time(real_spend_time) + "*";
			destFile << "转车转线总次数：" << change_times;
			result = result + "转车转线总次数：" + to_string(change_times) + "_";
			cout << "行程总时长：" << trans_to_str_time(real_spend_time) << endl;
			cout << "转车总次数：" << change_times << endl;
			destFile.close();
			cout << "方案规划成功" << endl;

		}
		if (plan == "最舒适")
		{
			sort(laststops.begin(), laststops.end(), cmp2);
			ofstream destFile("最舒适路径规划.txt", ios::out);
			transportation *pathData = &laststops[0];	//打印路径
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
			cout << endl << res_route.last->stop_name << "——" << res_route.starting_time << "_" << res_route.transportation_name
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
					cout << res_route.arrival_time << "——>" << res_route.stop_name << endl;

					destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
					destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
					destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
					destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
					cout << res_next_route.last->stop_name << "——" << res_next_route.starting_time << "_" << res_next_route.transportation_name
						<< "_" << res_next_route.represent_number << "_";
				}
			}
			destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
			destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
			cout << res_route.arrival_time << "——>" << res_route.stop_name << endl << endl;
			destFile << "——" << endl << "——" << endl << "——" << endl << "——" << endl;
			result = result + "——*——*——*——*";
			destFile << "总花费时间：" << trans_to_str_time(real_spend_time) << endl;
			result = result + "总花费时间：" + trans_to_str_time(real_spend_time) + "*";
			destFile << "转车转线总次数：" << change_times;
			result = result + "转车转线总次数：" + to_string(change_times) + "_";
			cout << "行程总时长：" << trans_to_str_time(real_spend_time) << endl;
			cout << "转车总次数：" << change_times << endl;
			destFile.close();
			cout << "方案规划成功" << endl;
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
	vector<transportation> laststops; //存放结果
	MinHeap heap = MinHeap();
	transportation *res;	//节点指针
	transportation laststop;	//存放最后一个站点信息，用于反向输出路径
	//map<int, railway_means>::iterator it;	//用于遍历railway_list
	vector<string> str_times;
	str_times = split(time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//将预计出发时间换算成整数
	/*获取最终站点end所在城市的经纬度lon_end, lat_end*/
	double lon_end = res_city_lon[city[end_city]];
	double lat_end = res_city_lat[city[end_city]];
	res = new transportation();

	int city_flag = city[start_city];
	vector<string> subname;
	for (vector<string>::iterator it = have_railway_stops[city_flag].begin(); it != have_railway_stops[city_flag].end(); it++)
	{
		/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[railway_stops[*it]];
		double lat_cur = res_rail_lat[railway_stops[*it]];
		double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway"); //预估函数换算成票价
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it]; //和高铁站最近的地铁站列表
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_price(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 10; //假设从地铁站步行到高铁站要 10min
			cur_res.price = 0; //步行不花钱
			cur_res.g = temp.g + cur_res.price; //计算g值
			cur_res.h = h;
			cur_res.transportation_name = "步行";
			cur_res.represent_number = "——";
			cur_res.change_trans_times = res->change_trans_times; //换交通方式
			cur_res.starting_time = res->arrival_time;
			str_times = split(cur_res.starting_time, ':');
			cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}

	for (vector<string>::iterator it = have_plane_stops[city_flag].begin(); it != have_plane_stops[city_flag].end(); it++)
	{
		/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
		double lon_cur = res_rail_lon[plane_stops[*it]];
		double lat_cur = res_rail_lat[plane_stops[*it]];
		double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
		if (railplane_to_sub.count(*it) == 0)
			continue;
		subname = railplane_to_sub[*it];	//飞机场最靠近的地铁站名列表
		for (vector<string>::iterator t = subname.begin(); t != subname.end(); t++)
		{
			memset(subway_stop_visited[city_flag], false, sizeof(subway_stop_visited[city_flag]));
			transportation temp = search_subway_min_price(nullptr, start, *t, *it, r_time, city_flag, end_city);
			res = new transportation();
			*res = temp;
			transportation cur_res;
			cur_res.stop_name = *it;
			cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
			cur_res.price = 0; //步行不花钱
			cur_res.g = temp.g + cur_res.price;
			cur_res.h = h;
			cur_res.transportation_name = "步行";
			cur_res.represent_number = "——";
			cur_res.change_trans_times = res->change_trans_times; //换交通方式
			cur_res.starting_time = res->arrival_time;
			str_times = split(cur_res.starting_time, ':');
			cur_res.arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + cur_res.spendtimes);
			cur_res.last = res;
			heap.add(cur_res);
		}
	}
	city_visited[city_flag] = true; //起始城市已访问

	/*A*搜索算法执行*/
	bool finish = false;
	int fin_flag = 0;
	if (heap.isEmpty())
	{
		finish = true;
		fin_flag = -1;
	}

	int result_flag = 0; //记录搜索到的可行路径数量，用于最舒适计算
	while (!finish && !heap.isEmpty() && result_flag < 4)
	{
		res = new transportation();
		*res = heap.getAndRemoveMin();	//取出f值最小的点
		if (railway_stops.count(res->stop_name))	//接入高铁网络
		{
			//到达结束站点所在城市，退出搜索
			if (res_rail_city[railway_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //说明在该站点换乘过车
					{
						rail_stop_visited[railway_stops[pathData->last->stop_name]] = false;
						rail_stop_visited[railway_stops[pathData->stop_name]] = true;
						break;
					}
					pathData = pathData->last;
				}
				//rail_stop_visited[railway_stops[res->stop_name]] = true;	//车站标记为已访问
				laststop = search_subway_min_price(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				continue;
				//break;
			}
			rail_stop_visited[railway_stops[res->stop_name]] = true;	//车站标记为已访问
			string arr_chici = res->represent_number;	//保存到达当前车站的车次

			str_times = split(res->arrival_time, ':');
			//int arr_time = (int)res->g + 60 * atoi(time.c_str()); //到达高铁站的时间
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//保存该车次到达当前车站的时间
			int cur_stop = railway_stops[res->stop_name];	//当前站点编号
			//遍历从当前车站出发能够到达的所有站点
			for (map<int, railway_means>::iterator it = railway_list[cur_stop].begin(); it != railway_list[cur_stop].end(); it++)
			{
				/*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
				double lon_cur = res_rail_lon[it->first];
				double lat_cur = res_rail_lat[it->first];
				double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

				transportation temp;
				if (rail_stop_visited[it->first] == true)	//该目的站点已被访问过
					continue;

				int num = it->second.railnumbers;	//能够到达目的站点的高铁数量
				for (int i = 0; i < num; i++)
				{
					temp = it->second.railways[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//从到站到再次发车间隔的时间

					if (leave_time < arr_time)	//如果在到达之前就发车的话，不考虑
					{
						continue;
					}

					wait_time = leave_time - arr_time;

					if (temp.represent_number != arr_chici) //如果需要换乘车次或交通方式
					{
						if (wait_time < 15) //考虑换乘的实际情况，需要15min换乘时间
							continue;
						temp.change_trans_times += 1; //换车次
					}
					//if(wait_time > )
					temp.g = temp.price + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
					//查询是否已经在堆中
					if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //不在堆中，则加入
					{
						temp.h = h;
						temp.last = res;
						temp.transportation_name = "高铁";
						heap.add(temp);
					}
				}
			}
			//考虑从下一个到达的目的站点换飞机的情况
			int cur_city_flag = res_rail_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //说明该目的高铁站所在城市有地铁信息
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_plane_stops[cur_city_flag].begin(); j != have_plane_stops[cur_city_flag].end(); j++)
					{
						/*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
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
							cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
							cur_res.price = 0; //步行不花钱
							cur_res.g = temp.g + cur_res.price;
							cur_res.h = h;
							cur_res.transportation_name = "步行";
							cur_res.represent_number = "——";
							cur_res.change_trans_times = res->change_trans_times + 1; //换交通方式
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

		if (plane_stops.count(res->stop_name))	//接入飞机网络
		{
			//到达结束站点所在城市，退出搜索
			if (res_plane_city[plane_stops[res->stop_name]] == city[end_city] && railplane_to_sub.count(res->stop_name))
			{
				for (transportation *pathData = res; pathData->last != nullptr; )
				{
					if (pathData->represent_number != pathData->last->represent_number) //说明在该站点换乘过车
					{
						plane_stop_visited[plane_stops[pathData->last->stop_name]] = false;
						break;
					}
					pathData = pathData->last;
				}
				//plane_stop_visited[plane_stops[res->stop_name]] = true;	//车站标记为已访问
				laststop = search_subway_min_price(res, railplane_to_sub[res->stop_name][0], end, end, r_time, city[end_city], end_city);
				laststops.push_back(laststop);
				result_flag++;
				fin_flag = 1;
				//break;
				continue;
			}
			plane_stop_visited[plane_stops[res->stop_name]] = true;	//车站标记为已访问
			string arr_flight_number = res->represent_number;	//保存到达当前机场的航班次
			str_times = split(res->arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());//保存该班次飞机到达当前机场的时间
			int cur_stop = plane_stops[res->stop_name];
			//遍历从当前机场出发能够到达的所有机场
			for (map<int, plane_means>::iterator it = plane_list[cur_stop].begin(); it != plane_list[cur_stop].end(); it++)
			{
				/*获取it代表的目的机场的经纬度lon_cur,lat_cur*/
				double lon_cur = res_plane_lon[it->first];
				double lat_cur = res_plane_lat[it->first];
				double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

				transportation temp;
				if (plane_stop_visited[it->first] == true)	//该目的机场已被访问过
					continue;

				int num = it->second.planenumbers;	//能够到达目的机场的飞机数量
				for (int i = 0; i < num; i++)
				{
					temp = it->second.planes[i];
					temp.change_trans_times = res->change_trans_times;
					str_times = split(temp.starting_time, ':');
					int leave_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
					int wait_time;	//从到站到再次起飞间隔的时间

					if (leave_time < arr_time)	//在到达之前就已发车，不考虑
					{
						continue;
					}

					wait_time = leave_time - arr_time;

					if (temp.represent_number != arr_flight_number) //如果需要换乘
					{
						if (wait_time < 30) //考虑换乘的实际情况，需要30min换乘时间
							continue;
						temp.change_trans_times += 1; //换航班
					}

					temp.g = temp.price + res->g;
					transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
					if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
					{
						if (Inquire->g > temp.g)
						{
							Inquire->g = temp.g;
							Inquire->last = res;
						}
					}
					else //不在堆中，则加入
					{
						temp.h = h;
						temp.last = res;
						heap.add(temp);
					}
				}

			}
			//考虑从下一个到达的目的站点换高铁的情况
			int cur_city_flag = res_plane_city[cur_stop];
			if (cur_city_flag < allow_trans_numbers && railplane_to_sub.count(res->stop_name) && !city_visited[cur_city_flag]) //说明该目的地飞机场所在城市有地铁信息
			{
				for (vector<string>::iterator i = railplane_to_sub[res->stop_name].begin(); i != railplane_to_sub[res->stop_name].end(); i++)
				{
					for (vector<string>::iterator j = have_railway_stops[cur_city_flag].begin(); j != have_railway_stops[cur_city_flag].end(); j++)
					{
						if (railplane_to_sub.count(*j))
							continue;
						/*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
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
							cur_res.spendtimes = 10; //假设从地铁站步行到机场站要 10min
							cur_res.price = 0; //步行不花钱
							cur_res.g = temp.g + cur_res.price;
							cur_res.h = h;
							cur_res.transportation_name = "步行";
							cur_res.represent_number = "——";
							cur_res.change_trans_times = res->change_trans_times + 1; //换交通方式
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
		cout << "您选择的出发站点在您预计出行时间段附近没有车次，请尝试选择其它时间段" << endl;
		return string();
	}
	else if (fin_flag == 0)
	{
		cout << "抱歉，未能查询到合适的规划方案，请尝试不同时间和站点并重试" << endl;
		return string();
	}
	else
	{
		sort(laststops.begin(), laststops.end(), cmp1);
		transportation *pathData = &laststops[0];	//打印路径
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
		ofstream destFile("最省钱路径规划.txt", ios::out);
		res_route = route.top();
		destFile << res_route.transportation_name << endl;	result = result + res_route.transportation_name + "*";
		destFile << res_route.starting_time << endl;	result = result + res_route.starting_time + "*";
		destFile << res_route.represent_number << endl;	result = result + res_route.represent_number + "*";
		destFile << res_route.last->stop_name << endl;	result = result + res_route.last->stop_name + "*";
		cout << endl << res_route.last->stop_name << "——" << res_route.starting_time << "_" << res_route.transportation_name
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
				cout << res_route.arrival_time << "——>" << res_route.stop_name << endl;

				destFile << res_next_route.transportation_name << endl;	result = result + res_next_route.transportation_name + "*";
				destFile << res_next_route.starting_time << endl;	result = result + res_next_route.starting_time + "*";
				destFile << res_next_route.represent_number << endl;	result = result + res_next_route.represent_number + "*";
				destFile << res_next_route.last->stop_name << endl;	result = result + res_next_route.last->stop_name + "*";
				cout << res_next_route.last->stop_name << "——" << res_next_route.starting_time << "_" << res_next_route.transportation_name
					<< "_" << res_next_route.represent_number << "_";
			}
		}
		destFile << res_route.stop_name << endl;	result = result + res_route.stop_name + "*";
		destFile << res_route.arrival_time << endl;		result = result + res_route.arrival_time + "_";
		cout << res_route.arrival_time << "——>" << res_route.stop_name << endl << endl;
		destFile << "——" << endl << "——" << endl << "——" << endl << "——" << endl;
		result = result + "——*——*——*——*";
		destFile << "总票价：" << real_price << endl;
		result = result + "总票价：" + to_string(real_price) + "*";
		destFile << "转车转线总次数：" << change_times;
		result = result + "转车转线总次数：" + to_string(change_times) + "_";
		cout << "总票价" << real_price << endl;
		destFile.close();
		cout << "方案规划成功" << endl;
		return result;
	}
}

transportation search_subway_min_price(transportation * node, string start, string end, string nearname, int time, int cur_city_flag, string end_city)
{
	memset(subway_stop_visited[cur_city_flag], false, sizeof(subway_stop_visited[cur_city_flag]));
	MinHeap heap = MinHeap();
	transportation *res;	//节点指针
	transportation laststop;	//存放最后一个站点信息，用于反向输出路径
	map<int, subway_means>::iterator it;	//用于遍历subway_list
	vector<string> str_times;

	if (node == nullptr)
	{
		res = new transportation(start, 0, 0, nullptr, "起点");
		res->price = 3; //默认地铁票价3元
		res->g = 3;
		res->change_trans_times = 0;
		res->arrival_time = trans_to_str_time(time);
	}
	else
	{
		res = new transportation();
		res->stop_name = start;
		res->spendtimes = 10; //步行10min
		res->price = 0;
		res->last = node;
		res->g = node->g + res->price;
		res->h = 0;
		res->change_trans_times = node->change_trans_times; //换交通方式的次数
		res->transportation_name = "步行";
		res->represent_number = "——";
		res->starting_time = node->arrival_time;
		str_times = split(res->starting_time, ':');
		res->arrival_time = trans_to_str_time(60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + res->spendtimes);
	}
	str_times = split(res->arrival_time, ':');
	int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//将出发的时间换算成整数

	subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][start]] = true;	//起始站点标记为已访问

	//遍历从当前车站出发能够到达的所有站点
	for (it = subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].begin(); it != subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].end(); it++)
	{
		transportation temp;
		int num = it->second.subnumbers;	//经过一个站点的地铁数量
		for (int i = 0; i < num; i++)
		{
			temp = it->second.subways[i];
			str_times = split(temp.starting_time, ':');
			int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//获取地铁出发时间
			str_times = split(temp.arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arr_time < start_time)	//说明首末车次时间地铁跨天
			{
				arr_time += 1440;
			}
			//出发时间在乘坐地铁的运营时间内，视为满足条件
			if (start_time <= r_time && arr_time >= r_time)
			{
				temp.g = temp.price + res->g;
				str_times = split(res->arrival_time, ':');
				double s_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str()) + 2;
				temp.starting_time = trans_to_str_time(s_time);
				double a_time = s_time + temp.spendtimes;
				temp.arrival_time = trans_to_str_time(a_time);
				temp.change_trans_times = res->change_trans_times;
				temp.h = 0;	//地铁站不适用启发函数，退化为Dijkstra算法
				temp.last = res;
				heap.add(temp);
			}
		}
	}
	/*搜索算法执行*/
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
		*res = heap.getAndRemoveMin();	//取出f值最小的点
		if (res->stop_name == end)	//找到结束站点，退出搜索
		{
			laststop = *res;
			finish = true;
			fin_flag = 1;
			break;
		}
		string arr_checi = res->represent_number;	//保存到达当前车站的线路
		subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][res->stop_name]] = true;	//车站标记为已访问
		str_times = split(res->arrival_time, ':');
		int r_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//将出发的时间换算成整数
		int cur_stop = subway_stops[cur_city_flag][res->stop_name];
		//遍历从当前车站出发能够到达的所有站点
		for (map<int, subway_means>::iterator it = subway_list[cur_city_flag][cur_stop].begin(); it != subway_list[cur_city_flag][cur_stop].end(); it++)
		{
			transportation temp;
			if (subway_stop_visited[cur_city_flag][it->first] == true)	//该目的站点已被访问过
				continue;

			int num = it->second.subnumbers;	//能够到达目的站点的高铁数量
			for (int i = 0; i < num; i++)
			{
				temp = it->second.subways[i];
				str_times = split(temp.starting_time, ':');
				int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//获取地铁出发时间
				str_times = split(temp.arrival_time, ':');
				int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arr_time < start_time)	//说明首末车次时间地铁跨天
				{
					arr_time += 1440;
				}
				temp.price = res->price;
				temp.g = temp.price + res->g;
				temp.change_trans_times = res->change_trans_times;
				str_times = split(res->arrival_time, ':');
				int cur_start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (temp.represent_number != arr_checi) //需要换线
				{
					cur_start_time += 3;	//估算地铁换线额外花费的3min
					temp.change_trans_times += 1; //换交通方式
				}
				//res->arrival_time = trans_to_str_time(res->g + time);
				temp.starting_time = trans_to_str_time(cur_start_time);
				temp.arrival_time = trans_to_str_time(cur_start_time + temp.spendtimes);

				transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
				if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
				{
					if (Inquire->g > temp.g)
					{
						Inquire->g = temp.g;
						Inquire->last = res;
					}
				}
				else //不在堆中，则加入
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
	transportation *res;	//节点指针
	transportation laststop;	//存放最后一个站点信息，用于反向输出路径
	map<int, subway_means>::iterator it;	//用于遍历subway_list
	vector<string> str_times;

	if (node == nullptr)
	{
		res = new transportation(start, 0, 0, nullptr, "起点");
		res->price = 3; //默认地铁票价3元
		res->change_trans_times = 0;
	}
	else
	{
		res = new transportation();
		res->stop_name = start;
		res->spendtimes = 10; //步行10min
		res->last = node;
		res->g = node->g + res->spendtimes;
		res->h = 0;
		res->price = 0;
		res->change_trans_times = node->change_trans_times + 1; //换交通方式的次数
		res->transportation_name = "步行";
		res->represent_number = "——";
		double s_time = node->g + time;
		double a_time = res->g + time;
		res->starting_time = trans_to_str_time(s_time);
		res->arrival_time = trans_to_str_time(a_time);
	}

	int r_time = (int)res->g + time;	//将出发的时间换算成整数

	subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][start]] = true;	//起始站点标记为已访问

	//遍历从当前车站出发能够到达的所有站点
	for (it = subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].begin(); it != subway_list[cur_city_flag][subway_stops[cur_city_flag][start]].end(); it++)
	{
		transportation temp;
		int num = it->second.subnumbers;	//经过一个站点的地铁数量
		for (int i = 0; i < num; i++)
		{
			temp = it->second.subways[i];
			str_times = split(temp.starting_time, ':');
			int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//获取地铁出发时间
			str_times = split(temp.arrival_time, ':');
			int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
			if (arr_time < start_time)	//说明首末车次时间地铁跨天
			{
				arr_time += 1440;
			}
			//出发时间在乘坐地铁的运营时间内，视为满足条件
			if (start_time <= r_time && arr_time >= r_time)
			{
				temp.g = temp.spendtimes + res->g + 2; //等地铁花费 2min
				double s_time = res->g + 2 + time;
				temp.starting_time = trans_to_str_time(s_time);
				double a_time = temp.g + time;
				temp.arrival_time = trans_to_str_time(a_time);
				temp.change_trans_times = res->change_trans_times;
				temp.h = 0;	//地铁站不适用启发函数，退化为Dijkstra算法
				temp.last = res;
				heap.add(temp);
			}
		}
	}
	/*搜索算法执行*/
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
		*res = heap.getAndRemoveMin();	//取出f值最小的点
		if (res->stop_name == end)	//找到结束站点，退出搜索
		{
			laststop = *res;
			finish = true;
			fin_flag = 1;
			break;
		}
		string arr_checi = res->represent_number;	//保存到达当前车站的线路
		subway_stop_visited[cur_city_flag][subway_stops[cur_city_flag][res->stop_name]] = true;	//车站标记为已访问
		int r_time = (int)res->g + time; //保存到达当前车站的时间
		int cur_stop = subway_stops[cur_city_flag][res->stop_name];
		//遍历从当前车站出发能够到达的所有站点
		for (map<int, subway_means>::iterator it = subway_list[cur_city_flag][cur_stop].begin(); it != subway_list[cur_city_flag][cur_stop].end(); it++)
		{
			transportation temp;
			if (subway_stop_visited[cur_city_flag][it->first] == true)	//该目的站点已被访问过
				continue;

			int num = it->second.subnumbers;	//能够到达目的站点的高铁数量
			for (int i = 0; i < num; i++)
			{
				temp = it->second.subways[i];
				str_times = split(temp.starting_time, ':');
				int start_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());	//获取地铁出发时间
				str_times = split(temp.arrival_time, ':');
				int arr_time = 60 * atoi(str_times[0].c_str()) + atoi(str_times[1].c_str());
				if (arr_time < start_time)	//说明首末车次时间地铁跨天
				{
					arr_time += 1440;
				}
				temp.g = temp.spendtimes + res->g;
				temp.change_trans_times = res->change_trans_times;
				temp.price = res->price;
				if (temp.represent_number != arr_checi) //需要换线
				{
					temp.g += 3; //估算地铁换线额外花费的3min
					temp.change_trans_times += 1; //换交通方式
				}
				res->arrival_time = trans_to_str_time(res->g + time);
				temp.starting_time = trans_to_str_time(temp.g - temp.spendtimes + time);
				temp.arrival_time = trans_to_str_time(temp.g + time);

				transportation* Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
				if (Inquire != nullptr)	//在堆中，更新g值和上一个站点信息
				{
					if (Inquire->g > temp.g)
					{
						Inquire->g = temp.g;
						Inquire->last = res;
					}
				}
				else //不在堆中，则加入
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
	int num;	//用来存放结果集列数
	string temp[6];	//复制数据的列信息
	int city_flag = 0;	//记录不同城市标号
	//先读取机场信息，获知可的城市
	if (!mysql_query(&ceshi, "SELECT * FROM 飞机场信息表"))   //若查询成功返回0，失败返回随机数
		cout << "\n----飞机场信息表查询成功----" << endl;
	result = mysql_store_result(&ceshi);    //将查询到的结果集储存到result中
	num = mysql_num_fields(result);		//将结果集列数存放到num中
	while ((row = mysql_fetch_row(result)))  //遇到最后一行，则中止循环
	{
		for (int i = 0; i < num; i++)         //输出该行的每一列
			temp[i] = row[i];
		int flag = plane_stops[temp[0]];	//站点编号
		res_plane_lon[flag] = atof(temp[2].c_str());
		res_plane_lat[flag] = atof(temp[1].c_str());
		if (city.insert(map<string, int>::value_type(temp[3], city_flag)).second == true)
		{
			city_flag++;
		}
		res_plane_city[flag] = city[temp[3]];	//保存机场所属城市的编号
		res_city_lon[city[temp[3]]] = atof(temp[5].c_str());	//保存可行城市的经纬度信息
		res_city_lat[city[temp[3]]] = atof(temp[4].c_str());
		have_plane_stops[city[temp[3]]].push_back(temp[0]);	//加入所属城市的机场名表
	}
	allow_trans_numbers = city_flag;	//跟新允许的城市数量
	//cout << allow_trans_numbers << endl;

	if (!mysql_query(&ceshi, "SELECT * FROM 高铁站信息表"))   //若查询成功返回0，失败返回随机数
		cout << "\n----高铁站信息表查询成功----" << endl;
	result = mysql_store_result(&ceshi);    //将查询到的结果集储存到result中
	num = mysql_num_fields(result);		//将结果集列数存放到num中
	while ((row = mysql_fetch_row(result)))  //遇到最后一行，则中止循环
	{
		for (int i = 0; i < num; i++)         //输出该行的每一列
			temp[i] = row[i];
		int flag = railway_stops[temp[0]];
		res_rail_lon[flag] = atof(temp[2].c_str());
		res_rail_lat[flag] = atof(temp[1].c_str());
		if (city.insert(map<string, int>::value_type(temp[3], city_flag)).second == true)
		{
			city_flag++;
		}
		res_rail_city[flag] = city[temp[3]];	//保存车站所属城市的编号
		if (city[temp[3]] < allow_trans_numbers)
			have_railway_stops[city[temp[3]]].push_back(temp[0]);	//加入所属城市的高铁站名表

		//cout << temp[0] << endl;
	}

	cout << "\n站点信息读取成功" << endl;
}

vector<string> split(string strs, char ch)
{
	vector<string> res;
	if (strs == "")
		return res;
	//在字符串末尾也加入分隔符，方便截取最后一段
	strs = strs + ch;
	size_t pos = strs.find(ch);
	string temp;
	while (pos != strs.npos)
	{
		temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(ch);
	}
	return res;
}

double cal_h(double lon1, double lat1, double lon2, double lat2, string trans_means)
{
	// 得到起点经纬度,并转换为角度
	double startLon = (PI / 180) * lon1;
	double startLan = (PI / 180) * lat1;
	// 得到终点经纬度,并转换为角度
	double endLon = (PI / 180) * lon2;
	double endtLan = (PI / 180) * lat2;

	// 地球平均半径为6371km
	double earthR = 6371;

	// 计算公式
	double distence =
		acos(sin(startLan) * sin(endtLan) + cos(startLan) * cos(endtLan) * cos(endLon - startLon)) * earthR;

	if (trans_means == "railway")
		return (distence / 230) * 60; //假设地铁的平均运行速度为230km/h

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
