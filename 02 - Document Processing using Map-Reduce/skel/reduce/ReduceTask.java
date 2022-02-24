package reduce;

import map.MapResult;
import utils.Rank;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Semaphore;

public class ReduceTask {
    private final String name;
    int id;

    private List<MapResult> taskResults;

    // map obtained after combining the map tasks results
    private Map<Integer, Integer> words;

    // determine the maximum length of a word in the document
    private int maxLen;
    private int occ;
    private double rank;

    private ReduceResult res;

    private final Semaphore sem;

    public ReduceTask(String name, List<MapResult> results, int id, Semaphore sem) {
        this.name = name;
        this.id = id;
        this.taskResults = results;

        this.sem = sem;

        words = new HashMap<>();
        maxLen = 0;
        occ = 0;
    }

    void combine() {
        for (var docRes : taskResults) {
            var wd = docRes.getWordDict();
            var lw = docRes.getLongestWs();

            for (Integer len : wd.keySet()) {
                int ct = wd.get(len);

                int curr = words.getOrDefault(len, 0);
                curr += ct;
                words.put(len, curr);
            }
        }
    }

    void process() {
        rank = Rank.computeRank(words);

        for (int len : words.keySet()) {
            if (len > maxLen) {
                maxLen = len;
                occ = words.get(len);
            }
        }
    }

    public void solveTask() {
        // make sure that all the Map tasks
        // associated to the document <name>
        // have finished
        try {
            sem.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        combine();
        process();

        res = new ReduceResult(id, name, rank, maxLen, occ);
    }

    // Getters

    public ReduceResult getResult() {
        return res;
    }
}
