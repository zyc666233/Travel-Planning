package travel_planning;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.sql.*;
import java.util.*;

public class Test {

    static ResultSet rst;  //创建resultset对象，用来存放查询结果
    static Connection conn; //创建connection对象,用来连接数据库
    static Statement state;	//创建statement对象，用来执行sql语句

    void Init(){
        for(int i = 0; i < 6; i++){
            Data.have_plane_stops[i] = new Vector<String>();
            Data.have_railway_stops[i] = new Vector<String>();
            Data.subway_stops[i] = new HashMap<String, Integer>();
        }

        for(int j = 0; j < 620; j++)
            Data.railway_list[j] = new HashMap<Integer, railway_means>();

        for(int i = 0; i < 100; i++)
            Data.plane_list[i] = new HashMap<Integer, plane_means>();

        for(int i = 0; i < 6; i++)
            for(int j = 0; j < 400; j++)
                Data.subway_list[i][j] = new HashMap<Integer, subway_means>();
    }

    static void create_map(){
        Test in = new Test();
        in.Init();
        String driver = "com.mysql.cj.jdbc.Driver";	//驱动名，默认
        //129.211.3.97 123456
        String url = "jdbc:mysql://129.211.3.97/test?serverTimezone=UTC&useUnicode=true&characterEncoding=UTF-8";	//将要访问的数据库名称test
        String user = "root";	//mysql数据库用户名
        String password = "123456";	//mysql数据库用户密码

        try {
            Class.forName(driver);	//加载驱动
            conn = DriverManager.getConnection(url, user, password);	//创建connection对象,用来连接数据库
            if(!conn.isClosed())
                System.out.println("\n----MySQL已连接----");

            create_railway_map();
            create_plane_map();
            get_lon_and_lat_and_city();
            create_subway_map();

            rst.close();
            state.close();
            conn.close();

        }catch(Exception e) {
            System.out.println("defeat!");
            System.out.println(e);
        }
    }
    static void create_railway_map(){
        String[] temp = new String[50];    //复制数据的列信息
        String[] str1;
        String[] str2;
        String[] str_times;
        int start_time, arrival_time;
        int num;    ////用来存放结果集列数
        int flag;
        String sql;

        /*查询高铁表*/
        try {
            state = conn.createStatement();
            sql = "select * from 高铁;";	//执行的sql语句
            rst = state.executeQuery(sql);	//创建resultset对象，用来存放查询结果
            num = rst.getMetaData().getColumnCount();   //用来存放结果集列数
            flag = 0;    //记录不同的站点的编号
            while(rst.next()) {
                for (int i = 1; i <= num; i++)         //输出该行的每一列
                    temp[i-1] = rst.getString(i);

                str1 = temp[1].split("_");
                if (!Data.railway_stops.containsKey(str1[0]))
                {  //防止重复插入
                    Data.railway_stops.put(str1[0], flag);
                    //find_railway_stops[flag] = str1[0];
                    //cout << str1[0] << " " << flag << endl;
                    //destFile << flag << "," << str1[0] << "站" << endl;
                    flag++;
                }

                for (int j = 2; j < num; j++)
                {
                    if (temp[j].equals("")) break;
                    str1 = temp[j - 1].split("_");    //name_lastarrivaltime_nextstarttime_stoptime
                    str2 = temp[j].split("_");

                    if (!Data.railway_stops.containsKey(str2[0]))    //防止同一站名重复映射
                    {
                        Data.railway_stops.put(str2[0], flag);
                        //find_railway_stops[flag] = str2[0];
                        //cout << str2[0] << " " << flag << endl;
                        //destFile << flag << "," << str2[0] << "站" << endl;
                        flag++;
                    }

                    //str1[0]为出发站，str2[0]为目的站
                    int flag1 = Data.railway_stops.get(str1[0]);
                    int flag2 = Data.railway_stops.get(str2[0]);

                    if (!Data.railway_list[flag1].containsKey(flag2)) {
                        railway_means res = new railway_means();
                        //res.flag = flag2;
                        res.railways[res.railnumbers].represent_number = temp[0];    //车次
                        res.railways[res.railnumbers].starting_time = str1[2];
                        res.railways[res.railnumbers].arrival_time = str2[1];
                        res.railways[res.railnumbers].transportation_name = "高铁";
                        str_times = str1[2].split(":");
                        start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        str_times = str2[1].split(":");
                        arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        if (arrival_time < start_time)    //说明发生跨天情况
                        {
                            res.railways[res.railnumbers].spendtimes = 1440 - start_time + arrival_time;
                        } else //计算时间
                        {
                            res.railways[res.railnumbers].spendtimes = arrival_time - start_time;
                        }

                        if (temp[0].charAt(0) == 'G')    //高铁票价
                            res.railways[res.railnumbers].price = (int) (1.5 * res.railways[res.railnumbers].spendtimes);
                        else if (temp[0].charAt(0) == 'C')    //城际票价
                            res.railways[res.railnumbers].price = (int) (1.75 * res.railways[res.railnumbers].spendtimes);
                        else //动车票价
                            res.railways[res.railnumbers].price = (int) (0.9 * res.railways[res.railnumbers].spendtimes);

                        res.railways[res.railnumbers].stop_name = str2[0];
                        res.railnumbers++;
                        //data.railway_list[flag1].insert(map<int, railway_means>::value_type(flag2, res));
                        Data.railway_list[flag1].put(flag2, res);
                        //cout << times;
                    }

                    else
                    {
                        int railnumbers = Data.railway_list[flag1].get(flag2).railnumbers;
                        Data.railway_list[flag1].get(flag2).railways[railnumbers].represent_number = temp[0];
                        Data.railway_list[flag1].get(flag2).railways[railnumbers].starting_time = str1[2];
                        Data.railway_list[flag1].get(flag2).railways[railnumbers].arrival_time = str2[1];
                        Data.railway_list[flag1].get(flag2).railways[railnumbers].transportation_name = "高铁";
                        str_times = str1[2].split(":");
                        start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        str_times = str2[1].split(":");
                        arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        if (arrival_time < start_time)    //说明发生跨天情况
                        {
                            Data.railway_list[flag1].get(flag2).railways[railnumbers].spendtimes = 1440 - start_time + arrival_time;
                        }
                        else //计算时间
                        {
                            Data.railway_list[flag1].get(flag2).railways[railnumbers].spendtimes = arrival_time - start_time;
                        }

                        if (temp[0].charAt(0) == 'G')    //高铁票价
                            Data.railway_list[flag1].get(flag2).railways[railnumbers].price = (int) (1.5 * Data.railway_list[flag1].get(flag2).railways[railnumbers].spendtimes);
                        else if (temp[0].charAt(0) == 'C')    //城际票价
                            Data.railway_list[flag1].get(flag2).railways[railnumbers].price = (int) (1.75 * Data.railway_list[flag1].get(flag2).railways[railnumbers].spendtimes);
                        else  //动车票价
                            Data.railway_list[flag1].get(flag2).railways[railnumbers].price = (int) (0.9 * Data.railway_list[flag1].get(flag2).railways[railnumbers].spendtimes);

                        Data.railway_list[flag1].get(flag2).railways[railnumbers].stop_name = str2[0];
                        Data.railway_list[flag1].get(flag2).railnumbers++;
                    }
                }
            }
            System.out.println("\n高铁表构建成功");

        }catch(Exception e) {
            System.out.println("defeat!");
            System.out.println(e);
        }

    }
    static void create_plane_map(){
        String[] temp = new String[50];    //复制数据的列信息
        String[] str1;
        String[] str2;
        String[] str_times;
        int start_time, arrival_time;
        int num;    ////用来存放结果集列数
        int flag;
        String sql;

        /*查询飞机表*/
        try{
            sql = "select * from 飞机;";	//执行的sql语句
            rst = state.executeQuery(sql);	//创建resultset对象，用来存放查询结果
            num = rst.getMetaData().getColumnCount();   //用来存放结果集列数
            flag = 0;    //记录不同的站点的编号
            while(rst.next()) {
                for (int i = 1; i <= num; i++)         //输出该行的每一列
                    temp[i-1] = rst.getString(i);

                if (!Data.plane_stops.containsKey(temp[1]))
                {  //防止重复插入
                    Data.plane_stops.put(temp[1], flag);
                    //find_railway_stops[flag] = str1[0];
                    //cout << str1[0] << " " << flag << endl;
                    //destFile << flag << "," << str1[0] << "站" << endl;
                    flag++;
                }
                if (!Data.plane_stops.containsKey(temp[3]))
                {  //防止重复插入
                    Data.plane_stops.put(temp[3], flag);
                    //find_railway_stops[flag] = str1[0];
                    //cout << str1[0] << " " << flag << endl;
                    //destFile << flag << "," << str1[0] << "站" << endl;
                    flag++;
                }

                //temp[1]为出发站，temp[3]为目的站
                int flag1 = Data.plane_stops.get(temp[1]);
                int flag2 = Data.plane_stops.get(temp[3]);

                if (!Data.plane_list[flag1].containsKey(flag2)) {
                    plane_means res = new plane_means();
                    //res.flag = flag2;
                    res.planes[res.planenumbers].represent_number = temp[0];    //车次
                    res.planes[res.planenumbers].starting_time = temp[2];
                    res.planes[res.planenumbers].stop_name = temp[3];
                    res.planes[res.planenumbers].arrival_time = temp[4];
                    res.planes[res.planenumbers].transportation_name = "飞机";
                    str_times = temp[2].split(":");
                    start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    str_times = temp[4].split(":");
                    arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    if (arrival_time < start_time)    //说明发生跨天情况
                    {
                        res.planes[res.planenumbers].spendtimes = 1440 - start_time + arrival_time;
                    } else //计算时间
                    {
                        res.planes[res.planenumbers].spendtimes = arrival_time - start_time;
                    }

                    res.planes[res.planenumbers].price = Integer.parseInt(temp[5]);
                    res.planenumbers++;
                    //data.railway_list[flag1].insert(map<int, railway_means>::value_type(flag2, res));
                    Data.plane_list[flag1].put(flag2, res);
                    //cout << times;
                }

                else
                {
                    int planenumbers = Data.plane_list[flag1].get(flag2).planenumbers;
                    Data.plane_list[flag1].get(flag2).planes[planenumbers].represent_number = temp[0];
                    Data.plane_list[flag1].get(flag2).planes[planenumbers].starting_time = temp[2];
                    Data.plane_list[flag1].get(flag2).planes[planenumbers].stop_name = temp[3];
                    Data.plane_list[flag1].get(flag2).planes[planenumbers].arrival_time = temp[4];
                    Data.plane_list[flag1].get(flag2).planes[planenumbers].transportation_name = "飞机";
                    str_times = temp[2].split(":");
                    start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    str_times = temp[4].split(":");
                    arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    if (arrival_time < start_time)    //说明发生跨天情况
                    {
                        Data.plane_list[flag1].get(flag2).planes[planenumbers].spendtimes = 1440 - start_time + arrival_time;
                    }
                    else //计算时间
                    {
                        Data.plane_list[flag1].get(flag2).planes[planenumbers].spendtimes = arrival_time - start_time;
                    }


                    Data.plane_list[flag1].get(flag2).planes[planenumbers].price = Integer.parseInt(temp[5]);
                    Data.plane_list[flag1].get(flag2).planenumbers++;
                }
            }
            System.out.println("\n飞机表构建成功");
        }catch (Exception e){
            System.out.println("defeat!");
            System.out.println(e);
        }

    }
    static void create_subway_map(){
        String[] temp = new String[50];    //复制数据的列信息
        String[] str1;
        String[] str2;
        String[] str_times;
        int start_time, arrival_time;
        int num;    ////用来存放结果集列数
        int flag;
        String sql;

        /*查询地铁表*/
        try{
            int allow_city = 0;
            String[] str3;
            for(String k : Data.city.keySet()){
                int city_flag = Data.city.get(k);
                if(city_flag < Data.allow_trans_numbers){
                    allow_city++;
                    sql = "SELECT * FROM " + k + ";";
                    rst = state.executeQuery(sql);	//创建resultset对象，用来存放查询结果
                    num = rst.getMetaData().getColumnCount();   //用来存放结果集列数
                    flag = 0;    //记录不同的站点的编号
                    while(rst.next()) {
                        for (int i = 1; i <= num; i++)         //输出该行的每一列
                            temp[i-1] = rst.getString(i);

                        str1 = temp[1].split("_");
                        if(str1[0].contains("*")){
                            str3 = str1[0].split("\\*");
                            for(int i = 1; i < str3.length; i++){
                                if(!Data.railplane_to_sub.containsKey(str3[i]))
                                    Data.railplane_to_sub.put(str3[i], new Vector<String>());
                                if(!Data.railplane_to_sub.get(str3[i]).contains(str3[0])){
                                    Data.railplane_to_sub.get(str3[i]).add(str3[0]);
                                }
                            }
                            str1[0] = str3[0];
                        }

                        if (!Data.subway_stops[city_flag].containsKey(str1[0]))
                        {  //防止重复插入
                            Data.subway_stops[city_flag].put(str1[0], flag);
                            //find_railway_stops[flag] = str1[0];
                            //cout << str1[0] << " " << flag << endl;
                            //destFile << flag << "," << str1[0] << "站" << endl;
                            flag++;
                        }

                        for (int j = 2; j < num; j++)
                        {
                            if (temp[j].equals("")) break;
                            str1 = temp[j - 1].split("_");    //name_lastarrivaltime_nextstarttime_stoptime
                            if(str1[0].contains("*")){
                                str3 = str1[0].split("\\*");
                                str1[0] = str3[0];
                            }
                            str2 = temp[j].split("_");
                            if(str2[0].contains("*")){
                                str3 = str2[0].split("\\*");
                                for(int i = 1; i < str3.length; i++){
                                    if(!Data.railplane_to_sub.containsKey(str3[i]))
                                        Data.railplane_to_sub.put(str3[i], new Vector<String>());
                                    if(!Data.railplane_to_sub.get(str3[i]).contains(str3[0])){
                                        Data.railplane_to_sub.get(str3[i]).add(str3[0]);
                                    }
                                }
                                str2[0] = str3[0];
                            }

                            if (!Data.subway_stops[city_flag].containsKey(str2[0]))    //防止同一站名重复映射
                            {
                                Data.subway_stops[city_flag].put(str2[0], flag);
                                //find_railway_stops[flag] = str2[0];
                                //cout << str2[0] << " " << flag << endl;
                                //destFile << flag << "," << str2[0] << "站" << endl;
                                flag++;
                            }

                            //str1[0]为出发站，str2[0]为目的站
                            int flag1 = Data.subway_stops[city_flag].get(str1[0]);
                            int flag2 = Data.subway_stops[city_flag].get(str2[0]);

                            if (!Data.subway_list[city_flag][flag1].containsKey(flag2)) {
                                subway_means res = new subway_means();
                                //res.flag = flag2;
                                res.subways[res.subnumbers].represent_number = temp[0];    //车次
                                res.subways[res.subnumbers].starting_time = str1[1];
                                res.subways[res.subnumbers].arrival_time = str1[2];
                                res.subways[res.subnumbers].transportation_name = "地铁";
                                str_times = str1[1].split(":");
                                start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                                str_times = str2[1].split(":");
                                arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                                if (arrival_time < start_time)    //说明发生跨天情况
                                {
                                    res.subways[res.subnumbers].spendtimes = 3;
                                } else //计算时间
                                {
                                    res.subways[res.subnumbers].spendtimes = arrival_time - start_time;
                                }
                                res.subways[res.subnumbers].stop_name = str2[0];
                                res.subways[res.subnumbers].price = 0;
                                res.subnumbers++;
                                //data.railway_list[flag1].insert(map<int, railway_means>::value_type(flag2, res));
                                Data.subway_list[city_flag][flag1].put(flag2, res);
                                //cout << times;
                            }

                            else
                            {
                                int railnumbers = Data.subway_list[city_flag][flag1].get(flag2).subnumbers;
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].represent_number = temp[0];
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].starting_time = str1[1];
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].arrival_time = str1[2];
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].transportation_name = "地铁";
                                str_times = str1[1].split(":");
                                start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                                str_times = str2[1].split(":");
                                arrival_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                                if (arrival_time < start_time)    //说明发生跨天情况
                                {
                                    Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].spendtimes = 3;
                                }
                                else //计算时间
                                {
                                    Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].spendtimes = arrival_time - start_time;
                                }
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].stop_name = str2[0];
                                Data.subway_list[city_flag][flag1].get(flag2).subways[railnumbers].price = 0;
                                Data.subway_list[city_flag][flag1].get(flag2).subnumbers++;
                            }
                        }
                    }
                    System.out.println("\n" + k + "地表构建成功");
                }
            }
            System.out.println("\n地表构建成功");
        }catch (Exception e){
            System.out.println("defeat!");
            System.out.println(e);
        }
    }
    static void get_lon_and_lat_and_city(){
        int num;	//用来存放结果集列数
        String[] temp = new String[6];	//复制数据的列信息
        int city_flag = 0;	//记录不同城市标号

        //先读取机场信息，获知可的城市
        try {
            state = conn.createStatement();
            String sql = "select * from 飞机场信息表;";    //执行的sql语句
            rst = state.executeQuery(sql);    //创建resultset对象，用来存放查询结果
            num = rst.getMetaData().getColumnCount();   //用来存放结果集列数
            while (rst.next()) {
                for (int i = 1; i <= num; i++)         //输出该行的每一列
                    temp[i - 1] = rst.getString(i);

                int flag = Data.plane_stops.get(temp[0]);	//站点编号
                Data.res_plane_lon[flag] = Double.parseDouble(temp[2]);
                Data.res_plane_lat[flag] = Double.parseDouble(temp[1]);;
                if (!Data.city.containsKey(temp[3])) {
                    Data.city.put(temp[3], city_flag);
                    city_flag++;
                }
                Data.res_plane_city[flag] = Data.city.get(temp[3]);	//保存机场所属城市的编号
                Data.res_city_lon[Data.city.get(temp[3])] = Double.parseDouble(temp[5]);	//保存可行城市的经纬度信息
                Data.res_city_lat[Data.city.get(temp[3])] = Double.parseDouble(temp[4]);
                Data.have_plane_stops[Data.city.get(temp[3])].addElement(temp[0]);	//加入所属城市的机场名表
            }
            System.out.println("\n飞机场信息读取成功");
        }catch (Exception e){
            System.out.println("defeat!");
            System.out.println(e);
        }

        Data.allow_trans_numbers = city_flag;	//跟新允许的城市数量

        try {
            state = conn.createStatement();
            String sql = "select * from 高铁站信息表;";    //执行的sql语句
            rst = state.executeQuery(sql);    //创建resultset对象，用来存放查询结果
            num = rst.getMetaData().getColumnCount();   //用来存放结果集列数
            while (rst.next()) {
                for (int i = 1; i <= num; i++)         //输出该行的每一列
                    temp[i - 1] = rst.getString(i);

                int flag = Data.railway_stops.get(temp[0]);	//站点编号
                Data.res_rail_lon[flag] = Double.parseDouble(temp[2]);
                Data.res_rail_lat[flag] = Double.parseDouble(temp[1]);;
                if (!Data.city.containsKey(temp[3])) {
                    Data.city.put(temp[3], city_flag);
                    city_flag++;
                }
                Data.res_rail_city[flag] = Data.city.get(temp[3]);	//保存车站所属城市的编号
                if (Data.city.get(temp[3]) < Data.allow_trans_numbers)
                    Data.have_railway_stops[Data.city.get(temp[3])].addElement(temp[0]);	//加入所属城市的高铁站名表

                //cout << temp[0] << endl;
            }
            System.out.println("\n高铁站信息读取成功");
        }catch (Exception e){
            System.out.println("defeat!");
            System.out.println(e);
        }
    }

    static String[] search_min(String start, String start_city, String end, String end_city, String time, int plan){
        if(plan == 1)
            return search_min_price(start, start_city, end, end_city, time, plan);
        else
            return search_min_time_or_trouble(start, start_city, end, end_city, time, plan);
    }
    static String[] search_min_time_or_trouble(String start, String start_city, String end, String end_city, String time, int plan){
        Arrays.fill(Data.rail_stop_visited, false);
        Arrays.fill(Data.city_visited, false);
        Arrays.fill(Data.plane_stop_visited, false);
        for(int i = 0; i < 6; i++)
            Arrays.fill(Data.subway_stop_visited[i], false);
        Vector<transportation> laststops = new Vector<transportation>(); //存放结果
        MinHeap heap = new MinHeap();
        transportation res;	//节点指针
        transportation laststop = new transportation();	//存放最后一个站点信息，用于反向输出路径
        //map<int, railway_means>::iterator it;	//用于遍历railway_list
        String[] str_times;
        str_times = time.split(":");
        int r_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//将预计出发时间换算成整数
        /*获取最终站点end所在城市的经纬度lon_end, lat_end*/
        double lon_end = Data.res_city_lon[Data.city.get(end_city)];
        double lat_end = Data.res_city_lat[Data.city.get(end_city)];
        res = new transportation();

        int city_flag = Data.city.get(start_city);
        Vector<String> subname = new Vector<String>();
        Iterator<String> it = Data.have_railway_stops[city_flag].iterator();
        while (it.hasNext()) {
            /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
            String cu_it = it.next();
            double lon_cur = Data.res_rail_lon[Data.railway_stops.get(cu_it)];
            double lat_cur = Data.res_rail_lat[Data.railway_stops.get(cu_it)];
            double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
            if (!Data.railplane_to_sub.containsKey(cu_it))
                continue;
            subname = Data.railplane_to_sub.get(cu_it); //和高铁站最近的地铁站列表
            Iterator<String> t = subname.iterator();
            while (t.hasNext()){
                String cu_t = t.next();
                Arrays.fill(Data.subway_stop_visited[city_flag], false);
                transportation temp = search_subway_min_time(null, start, cu_t, r_time, city_flag);
                res = new transportation(temp);
                transportation cur_res = new transportation();
                cur_res.stop_name = cu_it;
                cur_res.spendtimes = 10; //假设从地铁站步行到高铁站要 10min
                cur_res.price = 0; //步行不花钱
                cur_res.g = temp.g + cur_res.spendtimes;
                cur_res.h = h;
                cur_res.transportation_name = "步行";
                cur_res.represent_number = "——";
                cur_res.change_trans_times = res.change_trans_times; //换交通方式
                cur_res.starting_time = trans_to_str_time(temp.g + r_time);
                cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
                cur_res.last = res;
                heap.add(cur_res);
            }
        }

        it = Data.have_plane_stops[city_flag].iterator();
        while (it.hasNext()) {
            /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
            String cu_it = it.next();
            double lon_cur = Data.res_rail_lon[Data.plane_stops.get(cu_it)];
            double lat_cur = Data.res_rail_lat[Data.plane_stops.get(cu_it)];
            double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
            if (!Data.railplane_to_sub.containsKey(cu_it))
                continue;
            subname = Data.railplane_to_sub.get(cu_it);	//飞机场最靠近的地铁站名列表
            Iterator<String> t = subname.iterator();
            while (t.hasNext()) {
                String cu_t = t.next();
                Arrays.fill(Data.subway_stop_visited[city_flag], false);
                transportation temp = search_subway_min_time(null, start, cu_t, r_time, city_flag);
                res = new transportation(temp);
                transportation cur_res = new transportation();
                cur_res.stop_name = cu_it;
                cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
                cur_res.price = 0; //步行不花钱
                cur_res.g = temp.g + cur_res.spendtimes;
                cur_res.h = h;
                cur_res.transportation_name = "步行";
                cur_res.represent_number = "——";
                cur_res.change_trans_times = res.change_trans_times; //换交通方式
                cur_res.starting_time = trans_to_str_time(temp.g + r_time);
                cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
                cur_res.last = res;
                heap.add(cur_res);
            }
        }
        Data.city_visited[city_flag] = true; //起始城市已访问

        /*A*搜索算法执行*/
        boolean finish = false;
        int fin_flag = 0;
        if (heap.isEmpty()) {
            finish = true;
            fin_flag = -1;
        }

        int result_flag = 0; //记录搜索到的可行路径数量，用于最舒适计算
        while (!finish && !heap.isEmpty() && result_flag < 2) {
            res = new transportation(heap.getAndRemoveMin());   //取出f值最小的点

            if (Data.railway_stops.containsKey(res.stop_name))	{   //接入高铁网络
                //到达结束站点所在城市，退出搜索
                if (Data.res_rail_city[Data.railway_stops.get(res.stop_name)] == Data.city.get(end_city) && Data.railplane_to_sub.containsKey(res.stop_name))
                {
                    for (transportation pathData = res; pathData.last != null; )
                    {
                        if (!pathData.represent_number.equals(pathData.last.represent_number)) //说明在该站点换乘过车
                        {
                            Data.rail_stop_visited[Data.railway_stops.get(pathData.last.stop_name)] = false;
                            Data.rail_stop_visited[Data.railway_stops.get(pathData.stop_name)] = true;
                            break;
                        }
                        pathData = pathData.last;
                    }
                    //rail_stop_visited[railway_stops[res.stop_name]] = true;	//车站标记为已访问
                    laststop = search_subway_min_time(res, Data.railplane_to_sub.get(res.stop_name).get(0), end, r_time, Data.city.get(end_city));
                    laststops.addElement(laststop);
                    result_flag++;
                    fin_flag = 1;
                    continue;
                    //break;
                }
                Data.rail_stop_visited[Data.railway_stops.get(res.stop_name)] = true;	//车站标记为已访问
                String arr_chici = res.represent_number;	//保存到达当前车站的车次
                str_times = res.arrival_time.split(":");
                //int arr_time = (int)res.g + 60 * Integer.parseInt(time); //到达高铁站的时间
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);//保存该车次到达当前车站的时间
                int cur_stop = Data.railway_stops.get(res.stop_name);	//当前站点编号
                //遍历从当前车站出发能够到达的所有站点
                for (Map.Entry<Integer, railway_means> entry : Data.railway_list[cur_stop].entrySet())
                {
                    int key = entry.getKey();
                    railway_means value = entry.getValue();
                    /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
                    double lon_cur = Data.res_rail_lon[key];
                    double lat_cur = Data.res_rail_lat[key];
                    double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

                    transportation temp;
                    if (Data.rail_stop_visited[key])	//该目的站点已被访问过
                        continue;

                    int num = value.railnumbers;	//能够到达目的站点的高铁数量
                    for (int i = 0; i < num; i++)
                    {
                        temp = new transportation(value.railways[i]);
                        temp.change_trans_times = res.change_trans_times;
                        str_times = temp.starting_time.split(":");
                        int leave_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        int wait_time;	//从到站到再次发车间隔的时间

                        if (leave_time < arr_time)	//如果出现跨天的情况
                        {
                            wait_time = 1440 - arr_time + leave_time;
                        }
                        else //获取新乘坐高铁的出发时间和到站时间的差值
                        {
                            wait_time = leave_time - arr_time;
                        }

                        if (!temp.represent_number.equals(arr_chici)) //如果需要换乘车次或交通方式
                        {
                            if (wait_time < 15) //考虑换乘的实际情况，需要15min换乘时间
                                continue;
                            temp.change_trans_times += 1; //换车次
                        }
                        //if(wait_time > )
                        temp.g = temp.spendtimes + wait_time + res.g;
                        transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
                        //查询是否已经在堆中
                        if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                        {
                            if (Inquire.g > temp.g)
                            {
                                Inquire.g = temp.g;
                                Inquire.last = res;
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
                int cur_city_flag = Data.res_rail_city[cur_stop];
                if (cur_city_flag < Data.allow_trans_numbers && Data.railplane_to_sub.containsKey(res.stop_name) && !Data.city_visited[cur_city_flag]) //说明该目的高铁站所在城市有地铁信息
                {
                    Iterator<String> i = Data.railplane_to_sub.get(res.stop_name).iterator();
                    while (i.hasNext())
                    {
                        String cu_i = i.next();
                        Iterator<String> j = Data.have_plane_stops[cur_city_flag].iterator();
                        while (j.hasNext())
                        {
                            String cu_j = j.next();
                            /*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
                            double lon_cur = Data.res_plane_lon[Data.plane_stops.get(cu_j)];
                            double lat_cur = Data.res_plane_lat[Data.plane_stops.get(cu_j)];
                            double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
                            if (Data.railplane_to_sub.containsKey(cu_j))
                                continue;
                            subname = Data.railplane_to_sub.get(cu_j);
                            Iterator<String> t = subname.iterator();
                            while (t.hasNext())
                            {
                                String cu_t = t.next();
                                Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
                                transportation temp = search_subway_min_time(res, cu_i, cu_t, r_time, cur_city_flag);
                                res = new transportation(temp);
                                transportation cur_res = new transportation();
                                cur_res.stop_name = cu_j;
                                cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
                                cur_res.price = 0; //步行不花钱
                                cur_res.g = temp.g + cur_res.spendtimes;
                                cur_res.h = h;
                                cur_res.transportation_name = "步行";
                                cur_res.represent_number = "——";
                                cur_res.change_trans_times = res.change_trans_times + 1; //换交通方式
                                cur_res.starting_time = trans_to_str_time(temp.g + r_time);
                                cur_res.arrival_time = trans_to_str_time(cur_res.g + r_time);
                                cur_res.last = res;
                                heap.add(cur_res);
                            }
                        }
                    }
                }
            }

            if (Data.plane_stops.containsKey(res.stop_name))	//接入飞机网络
            {
                //到达结束站点所在城市，退出搜索
                if (Data.res_plane_city[Data.plane_stops.get(res.stop_name)] == Data.city.get(end_city) && Data.railplane_to_sub.containsKey(res.stop_name))
                {
                    for (transportation pathData = res; pathData.last != null; )
                    {
                        if (!pathData.represent_number.equals(pathData.last.represent_number)) //说明在该站点换乘过车
                        {
                            Data.plane_stop_visited[Data.plane_stops.get(pathData.last.stop_name)] = false;
                            break;
                        }
                        pathData = pathData.last;
                    }
                    //plane_stop_visited[plane_stops[res.stop_name]] = true;	//车站标记为已访问
                    laststop = search_subway_min_time(res, Data.railplane_to_sub.get(res.stop_name).get(0), end, r_time, Data.city.get(end_city));
                    laststops.addElement(laststop);
                    result_flag++;
                    fin_flag = 1;
                    //break;
                    continue;
                }
                Data.plane_stop_visited[Data.plane_stops.get(res.stop_name)] = true;	//车站标记为已访问
                String arr_flight_number = res.represent_number;	//保存到达当前机场的航班次
                str_times = res.arrival_time.split(":");
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);//保存该班次飞机到达当前机场的时间
                int cur_stop = Data.plane_stops.get(res.stop_name);
                //遍历从当前机场出发能够到达的所有机场
                for (Map.Entry<Integer, plane_means> entry : Data.plane_list[cur_stop].entrySet())
                {
                    int key = entry.getKey();
                    plane_means value = entry.getValue();
                    /*获取it代表的目的机场的经纬度lon_cur,lat_cur*/
                    double lon_cur = Data.res_plane_lon[key];
                    double lat_cur = Data.res_plane_lat[key];
                    double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

                    transportation temp;
                    if (Data.plane_stop_visited[key])	//该目的机场已被访问过
                        continue;

                    int num = value.planenumbers;	//能够到达目的机场的飞机数量
                    for (int i = 0; i < num; i++)
                    {
                        temp = new transportation(value.planes[i]);
                        temp.change_trans_times = res.change_trans_times;
                        str_times = temp.starting_time.split(":");
                        int leave_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        int wait_time;	//从到站到再次起飞间隔的时间

                        if (leave_time < arr_time)	//如果出现跨天的情况
                        {
                            wait_time = 1440 - arr_time + leave_time;
                        }
                        else //获取新乘坐高铁的出发时间和到站时间的差值
                        {
                            wait_time = leave_time - arr_time;
                        }

                        if (!temp.represent_number.equals(arr_flight_number)) //如果需要换乘
                        {
                            if (wait_time < 30) //考虑换乘的实际情况，需要30min换乘时间
                                continue;
                            temp.change_trans_times += 1; //换航班
                        }

                        temp.g = temp.spendtimes + wait_time + res.g;
                        transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
                        if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                        {
                            if (Inquire.g > temp.g)
                            {
                                Inquire.g = temp.g;
                                Inquire.last = res;
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
                int cur_city_flag = Data.res_plane_city[cur_stop];
                if (cur_city_flag < Data.allow_trans_numbers && Data.railplane_to_sub.containsKey(res.stop_name) && !Data.city_visited[cur_city_flag]) //说明该目的地飞机场所在城市有地铁信息
                {
                    Iterator<String> i = Data.railplane_to_sub.get(res.stop_name).iterator();
                    while (i.hasNext())
                    {
                        String cu_i = i.next();
                        Iterator<String> j = Data.have_railway_stops[cur_city_flag].iterator();
                        while (j.hasNext())
                        {
                            String cu_j = j.next();
                            if (Data.railplane_to_sub.containsKey(cu_j))
                                continue;
                            /*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
                            double lon_cur = Data.res_rail_lon[Data.railway_stops.get(cu_j)];
                            double lat_cur = Data.res_rail_lat[Data.railway_stops.get(cu_j)];
                            double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
                            subname = Data.railplane_to_sub.get(cu_j);
                            Iterator<String> t = subname.iterator();
                            while (t.hasNext())
                            {
                                String cu_t = t.next();
                                Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
                                transportation temp = search_subway_min_time(res, cu_i, cu_t, r_time, cur_city_flag);
                                res = new transportation(temp);
                                transportation cur_res = new transportation();
                                cur_res.stop_name = cu_j;
                                cur_res.spendtimes = 10; //假设从地铁站步行到机场站要 10min
                                cur_res.price = 0; //步行不花钱
                                cur_res.g = temp.g + cur_res.spendtimes;
                                cur_res.h = h;
                                cur_res.transportation_name = "步行";
                                cur_res.represent_number = "——";
                                cur_res.change_trans_times = res.change_trans_times + 1; //换交通方式
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

        String[] result = new String[200];
        if (fin_flag == -1)
        {
            System.out.println("您选择的出发站点在您预计出行时间段附近没有车次，请尝试选择其它时间段");
        }
        else if (fin_flag == 0)
        {
            System.out.println("抱歉，未能查询到合适的规划方案，请尝试不同时间和站点并重试");
        }
        else
        {
            int line = 0;
            if (plan == 0) {
                Collections.sort(laststops, new Comparator<transportation>() {
                    @Override
                    public int compare(transportation o1, transportation o2) {
                        if (o1.g < o2.g)
                            return -1;
                        else if (o1.g == o2.g)
                            return 0;
                        return 1;
                    }
                });

                try {
                    //File destFile = new File("最省时路径规划.txt");
                    //FileWriter writer = new FileWriter(destFile);
                    //BufferedWriter out = new BufferedWriter(writer);

                    transportation pathData = new transportation(laststops.get(0));    //打印路径
                    transportation res_route = new transportation();
                    transportation res_next_route = new transportation();
                    double real_spend_time = pathData.g;
                    int change_times = pathData.change_trans_times;
                    Stack<transportation> route = new Stack<transportation>();
                    int fl = 0;
                    String cur_represent_number = "";
                    for (; pathData.last != null; ) {
                        route.push(pathData);
                        pathData = pathData.last;
                    }

                    res_route = route.peek();

                    //out.write(res_route.transportation_name + "\n");
                    //out.write(res_route.starting_time + "\n");
                    //out.write(res_route.represent_number + "\n");
                    //out.write(res_route.last.stop_name + "\n");

                    result[line++] = res_route.transportation_name;
                    result[line++] = res_route.starting_time;
                    if(res_route.represent_number.contains("线")){
                        String str = res_route.represent_number.split("线",2)[0]+"线";
                        result[line++] = str;
                    }
                    else
                        result[line++] = res_next_route.represent_number;
                    //result[line++] = res_route.represent_number;
                    result[line++] = res_route.last.stop_name;

                    while (!route.empty()) {
                        res_route = route.peek();
                        route.pop();
                        if (route.empty()) break;
                        res_next_route = route.peek();
                        if (!res_route.represent_number.equals(res_next_route.represent_number)) {
                            //out.write(res_route.stop_name + "\n");
                            //out.write(res_route.arrival_time + "\n");
                            result[line++] = res_route.stop_name;
                            result[line++] = res_route.arrival_time;

                            //out.write(res_next_route.transportation_name + "\n");
                            //out.write(res_next_route.starting_time + "\n");
                            //out.write(res_next_route.represent_number + "\n");
                            //out.write(res_next_route.last.stop_name + "\n");
                            result[line++] = res_next_route.transportation_name;
                            result[line++] = res_next_route.starting_time;
                            if(res_next_route.represent_number.contains("线")){
                                String str = res_next_route.represent_number.split("线",2)[0]+"线";
                                result[line++] = str;
                            }
                            else
                                result[line++] = res_next_route.represent_number;
                            result[line++] = res_next_route.last.stop_name;
                            //out.close();
                        }
                    }

                    //out.write(res_route.stop_name + "\n");
                    //out.write(res_route.arrival_time + "\n");
                    result[line++] = res_route.stop_name;
                    result[line++] = res_route.arrival_time;

                    //out.write("——\n——\n——\n——\n");
                    //out.write("总花费时间：" + trans_to_str_time(real_spend_time) + "\n");
                    //out.write("转车转线总次数：" + change_times + "\n");
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "总花费时间：" + trans_to_str_time(real_spend_time);
                    result[line++] = "转车转线总次数：" + Integer.toString(change_times);
                    //out.close();

                    System.out.println(trans_to_str_time(real_spend_time));
                    System.out.println("方案规划成功");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

            if (plan == 2)
            {
                Collections.sort(laststops, new Comparator<transportation>() {
                    @Override
                    public int compare(transportation o1, transportation o2) {
                        if (o1.change_trans_times == o2.change_trans_times) {
                            return (int) o1.g - (int) o2.g;
                        }
                        return o1.change_trans_times - o2.change_trans_times;
                    }
                });

                try{
                    //File destFile = new File("最舒适路径规划.txt");
                    //FileWriter writer = new FileWriter(destFile);
                    //BufferedWriter out = new BufferedWriter(writer);

                    transportation pathData = new transportation(laststops.get(0));	//打印路径
                    transportation res_route = new transportation();
                    transportation res_next_route = new transportation();
                    double real_spend_time = pathData.g;
                    int change_times = pathData.change_trans_times;
                    Stack<transportation> route = new Stack<transportation>();
                    int fl = 0;
                    String cur_represent_number = "";
                    for (;pathData.last != null; )
                    {
                        route.push(pathData);
                        pathData = pathData.last;
                    }
                    res_route = route.peek();
                    //out.write(res_route.transportation_name + "\n");
                    //out.write(res_route.starting_time + "\n");
                    //out.write(res_route.represent_number + "\n");
                    //out.write(res_route.last.stop_name + "\n");

                    result[line++] = res_route.transportation_name;
                    result[line++] = res_route.starting_time;
                    if(res_route.represent_number.contains("线")){
                        String str = res_route.represent_number.split("线",2)[0]+"线";
                        result[line++] = str;
                    }
                    else
                        result[line++] = res_route.represent_number;
                    result[line++] = res_route.last.stop_name;
                    while (!route.empty())
                    {
                        res_route = route.peek();
                        route.pop();
                        if (route.empty())	break;
                        res_next_route = route.peek();
                        if (!res_route.represent_number.equals(res_next_route.represent_number))
                        {
                            //out.write(res_route.stop_name + "\n");
                            //out.write(res_route.arrival_time + "\n");
                            result[line++] = res_route.stop_name;
                            result[line++] = res_route.arrival_time;

                            //out.write(res_next_route.transportation_name + "\n");
                            //out.write(res_next_route.starting_time + "\n");
                            //out.write(res_next_route.represent_number + "\n");
                            //out.write(res_next_route.last.stop_name + "\n");
                            result[line++] = res_next_route.transportation_name;
                            result[line++] = res_next_route.starting_time;
                            if(res_next_route.represent_number.contains("线")){
                                String str = res_next_route.represent_number.split("线",2)[0]+"线";
                                result[line++] = str;
                            }
                            else
                                result[line++] = res_next_route.represent_number;
                            result[line++] = res_next_route.last.stop_name;
                        }
                    }

                    //out.write(res_route.stop_name + "\n");
                    //out.write(res_route.arrival_time + "\n");
                    result[line++] = res_route.stop_name;
                    result[line++] = res_route.arrival_time;

                    //out.write("——\n——\n——\n——\n");
                    //out.write("总花费时间：" + trans_to_str_time(real_spend_time) + "\n");
                    //out.write("转车转线总次数：" + change_times + "\n");
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "——";
                    result[line++] = "总花费时间：" + trans_to_str_time(real_spend_time);
                    result[line++] = "转车转线总次数：" + Integer.toString(change_times);
                    //out.close();

                    System.out.println(trans_to_str_time(real_spend_time));
                    System.out.println("方案规划成功");
                }catch(Exception e){
                    e.printStackTrace();
                }
            }
        }
        return result;
    }
    static String[] search_min_price(String start, String start_city, String end, String end_city, String time, int plan){
        Arrays.fill(Data.rail_stop_visited, false);
        Arrays.fill(Data.city_visited, false);
        Arrays.fill(Data.plane_stop_visited, false);
        for(int i = 0; i < 6; i++)
            Arrays.fill(Data.subway_stop_visited[i], false);
        Vector<transportation> laststops = new Vector<transportation>(); //存放结果
        MinHeap heap = new MinHeap();
        transportation res;	//节点指针
        transportation laststop = new transportation();	//存放最后一个站点信息，用于反向输出路径
        //map<int, railway_means>::iterator it;	//用于遍历railway_list
        String[] str_times;
        str_times = time.split(":");
        int r_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//将预计出发时间换算成整数
        /*获取最终站点end所在城市的经纬度lon_end, lat_end*/
        double lon_end = Data.res_city_lon[Data.city.get(end_city)];
        double lat_end = Data.res_city_lat[Data.city.get(end_city)];
        res = new transportation();

        int city_flag = Data.city.get(start_city);
        Vector<String> subname = new Vector<String>();
        Iterator<String> it = Data.have_railway_stops[city_flag].iterator();
        while (it.hasNext())
        {
            String cu_it = it.next();
            /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
            double lon_cur = Data.res_rail_lon[Data.railway_stops.get(cu_it)];
            double lat_cur = Data.res_rail_lat[Data.railway_stops.get(cu_it)];
            double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway"); //预估函数换算成票价
            if (!Data.railplane_to_sub.containsKey(cu_it))
                continue;
            subname = Data.railplane_to_sub.get(cu_it); //和高铁站最近的地铁站列表
            Iterator<String> t = subname.iterator();
            while (t.hasNext())
            {
                String cu_t = t.next();
                Arrays.fill(Data.subway_stop_visited[city_flag], false);
                transportation temp = search_subway_min_price(null, start, cu_t, r_time, city_flag);
                res = new transportation(temp);
                transportation cur_res = new transportation();
                cur_res.stop_name = cu_it;
                cur_res.spendtimes = 10; //假设从地铁站步行到高铁站要 10min
                cur_res.price = 0; //步行不花钱
                cur_res.g = temp.g + cur_res.price; //计算g值
                cur_res.h = h;
                cur_res.transportation_name = "步行";
                cur_res.represent_number = "——";
                cur_res.change_trans_times = res.change_trans_times; //换交通方式
                cur_res.starting_time = res.arrival_time;
                str_times = cur_res.starting_time.split(":");
                cur_res.arrival_time = trans_to_str_time(60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + cur_res.spendtimes);
                cur_res.last = res;
                heap.add(cur_res);
            }
        }

        it = Data.have_plane_stops[city_flag].iterator();
        while (it.hasNext())
        {
            String cu_it = it.next();
            /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
            double lon_cur = Data.res_rail_lon[Data.plane_stops.get(cu_it)];
            double lat_cur = Data.res_rail_lat[Data.plane_stops.get(cu_it)];
            double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
            if (!Data.railplane_to_sub.containsKey(cu_it))
                continue;
            subname = Data.railplane_to_sub.get(cu_it);	//飞机场最靠近的地铁站名列表
            Iterator<String> t = subname.iterator();
            while (t.hasNext())
            {
                String cu_t = t.next();
                Arrays.fill(Data.subway_stop_visited[city_flag],false);
                transportation temp = search_subway_min_price(null, start, cu_t, r_time, city_flag);
                res = new transportation(temp);
                transportation cur_res = new transportation();
                cur_res.stop_name = cu_it;
                cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
                cur_res.price = 0; //步行不花钱
                cur_res.g = temp.g + cur_res.price;
                cur_res.h = h;
                cur_res.transportation_name = "步行";
                cur_res.represent_number = "——";
                cur_res.change_trans_times = res.change_trans_times; //换交通方式
                cur_res.starting_time = res.arrival_time;
                str_times = cur_res.starting_time.split(":");
                cur_res.arrival_time = trans_to_str_time(60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + cur_res.spendtimes);
                cur_res.last = res;
                heap.add(cur_res);
            }
        }
        Data.city_visited[city_flag] = true; //起始城市已访问

        /*A*搜索算法执行*/
        boolean finish = false;
        int fin_flag = 0;
        if (heap.isEmpty())
        {
            finish = true;
            fin_flag = -1;
        }

        int result_flag = 0; //记录搜索到的可行路径数量，用于最舒适计算
        while (!finish && !heap.isEmpty() && result_flag < 4)
        {
            res = new transportation(heap.getAndRemoveMin());   //取出f值最小的点

            if (Data.railway_stops.containsKey(res.stop_name))	//接入高铁网络
            {
                //到达结束站点所在城市，退出搜索
                if (Data.res_rail_city[Data.railway_stops.get(res.stop_name)] == Data.city.get(end_city) && Data.railplane_to_sub.containsKey(res.stop_name))
                {
                    for (transportation pathData = res; pathData.last != null; )
                    {
                        if (!pathData.represent_number.equals(pathData.last.represent_number)) //说明在该站点换乘过车
                        {
                            Data.rail_stop_visited[Data.railway_stops.get(pathData.last.stop_name)] = false;
                            Data.rail_stop_visited[Data.railway_stops.get(pathData.stop_name)] = true;
                            break;
                        }
                        pathData = pathData.last;
                    }
                    //rail_stop_visited[railway_stops[res.stop_name]] = true;	//车站标记为已访问
                    laststop = search_subway_min_price(res, Data.railplane_to_sub.get(res.stop_name).get(0), end, r_time, Data.city.get(end_city));
                    laststops.addElement(laststop);
                    result_flag++;
                    fin_flag = 1;
                    continue;
                    //break;
                }
                Data.rail_stop_visited[Data.railway_stops.get(res.stop_name)] = true;	//车站标记为已访问
                String arr_chici = res.represent_number;	//保存到达当前车站的车次

                str_times = res.arrival_time.split(":");
                //int arr_time = (int)res.g + 60 * atoi(time.c_str()); //到达高铁站的时间
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);//保存该车次到达当前车站的时间
                int cur_stop = Data.railway_stops.get(res.stop_name);	//当前站点编号
                //遍历从当前车站出发能够到达的所有站点
                for (Map.Entry<Integer, railway_means> entry : Data.railway_list[cur_stop].entrySet())
                {
                    int key = entry.getKey();
                    railway_means value = entry.getValue();
                    /*获取it代表的目的站点的经纬度lon_cur,lat_cur*/
                    double lon_cur = Data.res_rail_lon[key];
                    double lat_cur = Data.res_rail_lat[key];
                    double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");

                    transportation temp;
                    if (Data.rail_stop_visited[key])	//该目的站点已被访问过
                        continue;

                    int num = value.railnumbers;	//能够到达目的站点的高铁数量
                    for (int i = 0; i < num; i++)
                    {
                        temp = new transportation(value.railways[i]);
                        temp.change_trans_times = res.change_trans_times;
                        str_times = temp.starting_time.split(":");
                        int leave_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        int wait_time;	//从到站到再次发车间隔的时间

                        if (leave_time < arr_time)	//如果在到达之前就发车的话，不考虑
                        {
                            continue;
                        }

                        wait_time = leave_time - arr_time;

                        if (!temp.represent_number.equals(arr_chici)) //如果需要换乘车次或交通方式
                        {
                            if (wait_time < 15) //考虑换乘的实际情况，需要15min换乘时间
                                continue;
                            temp.change_trans_times += 1; //换车次
                        }
                        //if(wait_time > )
                        temp.g = temp.price + res.g;
                        transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);
                        //查询是否已经在堆中
                        if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                        {
                            if (Inquire.g > temp.g)
                            {
                                Inquire.g = temp.g;
                                Inquire.last = res;
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
                int cur_city_flag = Data.res_rail_city[cur_stop];
                if (cur_city_flag < Data.allow_trans_numbers && Data.railplane_to_sub.containsKey(res.stop_name) && !Data.city_visited[cur_city_flag]) //说明该目的高铁站所在城市有地铁信息
                {
                    Iterator<String > i = Data.railplane_to_sub.get(res.stop_name).iterator();
                    while (i.hasNext())
                    {
                        String cu_i = i.next();
                        Iterator<String> j = Data.have_plane_stops[cur_city_flag].iterator();
                        while (j.hasNext())
                        {
                            String cu_j = j.next();
                            /*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
                            double lon_cur = Data.res_plane_lon[Data.plane_stops.get(cu_j)];
                            double lat_cur = Data.res_plane_lat[Data.plane_stops.get(cu_j)];
                            double h = cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");
                            if (Data.railplane_to_sub.containsKey(cu_j))
                                continue;
                            subname = Data.railplane_to_sub.get(cu_j);
                            Iterator<String> t = subname.iterator();
                            while (t.hasNext())
                            {
                                String cu_t = t.next();
                                Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
                                transportation temp = search_subway_min_price(res, cu_i, cu_t, r_time, cur_city_flag);
                                res = new transportation(temp);
                                transportation cur_res = new transportation();
                                cur_res.stop_name = cu_j;
                                cur_res.spendtimes = 30; //假设从地铁站步行到机场站要 30min
                                cur_res.price = 0; //步行不花钱
                                cur_res.g = temp.g + cur_res.price;
                                cur_res.h = h;
                                cur_res.transportation_name = "步行";
                                cur_res.represent_number = "——";
                                cur_res.change_trans_times = res.change_trans_times + 1; //换交通方式
                                cur_res.starting_time = res.arrival_time;
                                str_times = cur_res.starting_time.split(":");
                                cur_res.arrival_time = trans_to_str_time(60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + cur_res.spendtimes);
                                cur_res.last = res;
                                heap.add(cur_res);
                            }
                        }
                    }
                }
            }

            if (Data.plane_stops.containsKey(res.stop_name))	//接入飞机网络
            {
                //到达结束站点所在城市，退出搜索
                if (Data.res_plane_city[Data.plane_stops.get(res.stop_name)] == Data.city.get(end_city) && Data.railplane_to_sub.containsKey(res.stop_name))
                {
                    for (transportation pathData = res; pathData.last != null; )
                    {
                        if (!pathData.represent_number.equals(pathData.last.represent_number)) //说明在该站点换乘过车
                        {
                            Data.plane_stop_visited[Data.plane_stops.get(pathData.last.stop_name)] = false;
                            break;
                        }
                        pathData = pathData.last;
                    }
                    //plane_stop_visited[plane_stops[res.stop_name]] = true;	//车站标记为已访问
                    laststop = search_subway_min_price(res, Data.railplane_to_sub.get(res.stop_name).get(0), end, r_time, Data.city.get(end_city));
                    laststops.addElement(laststop);
                    result_flag++;
                    fin_flag = 1;
                    //break;
                    continue;
                }
                Data.plane_stop_visited[Data.plane_stops.get(res.stop_name)] = true;	//车站标记为已访问
                String arr_flight_number = res.represent_number;	//保存到达当前机场的航班次
                str_times = res.arrival_time.split(":");
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);//保存该班次飞机到达当前机场的时间
                int cur_stop = Data.plane_stops.get(res.stop_name);
                //遍历从当前机场出发能够到达的所有机场
                for (Map.Entry<Integer,plane_means> entry : Data.plane_list[cur_stop].entrySet())
                {
                    int key = entry.getKey();
                    plane_means value = entry.getValue();
                    /*获取it代表的目的机场的经纬度lon_cur,lat_cur*/
                    double lon_cur = Data.res_plane_lon[key];
                    double lat_cur = Data.res_plane_lat[key];
                    double h = 3.5 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "plane");

                    transportation temp = new transportation();
                    if (Data.plane_stop_visited[key])	//该目的机场已被访问过
                        continue;

                    int num = value.planenumbers;	//能够到达目的机场的飞机数量
                    for (int i = 0; i < num; i++)
                    {
                        temp = new transportation(value.planes[i]);
                        temp.change_trans_times = res.change_trans_times;
                        str_times = temp.starting_time.split(":");
                        int leave_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                        int wait_time;	//从到站到再次起飞间隔的时间

                        if (leave_time < arr_time)	//在到达之前就已发车，不考虑
                        {
                            continue;
                        }

                        wait_time = leave_time - arr_time;

                        if (!temp.represent_number.equals(arr_flight_number)) //如果需要换乘
                        {
                            if (wait_time < 30) //考虑换乘的实际情况，需要30min换乘时间
                                continue;
                            temp.change_trans_times += 1; //换航班
                        }

                        temp.g = temp.price + res.g;
                        transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
                        if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                        {
                            if (Inquire.g > temp.g)
                            {
                                Inquire.g = temp.g;
                                Inquire.last = res;
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
                int cur_city_flag = Data.res_plane_city[cur_stop];
                if (cur_city_flag < Data.allow_trans_numbers && Data.railplane_to_sub.containsKey(res.stop_name) && !Data.city_visited[cur_city_flag]) //说明该目的地飞机场所在城市有地铁信息
                {
                    Iterator<String> i = Data.railplane_to_sub.get(res.stop_name).iterator();
                    while (i.hasNext())
                    {
                        String cu_i = i.next();
                        Iterator<String> j = Data.have_railway_stops[cur_city_flag].iterator();
                        while (j.hasNext())
                        {
                            String cu_j = j.next();
                            if (Data.railplane_to_sub.containsKey(cu_j))
                                continue;
                            /*获取j代表的目的站点的经纬度lon_cur,lat_cur*/
                            double lon_cur = Data.res_rail_lon[Data.railway_stops.get(cu_j)];
                            double lat_cur = Data.res_rail_lat[Data.railway_stops.get(cu_j)];
                            double h = 1.1 * cal_h(lon_end, lat_end, lon_cur, lat_cur, "railway");
                            subname = Data.railplane_to_sub.get(cu_j);
                            Iterator<String> t = subname.iterator();
                            while (t.hasNext())
                            {
                                String cu_t = t.next();
                                Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
                                transportation temp = search_subway_min_price(res, cu_i, cu_t, r_time, cur_city_flag);
                                res = new transportation(temp);
                                transportation cur_res = new transportation();
                                cur_res.stop_name = cu_j;
                                cur_res.spendtimes = 10; //假设从地铁站步行到机场站要 10min
                                cur_res.price = 0; //步行不花钱
                                cur_res.g = temp.g + cur_res.price;
                                cur_res.h = h;
                                cur_res.transportation_name = "步行";
                                cur_res.represent_number = "——";
                                cur_res.change_trans_times = res.change_trans_times + 1; //换交通方式
                                cur_res.starting_time = res.arrival_time;
                                str_times = cur_res.starting_time.split(":");
                                cur_res.arrival_time = trans_to_str_time(60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + cur_res.spendtimes);
                                cur_res.last = res;
                                heap.add(cur_res);
                            }
                        }
                    }
                }
            }
        }


        String[] result = new String[200];
        if (fin_flag == -1)
        {
            System.out.println("您选择的出发站点在您预计出行时间段附近没有车次，请尝试选择其它时间段");
        }
        else if (fin_flag == 0)
        {
            System.out.println("抱歉，未能查询到合适的规划方案，请尝试不同时间和站点并重试");
        }
        else
        {
            int line = 0;
            Collections.sort(laststops, new Comparator<transportation>() {
                @Override
                public int compare(transportation o1, transportation o2) {
                    if (o1.g < o2.g)
                        return -1;
                    else if (o1.g == o2.g)
                        return 0;
                    return 1;
                }
            });

            try {
                //File destFile = new File("最省钱路径规划.txt");
                //FileWriter writer = new FileWriter(destFile);
                //BufferedWriter out = new BufferedWriter(writer);

                transportation pathData = new transportation(laststops.get(0));    //打印路径
                transportation res_route = new transportation();
                transportation res_next_route = new transportation();
                int real_price = (int)pathData.g;
                int change_times = pathData.change_trans_times;
                Stack<transportation> route = new Stack<transportation>();
                int fl = 0;
                String cur_represent_number = "";
                for (; pathData.last != null; ) {
                    route.push(pathData);
                    pathData = pathData.last;
                }

                res_route = route.peek();

                //out.write(res_route.transportation_name + "\n");
                //out.write(res_route.starting_time + "\n");
                //out.write(res_route.represent_number + "\n");
                //out.write(res_route.last.stop_name + "\n");

                result[line++] = res_route.transportation_name;
                result[line++] = res_route.starting_time;
                if(res_route.represent_number.contains("线")){
                    String str = res_route.represent_number.split("线",2)[0]+"线";
                    result[line++] = str;
                }
                else
                    result[line++] = res_route.represent_number;
                result[line++] = res_route.last.stop_name;

                while (!route.empty()) {
                    res_route = route.peek();
                    route.pop();
                    if (route.empty()) break;
                    res_next_route = route.peek();
                    if (!res_route.represent_number.equals(res_next_route.represent_number)) {
                        //out.write(res_route.stop_name + "\n");
                        //out.write(res_route.arrival_time + "\n");
                        result[line++] = res_route.stop_name;
                        result[line++] = res_route.arrival_time;

                        //out.write(res_next_route.transportation_name + "\n");
                        //out.write(res_next_route.starting_time + "\n");
                        //out.write(res_next_route.represent_number + "\n");
                        //out.write(res_next_route.last.stop_name + "\n");
                        result[line++] = res_next_route.transportation_name;
                        result[line++] = res_next_route.starting_time;
                        if(res_next_route.represent_number.contains("线")){
                            String str = res_next_route.represent_number.split("线",2)[0]+"线";
                            result[line++] = str;
                        }
                        else
                            result[line++] = res_next_route.represent_number;
                        result[line++] = res_next_route.last.stop_name;
                        //out.close();
                    }
                }

                //out.write(res_route.stop_name + "\n");
                //out.write(res_route.arrival_time + "\n");
                result[line++] = res_route.stop_name;
                result[line++] = res_route.arrival_time;

                //out.write("——\n——\n——\n——\n");
                //out.write("总票价：" + Integer.toString(real_price) + "\n");
                //out.write("转车转线总次数：" + change_times + "\n");
                result[line++] = "——";
                result[line++] = "——";
                result[line++] = "——";
                result[line++] = "——";
                result[line++] = "总票价：" + Integer.toString(real_price);
                result[line++] = "转车转线总次数：" + Integer.toString(change_times);
                //out.close();

                System.out.println(real_price);
                System.out.println("方案规划成功");
            } catch (Exception e) {
                e.printStackTrace();
            }

        }
        return result;
    }
    static transportation search_subway_min_time(transportation node, String start, String end,
                                                 int time, int cur_city_flag){
        Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
        MinHeap heap = new MinHeap();
        transportation res;	//节点指针
        transportation laststop = new transportation();	//存放最后一个站点信息，用于反向输出路径
        String[] str_times;

        if (node == null)
        {
            res = new transportation(start, 0, 0, null, "起点");
            res.price = 3; //默认地铁票价3元
            res.change_trans_times = 0;
        }
        else
        {
            res = new transportation();
            res.stop_name = start;
            res.spendtimes = 10; //步行10min
            res.last = node;
            res.g = node.g + res.spendtimes;
            res.h = 0;
            res.price = 0;
            res.change_trans_times = node.change_trans_times + 1; //换交通方式的次数
            res.transportation_name = "步行";
            res.represent_number = "——";
            double s_time = node.g + time;
            double a_time = res.g + time;
            res.starting_time = trans_to_str_time(s_time);
            res.arrival_time = trans_to_str_time(a_time);
        }

        int r_time = (int)res.g + time;	//将出发的时间换算成整数

        Data.subway_stop_visited[cur_city_flag][Data.subway_stops[cur_city_flag].get(start)] = true;	//起始站点标记为已访问

        //遍历从当前车站出发能够到达的所有站点
        for (Map.Entry<Integer, subway_means> entry : Data.subway_list[cur_city_flag][Data.subway_stops[cur_city_flag].get(start)].entrySet())
        {
            int key = entry.getKey();
            subway_means value = entry.getValue();
            transportation temp;
            int num = value.subnumbers;	//经过一个站点的地铁数量
            for (int i = 0; i < num; i++)
            {
                temp = new transportation(value.subways[i]);
                str_times = temp.starting_time.split(":");
                int start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//获取地铁出发时间
                str_times = temp.arrival_time.split(":");
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                if (arr_time < start_time)	//说明首末车次时间地铁跨天
                {
                    arr_time += 1440;
                }
                //出发时间在乘坐地铁的运营时间内，视为满足条件
                if (start_time <= r_time && arr_time >= r_time)
                {
                    temp.g = temp.spendtimes + res.g + 2; //等地铁花费 2min
                    double s_time = res.g + 2 + time;
                    temp.starting_time = trans_to_str_time(s_time);
                    double a_time = temp.g + time;
                    temp.arrival_time = trans_to_str_time(a_time);
                    temp.change_trans_times = res.change_trans_times;
                    temp.h = 0;	//地铁站不适用启发函数，退化为Dijkstra算法
                    temp.last = res;
                    heap.add(temp);
                }
            }
        }
        /*搜索算法执行*/
        boolean finish = false;
        int fin_flag = 0;
        if (heap.isEmpty())
        {
            finish = true;
            fin_flag = -1;
        }
        while (!finish && !heap.isEmpty())
        {
            res = new transportation(heap.getAndRemoveMin());   //取出f值最小的点
            if (res.stop_name.equals(end))  {   //找到结束站点，退出搜索
                laststop = res;
                finish = true;
                fin_flag = 1;
                break;
            }
            String arr_checi = res.represent_number;	//保存到达当前车站的线路
            Data.subway_stop_visited[cur_city_flag][Data.subway_stops[cur_city_flag].get(res.stop_name)] = true;	//车站标记为已访问
            //r_time = (int)res.g + time; //保存到达当前车站的时间
            int cur_stop = Data.subway_stops[cur_city_flag].get(res.stop_name);
            //遍历从当前车站出发能够到达的所有站点
            for (Map.Entry<Integer, subway_means> entry : Data.subway_list[cur_city_flag][cur_stop].entrySet()) {
                int key = entry.getKey();
                subway_means value = entry.getValue();

                transportation temp;
                if (Data.subway_stop_visited[cur_city_flag][key])	//该目的站点已被访问过
                    continue;

                int num = value.subnumbers;	//能够到达目的站点的高铁数量
                for (int i = 0; i < num; i++)
                {
                    temp = new transportation(value.subways[i]);
                    str_times = temp.starting_time.split(":");
                    int start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//获取地铁出发时间
                    str_times = temp.arrival_time.split(":");
                    int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    if (arr_time < start_time)	//说明首末车次时间地铁跨天
                    {
                        arr_time += 1440;
                    }
                    temp.g = temp.spendtimes + res.g;
                    temp.change_trans_times = res.change_trans_times;
                    temp.price = res.price;
                    if (!temp.represent_number.equals(arr_checi)) //需要换线
                    {
                        temp.g += 3; //估算地铁换线额外花费的3min
                        temp.change_trans_times += 1; //换交通方式
                    }
                    res.arrival_time = trans_to_str_time(res.g + time);
                    temp.starting_time = trans_to_str_time(temp.g - temp.spendtimes + time);
                    temp.arrival_time = trans_to_str_time(temp.g + time);

                    transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
                    if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                    {
                        if (Inquire.g > temp.g)
                        {
                            Inquire.g = temp.g;
                            Inquire.last = res;
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
    static transportation search_subway_min_price(transportation node, String start, String end,
                                                  int time, int cur_city_flag){
        Arrays.fill(Data.subway_stop_visited[cur_city_flag], false);
        MinHeap heap = new MinHeap();
        transportation res;	//节点指针
        transportation laststop = new transportation();	//存放最后一个站点信息，用于反向输出路径
        String[] str_times;

        if (node == null)
        {
            res = new transportation(start, 0, 0, null, "起点");
            res.price = 3; //默认地铁票价3元
            res.g = 3;
            res.change_trans_times = 0;
            res.arrival_time = trans_to_str_time(time);
        }
        else
        {
            res = new transportation();
            res.stop_name = start;
            res.spendtimes = 10; //步行10min
            res.price = 0;
            res.last = node;
            res.g = node.g + res.price;
            res.h = 0;
            res.change_trans_times = node.change_trans_times; //换交通方式的次数
            res.transportation_name = "步行";
            res.represent_number = "——";
            res.starting_time = node.arrival_time;
            str_times = res.starting_time.split(":");
            res.arrival_time = trans_to_str_time(60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + res.spendtimes);
        }
        str_times = res.arrival_time.split(":");
        int r_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//将出发的时间换算成整数

        Data.subway_stop_visited[cur_city_flag][Data.subway_stops[cur_city_flag].get(start)] = true;	//起始站点标记为已访问

        //遍历从当前车站出发能够到达的所有站点

        for (Map.Entry<Integer, subway_means> entry : Data.subway_list[cur_city_flag][Data.subway_stops[cur_city_flag].get(start)].entrySet()) {
            int key = entry.getKey();
            subway_means value = entry.getValue();
            transportation temp;
            int num = value.subnumbers;	//经过一个站点的地铁数量
            for (int i = 0; i < num; i++)
            {
                temp = new transportation(value.subways[i]);
                str_times = temp.starting_time.split(":");
                int start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//获取地铁出发时间
                str_times = temp.arrival_time.split(":");
                int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                if (arr_time < start_time)	//说明首末车次时间地铁跨天
                {
                    arr_time += 1440;
                }
                //出发时间在乘坐地铁的运营时间内，视为满足条件
                if (start_time <= r_time && arr_time >= r_time)
                {
                    temp.g = temp.price + res.g;
                    str_times = res.arrival_time.split(":");
                    double s_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]) + 2;
                    temp.starting_time = trans_to_str_time(s_time);
                    double a_time = s_time + temp.spendtimes;
                    temp.arrival_time = trans_to_str_time(a_time);
                    temp.change_trans_times = res.change_trans_times;
                    temp.h = 0;	//地铁站不适用启发函数，退化为Dijkstra算法
                    temp.last = res;
                    heap.add(temp);
                }
            }
        }
        /*搜索算法执行*/
        boolean finish = false;
        int fin_flag = 0;
        if (heap.isEmpty())
        {
            finish = true;
            fin_flag = -1;
        }
        while (!finish && !heap.isEmpty())
        {
            res = new transportation(heap.getAndRemoveMin());   //取出f值最小的点
            if (res.stop_name.equals(end))	//找到结束站点，退出搜索
            {
                laststop = res;
                finish = true;
                fin_flag = 1;
                break;
            }
            String arr_checi = res.represent_number;	//保存到达当前车站的线路
            Data.subway_stop_visited[cur_city_flag][Data.subway_stops[cur_city_flag].get(res.stop_name)] = true;	//车站标记为已访问
            str_times = res.arrival_time.split(":");

            r_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//将出发的时间换算成整数
            int cur_stop = Data.subway_stops[cur_city_flag].get(res.stop_name);
            //遍历从当前车站出发能够到达的所有站点
            for (Map.Entry<Integer, subway_means> entry : Data.subway_list[cur_city_flag][cur_stop].entrySet())
            {
                int key = entry.getKey();
                subway_means value = entry.getValue();
                transportation temp;
                if (Data.subway_stop_visited[cur_city_flag][key])	//该目的站点已被访问过
                    continue;

                int num = value.subnumbers;	//能够到达目的站点的高铁数量
                for (int i = 0; i < num; i++)
                {
                    temp = new transportation(value.subways[i]);
                    str_times = temp.starting_time.split(":");
                    int start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);	//获取地铁出发时间
                    str_times = temp.arrival_time.split(":");
                    int arr_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    if (arr_time < start_time)	//说明首末车次时间地铁跨天
                    {
                        arr_time += 1440;
                    }
                    temp.price = res.price;
                    temp.g = temp.price + res.g;
                    temp.change_trans_times = res.change_trans_times;
                    str_times = res.arrival_time.split(":");
                    int cur_start_time = 60 * Integer.parseInt(str_times[0]) + Integer.parseInt(str_times[1]);
                    if (temp.represent_number != arr_checi) //需要换线
                    {
                        cur_start_time += 3;	//估算地铁换线额外花费的3min
                        temp.change_trans_times += 1; //换交通方式
                    }
                    //res.arrival_time = trans_to_str_time(res.g + time);
                    temp.starting_time = trans_to_str_time(cur_start_time);
                    temp.arrival_time = trans_to_str_time(cur_start_time + temp.spendtimes);

                    transportation Inquire = heap.find(temp.represent_number + "_" + temp.stop_name);	//查询是否已经在堆中
                    if (Inquire != null)	//在堆中，更新g值和上一个站点信息
                    {
                        if (Inquire.g > temp.g)
                        {
                            Inquire.g = temp.g;
                            Inquire.last = res;
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

    static String trans_to_str_time(double i_time) {
        if (i_time >= 1440)
            i_time -= 1440;
        int h = (int)i_time / 60;
        int min = (int)i_time - 60 * h;
        String str_h = Integer.toString(h);
        String str_min = Integer.toString(min);
        if (h < 10)
            str_h = "0" + str_h;
        if (min < 10)
            str_min = "0" + str_min;
        String str_time = str_h + ":" + str_min;
        return str_time;
    }
    static double cal_h(double lon1, double lat1, double lon2, double lat2, String trans_means) {
        // 得到起点经纬度,并转换为角度
        double PI = Math.PI;
        double startLon = (PI / 180) * lon1;
        double startLan = (PI / 180) * lat1;
        // 得到终点经纬度,并转换为角度
        double endLon = (PI / 180) * lon2;
        double endtLan = (PI / 180) * lat2;

        // 地球平均半径为6371km
        double earthR = 6371;

        // 计算公式
        double distence =
                Math.acos(Math.sin(startLan) * Math.sin(endtLan) + Math.cos(startLan) * Math.cos(endtLan) * Math.cos(endLon - startLon)) * earthR;

        if (trans_means.equals("railway"))
            return (distence / 230) * 60; //假设地铁的平均运行速度为230km/h

        else if (trans_means.equals("plane"))
            return (distence / 390) * 60;

        return -1;
    }

    public static void main(String[] args){
        create_map();
        String[] result = search_min("八角游乐园", "北京", "万达城", "合肥", "10:00", 2);
        for(int i = 0; i < result.length; i++){
            if(result[i] == null)
                break;
            System.out.println(result[i]);
        }
    }
}