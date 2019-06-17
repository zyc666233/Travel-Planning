package travel_planning;
import java.util.*;

public class MinHeap {
    public Map<String, transportation> index = new HashMap<String, transportation>(); //记录堆中的元素
    public PriorityQueue<transportation> queue = new PriorityQueue<transportation>(1500);	//优先队列模拟小顶堆

    public transportation getAndRemoveMin(){    //获取并删除堆顶值
        if (queue.isEmpty())
            return new transportation();
        transportation head = new transportation(queue.poll());
        String str = head.represent_number + "_" + head.stop_name;
        index.remove(str);

        return head;
    }

    public transportation find(String pnt){
        return index.get(pnt);
    }

    public void add(transportation data){
        queue.add(data);
        String str = data.represent_number + "_" + data.stop_name;
        index.put(str, data);
    }

    public boolean isEmpty(){
        return queue.isEmpty();
    }
}