#ifndef UTILS_H
#define UTILS_H

#include "individual.h"
#include "sack_object.h"
#include <pthread.h>

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

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/**
 * argument structure to be passed 
 * to the thread function
*/
struct th_arg {
    int id;

    individual *current_generation;
    individual *next_generation;

    int generations_count;

    int object_count;
    const sack_object *objects;
    int sack_capacity;

    int P; // no. threads
    pthread_barrier_t *barrier; // one barrier / program

    int *st; // start indices for each thread
    int *en; // end indices for each thread
};

#endif
