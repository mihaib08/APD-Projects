#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <string>

void findTopology(const std::string in, std::vector<int *>& clusters, int rank);

void rootCompute(std::vector<int *>& clusters, int rank, int N, int numTasks);
void coordinatorCompute(std::vector<int *>& clusters, int rank, int numTasks);

// BONUS
void bonusTopology(const std::string in, std::vector<int *>& clusters, int rank);

void rootBonusCompute(std::vector<int *>& clusters, int rank, int N, int numTasks);
void coordinator1Compute(std::vector<int *>& clusters, int rank, int numTasks);
void coordinator2Compute(std::vector<int *>& clusters, int rank, int numTasks);

#endif
