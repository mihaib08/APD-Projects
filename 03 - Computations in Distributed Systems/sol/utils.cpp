#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

void getTopology(int rank, std::vector<int *>& clusters) {
    // sort the clusters according to their coordinator
    std::sort(clusters.begin(), clusters.end(), [&](int *c1, int *c2) {
        return c1[0] < c2[0];
    });

    int i;

    printf("%d -> ", rank);
    for (auto c : clusters) {
        printf("%d:", c[0]);

        for (i = 0; i < c[1] - 1; ++i) {
            printf("%d,", c[i + 2]);
        }
        printf("%d ", c[i + 2]);
    }
    printf("\n");
}

void displayMessage(int from, int to) {
    printf("M(%d,%d)\n", from, to);
}
