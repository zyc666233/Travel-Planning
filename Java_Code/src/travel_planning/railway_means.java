package travel_planning;

public class railway_means {

    railway_means(){
        for(int i = 0; i < 300; i++){
            railways[i] = new transportation();
        }
        railnumbers = 0;
    }

    public transportation[] railways = new transportation[300]; //假定一个高铁站一天发出最多300车次
    //int flag;
    public int railnumbers;	//一个高铁站发出车次的数量
}
