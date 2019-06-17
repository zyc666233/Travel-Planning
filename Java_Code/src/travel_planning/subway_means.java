package travel_planning;

public class subway_means {

    subway_means(){
        for(int i = 0; i < 20; i++){
            subways[i] = new transportation();
        }
        subnumbers = 0;
    }
    public transportation[] subways = new transportation[20];  //一个地铁站最多经过20条不同线路
    //int flag;
    public int subnumbers;	//一个地铁站经过的不同线路数量
}
