#include "coordinator.h"

#include "utils.h"

void findTopology(const std::string in, std::vector<int *>& clusters, int rank) {
    FILE *fin = fopen(in.c_str(), "rt");

    int N;
    // no. workers
    fscanf(fin, "%d", &N);

    int *curr = new int[N + 2];
    curr[0] = rank;
    curr[1] = N;

    int i;
    for (i = 0; i < N; ++i) {
        fscanf(fin, "%d", &curr[i + 2]);
    }

    // other coordinators
    int cs[2];
    if (rank == 0) {
        cs[0] = 1;
        cs[1] = 2;
    } else if (rank == 1) {
        cs[0] = 0;
        cs[1] = 2;
    } else if (rank == 2) {
        cs[0] = 0;
        cs[1] = 1;
    }

    // send the current cluster to the other coordinators
    //  - no. workers (tag 0)
    //  - cluster array (tag 1)
    for (auto c : cs) {
        MPI_Send(&N, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        MPI_Send(curr, N + 2, MPI_INT, c, 1, MPI_COMM_WORLD);
        displayMessage(rank, c);
    }

    clusters.push_back(curr);
    // receive the cluster data from the other coordinators
    for (auto c : cs) {
        int nc;
        MPI_Recv(&nc, 1, MPI_INT, c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *cluster = new int[nc + 2];
        MPI_Recv(cluster, nc + 2, MPI_INT, c, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        clusters.push_back(cluster);
    }

    // send the data to the workers
    // - number of clusters
    // - each cluster
    int numClusters = clusters.size();
    int w;
    for (i = 0; i < N; ++i) {
        w = curr[i + 2];
        MPI_Send(&numClusters, 1, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    // send one cluster at a time to each worker
    // - cluster size
    // - the cluster
    for (auto c : clusters) {
        for (i = 0; i < N; ++i) {
            w = curr[i + 2];

            MPI_Send(&c[1], 1, MPI_INT, w, 3, MPI_COMM_WORLD);
            displayMessage(rank, w);

            MPI_Send(c, c[1] + 2, MPI_INT, w, 4, MPI_COMM_WORLD);
            displayMessage(rank, w);
        }
    }

    getTopology(rank, clusters);
}

void rootCompute(std::vector<int *>& clusters, int rank, int N, int numTasks) {
    int cs[] = {1, 2};
    // send N to the other coordinators
    for (auto c : cs) {
        MPI_Send(&N, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);
    }

    int *v = new int[N];
    int i;
    for (i = 0; i < N; ++i) {
        v[i] = i;
    }

    int W = numTasks - 3; // no. workers

    // no. workers currently assigned
    int curr = 0;
    double q = (double) N / W;
    int start = 0;

    std::vector<std::vector<int>> lims;
    // array limits to be worked on
    int l, r;
    for (i = 0; i < clusters[rank][1]; ++i) {
        int w = clusters[rank][i + 2];

        l = curr * q;
        r = std::min(N, (int) ((curr + 1) * q));
        ++curr;

        lims.push_back({l, r});

        int length = r - l;
        // send the length to the worker
        MPI_Send(&length, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    std::vector<int> cLims; // coordinators' limits
    cLims.push_back(r);
    for (auto c : cs) {
        MPI_Send(&curr, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        MPI_Send(&r, 1, MPI_INT, c, 1, MPI_COMM_WORLD);
        displayMessage(rank, c);

        // wait for the new limits
        MPI_Recv(&curr, 1, MPI_INT, c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r, 1, MPI_INT, c, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cLims.push_back(r);
    }

    // send the partitioned array
    // - to the workers
    // - to the other coordinators
    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Send(v + l, r - l, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    i = 0;
    for (auto c : cs) {
        int start = cLims[i];
        int len = cLims[i + 1] - cLims[i];

        MPI_Send(v + start, len, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        ++i;
    }

    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Recv(v + l, r - l, MPI_INT, w, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    i = 0;
    for (auto c : cs) {
        int s = cLims[i];
        int len = cLims[i + 1] - cLims[i];

        MPI_Recv(v + s, len, MPI_INT, c, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        ++i;
    }

    printf("Rezultat: ");
    for (i = 0; i < N - 1; ++i) {
        printf("%d ", v[i]);
    }
    printf("%d\n", v[i]);
}

void coordinatorCompute(std::vector<int *>& clusters, int rank, int numTasks) {
    int N;
    MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int curr, start;
    MPI_Recv(&curr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&start, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int W = numTasks - 3; // no. workers
    double q = (double) N / W;

    std::vector<std::vector<int>> lims;
    // array limits to be worked on
    int i, l, r;
    for (i = 0; i < clusters[rank][1]; ++i) {
        int w = clusters[rank][i + 2];

        l = curr * q;
        r = std::min(N, (int) ((curr + 1) * q));
        ++curr;

        lims.push_back({l, r});

        int length = r - l;
        // send the length to the worker
        MPI_Send(&length, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    // send the new curr, r
    MPI_Send(&curr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    displayMessage(rank, 0);

    MPI_Send(&r, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    displayMessage(rank, 0);

    int len = r - start;
    int *arr = new int[len];

    MPI_Recv(arr, len, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // send the partitioned array
    // - to the workers
    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Send(arr + l, r - l, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Recv(arr + l, r - l, MPI_INT, w, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Send(arr, len, MPI_INT, 0, 6, MPI_COMM_WORLD);
    displayMessage(rank, 0);
}

// ---- BONUS ----

void bonusTopology(const std::string in, std::vector<int *>& clusters, int rank) {
    FILE *fin = fopen(in.c_str(), "rt");

    int N;
    // no. workers
    fscanf(fin, "%d", &N);

    int *curr = new int[N + 2];
    curr[0] = rank;
    curr[1] = N;

    int i;
    for (i = 0; i < N; ++i) {
        fscanf(fin, "%d", &curr[i + 2]);
    }

    // other coordinators
    // -- (0, 1) is no longer an edge
    std::vector<int> cs;
    if (rank == 0) {
        cs.push_back(2);
    } else if (rank == 1) {
        cs.push_back(2);
    } else if (rank == 2) {
        cs.push_back(0);
        cs.push_back(1);
    }

    // send the current cluster to the other coordinators
    //  - no. workers (tag 0)
    //  - cluster array (tag 1)
    for (auto c : cs) {
        MPI_Send(&N, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        MPI_Send(curr, N + 2, MPI_INT, c, 1, MPI_COMM_WORLD);
        displayMessage(rank, c);
    }

    clusters.push_back(curr);
    // receive the cluster data from the other coordinators
    for (auto c : cs) {
        int nc;
        MPI_Recv(&nc, 1, MPI_INT, c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *cluster = new int[nc + 2];
        MPI_Recv(cluster, nc + 2, MPI_INT, c, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        clusters.push_back(cluster);

        // for Bonus -> send the data to the _other_ associated coordinators
        for (auto cc : cs) {
            if (cc != c) {
                // send coordinator <c> data to the coordinator <cc>
                // here, <0> -> <2> -> <1>
                //       <1> -> <2> -> <0>
                MPI_Send(&nc, 1, MPI_INT, cc, 2, MPI_COMM_WORLD);
                displayMessage(rank, cc);

                MPI_Send(cluster, nc + 2, MPI_INT, cc, 3, MPI_COMM_WORLD);
                displayMessage(rank, cc);
            }
        }
    }

    if (rank == 0 || rank == 1) {
        // wait for another cluster from 2
        // (!) tags are different
        for (auto c : cs) {
            int nc;
            MPI_Recv(&nc, 1, MPI_INT, c, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int *cluster = new int[nc + 2];
            MPI_Recv(cluster, nc + 2, MPI_INT, c, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            clusters.push_back(cluster);
        }
    }

    // send the data to the workers
    // - number of clusters
    // - each cluster
    int numClusters = clusters.size();
    int w;
    for (i = 0; i < N; ++i) {
        w = curr[i + 2];
        MPI_Send(&numClusters, 1, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    // send one cluster at a time to each worker
    // - cluster size
    // - the cluster
    for (auto c : clusters) {
        for (i = 0; i < N; ++i) {
            w = curr[i + 2];

            MPI_Send(&c[1], 1, MPI_INT, w, 3, MPI_COMM_WORLD);
            displayMessage(rank, w);

            MPI_Send(c, c[1] + 2, MPI_INT, w, 4, MPI_COMM_WORLD);
            displayMessage(rank, w);
        }
    }

    getTopology(rank, clusters);
}

void rootBonusCompute(std::vector<int *>& clusters, int rank, int N, int numTasks) {
    int cs[] = {2};
    // send N to the other connected coordinators
    for (auto c : cs) {
        MPI_Send(&N, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);
    }

    int *v = new int[N];
    int i;
    for (i = 0; i < N; ++i) {
        v[i] = i;
    }

    int W = numTasks - 3; // no. workers

    // no. workers currently assigned
    int curr = 0;
    double q = (double) N / W;
    int start = 0;

    std::vector<std::vector<int>> lims;
    // array limits to be worked on
    int l, r;
    for (i = 0; i < clusters[rank][1]; ++i) {
        int w = clusters[rank][i + 2];

        l = curr * q;
        r = std::min(N, (int) ((curr + 1) * q));
        ++curr;

        lims.push_back({l, r});

        int length = r - l;
        // send the length to the worker
        MPI_Send(&length, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    std::vector<int> cLims; // coordinators' limits
    cLims.push_back(r);
    for (auto c : cs) {
        MPI_Send(&curr, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        MPI_Send(&r, 1, MPI_INT, c, 1, MPI_COMM_WORLD);
        displayMessage(rank, c);

        // wait for the new limits
        MPI_Recv(&curr, 1, MPI_INT, c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r, 1, MPI_INT, c, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cLims.push_back(r);
    }

    // send the partitioned array
    // - to the workers
    // - to the other coordinators
    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Send(v + l, r - l, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    i = 0;
    for (auto c : cs) {
        int start = cLims[i];
        int len = cLims[i + 1] - cLims[i];

        MPI_Send(v + start, len, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);

        ++i;
    }

    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Recv(v + l, r - l, MPI_INT, w, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    i = 0;
    for (auto c : cs) {
        int s = cLims[i];
        int len = cLims[i + 1] - cLims[i];

        MPI_Recv(v + s, len, MPI_INT, c, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        ++i;
    }

    printf("Rezultat: ");
    for (i = 0; i < N - 1; ++i) {
        printf("%d ", v[i]);
    }
    printf("%d\n", v[i]);
}

void coordinator2Compute(std::vector<int *>& clusters, int rank, int numTasks) {
    int N;
    MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int cs[] = {1};
    // send N to the other coordinators
    for (auto c : cs) {
        MPI_Send(&N, 1, MPI_INT, c, 0, MPI_COMM_WORLD);
        displayMessage(rank, c);
    }

    int curr, start;

    int curr1, start1;
    MPI_Recv(&curr1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&start1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // forward the limits to cluster 1
    MPI_Send(&curr1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    displayMessage(rank, 1);

    MPI_Send(&start1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
    displayMessage(rank, 1);

    // wait for receive of the new limits from 1
    MPI_Recv(&curr, 1, MPI_INT, 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&start, 1, MPI_INT, 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int W = numTasks - 3; // no. workers
    double q = (double) N / W;

    std::vector<std::vector<int>> lims;
    // array limits to be worked on
    int i, l, r;
    for (i = 0; i < clusters[rank][1]; ++i) {
        int w = clusters[rank][i + 2];

        l = curr * q;
        r = std::min(N, (int) ((curr + 1) * q));
        ++curr;

        lims.push_back({l, r});

        int length = r - l;
        // send the length to the worker
        MPI_Send(&length, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    // send the new curr, r
    MPI_Send(&curr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    displayMessage(rank, 0);

    MPI_Send(&r, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    displayMessage(rank, 0);

    int len = r - start1;
    int *arr = new int[len];

    // offset from the array
    // correspondent to cluster 1
    int off = start - start1;

    MPI_Recv(arr, len, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // send the [start1, start) - start1 part to cluster 1
    MPI_Send(arr, off, MPI_INT, 1, 0, MPI_COMM_WORLD);
    displayMessage(rank, 1);

    // send the partitioned array
    // - to the workers
    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start + off;
        r = lims[i][1] - start + off;

        MPI_Send(arr + l, r - l, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start + off;
        r = lims[i][1] - start + off;

        MPI_Recv(arr + l, r - l, MPI_INT, w, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // receive the partition assigned to cluster 1
    MPI_Recv(arr, off, MPI_INT, 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // send the array worked by cluster 1 - [start1, start)
    //                      and cluster 2 - [start..]
    MPI_Send(arr, len, MPI_INT, 0, 6, MPI_COMM_WORLD);
    displayMessage(rank, 0);
}

void coordinator1Compute(std::vector<int *>& clusters, int rank, int numTasks) {
    int N;
    MPI_Recv(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int curr, start;
    MPI_Recv(&curr, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&start, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int W = numTasks - 3; // no. workers
    double q = (double) N / W;

    std::vector<std::vector<int>> lims;
    // array limits to be worked on
    int i, l, r;
    for (i = 0; i < clusters[rank][1]; ++i) {
        int w = clusters[rank][i + 2];

        l = curr * q;
        r = std::min(N, (int) ((curr + 1) * q));
        ++curr;

        lims.push_back({l, r});

        int length = r - l;
        // send the length to the worker
        MPI_Send(&length, 1, MPI_INT, w, 0, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    // send the new curr, r
    MPI_Send(&curr, 1, MPI_INT, 2, 2, MPI_COMM_WORLD);
    displayMessage(rank, 2);

    MPI_Send(&r, 1, MPI_INT, 2, 3, MPI_COMM_WORLD);
    displayMessage(rank, 2);

    int len = r - start;
    int *arr = new int[len];

    MPI_Recv(arr, len, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // send the partitioned array
    // - to the workers
    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Send(arr + l, r - l, MPI_INT, w, 2, MPI_COMM_WORLD);
        displayMessage(rank, w);
    }

    for (i = 0; i < lims.size(); ++i) {
        int w = clusters[rank][i + 2];

        l = lims[i][0] - start;
        r = lims[i][1] - start;

        MPI_Recv(arr + l, r - l, MPI_INT, w, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Send(arr, len, MPI_INT, 2, 6, MPI_COMM_WORLD);
    displayMessage(rank, 2);
}
