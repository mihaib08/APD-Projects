package map;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Semaphore;

public class MapTask {
    // document name
    private final String name;
    private final int id;

    // offset
    private final int off;

    // size
    private final long size;

    // File reader - RandomAccessFile
    //  -- seek()
    private final RandomAccessFile raf;

    private MapResult res;

    // "send" the task results to the coordinator
    private final Results center;

    private final Semaphore sem;

    public MapTask(String name, int off, long size, RandomAccessFile raf, int id, Results r, Semaphore sem) {
        this.name = name;
        this.off = off;
        this.size = size;
        this.raf = raf;
        this.id = id;

        this.sem = sem;

        this.center = r;
    }

    public void solveTask() throws IOException {
        Map<Integer, Integer> wordDict;
        List<String> longestWs;

        wordDict = new HashMap<>();
        longestWs = new ArrayList<>();

        List<MapResult> curr;

        // tasks of the same document
        //  --> the same RandomFileAccess object
        synchronized (raf) {
            byte[] bs = new byte[(int) size];
            raf.seek(off);
            raf.read(bs);

            String str = new String(bs);
            String[] words = str.split("[^\\p{Alnum}]+");

            int len;
            int maxLen = 0;
            for (var w : words) {
                len = w.length();
                if (len == 0) {
                    // jump over 0-lengthened words
                    continue;
                }

                int v = wordDict.getOrDefault(len, 0);
                ++v;
                wordDict.put(len, v);

                if (len > maxLen) {
                    longestWs.clear();
                    longestWs.add(w);
                    maxLen = len;
                }
            }
            res = new MapResult(name, wordDict, longestWs);

            // send the results to the coordinator
            curr = center.docResults.get(name);
            curr.add(res);
        }

        sem.release();
    }

    // Getters & Setters

    public int getOffset() {
        return off;
    }

    public long getSize() {
        return size;
    }

    public RandomAccessFile getRaf() {
        return raf;
    }

    public MapResult getResults() {
        return res;
    }
}
