package utils;

import java.util.Map;

public class Rank {
    private static final int KMAX = 2;

    private static int[][] multiplyMatrix(int[][] a, int[][] b) {
        int i, j, k;
        int[][] res = new int[KMAX][KMAX];

        for (i = 0; i < KMAX; ++i) {
            for (j = 0; j < KMAX; ++j) {
                // R[i][j]
                int sum = 0;

                for (k = 0; k < KMAX; ++k) {
                    int aux = a[i][k] * b[k][j];

                    sum = sum + aux;
                }

                res[i][j] = sum;
            }
        }

        return res;
    }

    private static int[][] logPow(int[][] a, int exp) {
        int[][] r = new int[KMAX][KMAX];
        int i;

        for (i = 0; i < KMAX; ++i) {
            r[i][i] = 1;
        }

        while (exp != 0) {
            // current bit is set
            if ((exp & 1) != 0) {
                r = multiplyMatrix(r, a);
            }

            // A = A * A
            a = multiplyMatrix(a, a);

            exp >>= 1;
        }

        return r;
    }

    private static int calcNth(int n) {
        int res;

        int[][] c = new int[KMAX][KMAX];
        c[0][0] = 0;
        c[0][1] = 1;
        c[1][0] = 1;
        c[1][1] = 1;

        int[][] r;

        r = logPow(c, n - 1);

        int f0 = 0;
        int f1 = 1;

        res = f0 * r[0][1] + f1 * r[1][1];

        return res;
    }

    public static double computeRank(Map<Integer, Integer> lens) {
        double res = 0.00f;
        int t = 0;

        for (var l : lens.keySet()) {
            int ct = lens.get(l);
            t += ct;

            res += (double)(calcNth(l + 1) * ct);
        }
        res = (double)res / t;

        return res;
    }
}
