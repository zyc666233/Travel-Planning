package travel_planning;
import java.util.*;

public class transportation implements Comparable<transportation>{
    transportation(){
        this.stop_name = "";
        this.g = -1;
        this.h = -1;
        this.last = null;
    }
    transportation(transportation x){
        this.stop_name = x.stop_name;
        this.represent_number = x.represent_number;
        this.starting_time = x.starting_time;
        this.arrival_time = x.arrival_time;
        this.change_trans_times = x.change_trans_times;
        this.g = x.g;
        this.h = x.h;
        this.price = x.price;
        this.spendtimes = x.spendtimes;
        this.transportation_name = x.transportation_name;
        this.last = x.last;
    }
    transportation(String stop_name, double g, double h, transportation last, String represent_number){
        this.stop_name = stop_name;
        this.g = g;
        this.h = h;
        this.represent_number = represent_number;
        this.last = last;
    }

    public int compareTo(transportation c){     //实现比较器，按f值升序
        if(this.g + this.h < c.g + c.h)
            return -1;
        else if(this.g + this.h == c.g + c.h)
            return 0;
        else
            return 1;
    }

    public String stop_name;	//驶向的目标车站或飞机场名或地铁站名
    public String represent_number;	//车次或航班号或地铁线路
    public String starting_time;	//在地铁表里表示首班车时间
    public String arrival_time;	//在地铁表里表示末班车时间
    public String transportation_name;	//到达目标站点的交通方式
    public int spendtimes;
    public int price;
    public int change_trans_times;
    public double g, h;
    public transportation last;	//记录上一条交通信息
}
