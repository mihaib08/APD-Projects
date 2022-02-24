import map.MapTask;
import reduce.ReduceTask;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Map-Reduce Thread
 *  1) Solves the Map tasks which are allocated to it
 *  2) Solves the Reduce tasks
 */
public class MRThread extends Thread {
    List<MapTask> mapTasks;
    List<ReduceTask> reduceTasks;

    MRThread() {
        mapTasks = new ArrayList<>();
        reduceTasks = new ArrayList<>();
    }

    @Override
    public void run() {
        for (var t : mapTasks) {
            try {
                t.solveTask();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        for (var t : reduceTasks) {
            t.solveTask();
        }
    }

    public void addMapTask(MapTask t) {
        mapTasks.add(t);
    }

    public void addReduceTask(ReduceTask t) {
        reduceTasks.add(t);
    }
}
