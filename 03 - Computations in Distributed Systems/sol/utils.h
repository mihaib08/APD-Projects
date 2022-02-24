#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <algorithm>

#include <errno.h>
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

void getTopology(int rank, std::vector<int *>& clusters);

void displayMessage(int from, int to);

#endif
