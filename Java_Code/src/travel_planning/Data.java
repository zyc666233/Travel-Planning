package travel_planning;
import java.util.*;

public class Data {
    public static Map<String, Integer> city = new HashMap<String, Integer>();	//城市名映射为编号
    public static Vector<String>[] have_railway_stops = new Vector[6];	//暂保存6个城市拥有的高铁站名称
    public static Vector<String>[] have_plane_stops = new Vector[6];	//暂保存6个城市拥有的飞机场名称
    public static Map<String, Vector<String> > railplane_to_sub = new HashMap<String, Vector<String>>();	//高铁站附近的地铁站
    public static boolean[] city_visited = new boolean[6];
    public static double[] res_city_lon = new double[6];
    public static double[] res_city_lat = new double[6];
    public static int allow_trans_numbers;	//暂时允许经地铁转换交通方式的城市数量

    public static Map<String, Integer> railway_stops = new HashMap<String, Integer>();	//将高铁站名映射为编号
    public static Map<Integer, railway_means>[] railway_list = new HashMap[620];	//构建邻接表存储高铁站的图信息
    public static boolean[] rail_stop_visited = new boolean[620];
    public static double[] res_rail_lon = new double[620];	//经度表
    public static double[] res_rail_lat = new double[620];	//纬度表
    public static int[] res_rail_city = new int[620];	//高铁站所属城市表

    public static Map<String, Integer> plane_stops = new HashMap<String, Integer>(); //将飞机场名映射为编号
    public static Map<Integer, plane_means>[] plane_list = new HashMap[100];	//构建邻接表存储飞机场的图信息
    public static boolean[] plane_stop_visited = new boolean[100];
    public static double[] res_plane_lon = new double[100];	//经度表
    public static double[] res_plane_lat = new double[100];	//纬度表
    public static int[] res_plane_city = new int[100];	//飞机场所属城市表

    public static Map<String, Integer>[] subway_stops = new HashMap[6];	//将6个城市的地铁站名映射为编号
    public static Map<Integer, subway_means>[][] subway_list = new HashMap[6][400];	//构建邻接表存储6个城市地铁站的图信息
    public static boolean[][] subway_stop_visited = new boolean[6][400];


}
