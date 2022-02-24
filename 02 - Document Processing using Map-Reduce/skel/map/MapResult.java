package map;

import java.util.List;
import java.util.Map;

public class MapResult {
    String name;

    // <(length) l, no. words that have length l>
    private final Map<Integer, Integer> wordDict;
    // list of the longest words
    private final List<String> longestWs;

    MapResult(String name, Map<Integer, Integer> wordDict, List<String> longestWs) {
        this.name = name;
        this.wordDict = wordDict;
        this.longestWs = longestWs;
    }

    @Override
    public String toString() {
        return "{" +
                "wordDict=" + wordDict +
                ", longestWs=" + longestWs +
                '}';
    }

    // Getters

    public Map<Integer, Integer> getWordDict() {
        return wordDict;
    }

    public List<String> getLongestWs() {
        return longestWs;
    }
}
