package map;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

// aggregate the MapResults per document
public class Results {
    public Map<String, List<MapResult>> docResults;

    public Results(List<String> documents) {
        docResults = new HashMap<>();

        for (var d : documents) {
            List<MapResult> res = new ArrayList<>();
            docResults.put(d, res);
        }
    }
}
