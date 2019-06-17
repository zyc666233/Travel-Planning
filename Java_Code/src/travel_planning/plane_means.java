package travel_planning;

public class plane_means {

    plane_means(){
        for(int i = 0; i < 300; i++){
            planes[i] = new transportation();
        }
        planenumbers = 0;
    }

    public transportation[] planes = new transportation[300];  //假定一个飞机场一天起飞最多300架次
    //int flag;
    public int planenumbers;	//一个飞机场起飞的航班数量
}
