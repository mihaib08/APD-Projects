package reduce;

public class ReduceResult {
    private final String name;
    // useful for sorting equality cases
    int id;

    private final double rank;
    private final int maxLen;
    private final int occ;

    ReduceResult(int id, String name, double rank, int maxLen, int occ) {
        this.name = name;
        this.id = id;

        this.rank = rank;
        this.maxLen = maxLen;
        this.occ = occ;
    }

    // Getters
    public String getName() {
        return name;
    }

    public int getId() {
        return id;
    }

    public double getRank() {
        return rank;
    }

    public int getMaxLen() {
        return maxLen;
    }

    public int getOcc() {
        return occ;
    }
}
