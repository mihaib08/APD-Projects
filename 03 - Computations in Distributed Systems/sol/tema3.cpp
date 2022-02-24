#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include "utils.h"
#include "coordinator.h"

int main(int argc, char *argv[]) {
    /**
     * each cluster sent/received has the form
     * (int *) c, where:
     * 
     * - c[0] = coordinator rank
     * - c[1] = no. workers
     * - c[2..] = workers' ranks
     */
    std::vector<int *> clusters;

    int numTasks, rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // coordinators (0, 1, 2)
    // vs workers
    if (rank == 0) {
        int err = atoi(argv[2]);
        int N = atoi(argv[1]);

        if (err == 0) {
            findTopology("cluster0.txt", clusters, rank);
            rootCompute(clusters, rank, N, numTasks);
        } else if (err == 1) {
            bonusTopology("cluster0.txt", clusters, rank);
            rootBonusCompute(clusters, rank, N, numTasks);
        }

    } else if (rank == 1) {
        int err = atoi(argv[2]);

        if (err == 0) {
            findTopology("cluster1.txt", clusters, rank);
            // COMPUTE
            coordinatorCompute(clusters, rank, numTasks);
        } else if (err == 1) {
            bonusTopology("cluster1.txt", clusters, rank);
            coordinator1Compute(clusters, rank, numTasks);
        }

    } else if (rank == 2) {
        int err = atoi(argv[2]);

        if (err == 0) {
            findTopology("cluster2.txt", clusters, rank);
            // COMPUTE
            coordinatorCompute(clusters, rank, numTasks);
        } else if (err == 1) {
            bonusTopology("cluster2.txt", clusters, rank);
            coordinator2Compute(clusters, rank, numTasks);
        }

    } else {
        // worker
        int numClusters;
        MPI_Status status;
        
        // receive the no. clusters from the coordinator
        // initially, the coordinator is unknown
        //   --> MPI_ANY_SOURCE
        MPI_Recv(&numClusters, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

        int coordRank = status.MPI_SOURCE;
        
        // receive the clusters' data
        int i, nc;
        for (i = 0; i < numClusters; ++i) {
            MPI_Recv(&nc, 1, MPI_INT, coordRank, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int *cluster = new int[nc + 2];
            MPI_Recv(cluster, nc + 2, MPI_INT, coordRank, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            clusters.push_back(cluster);
        }

        getTopology(rank, clusters);

        // COMPUTE
        int len;

        MPI_Recv(&len, 1, MPI_INT, coordRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *arr = new int[len];
        MPI_Recv(arr, len, MPI_INT, coordRank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (i = 0; i < len; ++i) {
            arr[i] *= 2;
        }

        MPI_Send(arr, len, MPI_INT, coordRank, 5, MPI_COMM_WORLD);
        displayMessage(rank, coordRank);
    }

    MPI_Finalize();

    return 0;
}
