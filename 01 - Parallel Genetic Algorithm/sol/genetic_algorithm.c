#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "genetic_algorithm.h"

#include "utils.h"

int read_input(int *P, sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count P\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	*P = (int) strtol(argv[3], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = 0; i < object_count; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void *thread_genetic(void *arg) {
	struct th_arg *data = (struct th_arg *)arg;

	int id = data->id;
	int P = data->P;
	int object_count = data->object_count;

	// set the iteration limits / thread
	int start = data->st[id];
	int end = data->en[id];
	int diff = end - start;

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; ++i) {
		data->current_generation[i].fitness = 0;
		data->current_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		data->current_generation[i].chromosomes[i] = 1;
		data->current_generation[i].index = i;
		data->current_generation[i].chromosome_length = object_count;

		data->next_generation[i].fitness = 0;
		data->next_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		data->next_generation[i].index = i;
		data->next_generation[i].chromosome_length = object_count;
	}

	pthread_barrier_wait(data->barrier);

	int cursor, count;

	// iterate for each generation
	for (int k = 0; k < data->generations_count; ++k) {
		if (id == 0 && k % 5 == 0) {
			print_best_fitness(data->next_generation);
		}

		pthread_barrier_wait(data->barrier);

		cursor = 0;

		// compute fitness and sort by it
		compute_fitness_function(data->objects, data->current_generation + start, diff, data->sack_capacity);
		qsort(data->current_generation + start, diff, sizeof(individual), cmpfunc);

		pthread_barrier_wait(data->barrier);

		// (*) _merge_ the sorted parts of data->current_generation
		//                            into data->next_generation
		// do this on only _one_ thread
		if (id == 0) {
			int curr_pos[P];
			int l, t;

			/**
			 * merge P _sorted_ arrays
			 * array[l] = current_generation[data->st[l]..data->en[l]]
			 */
			for (l = 0; l < P; ++l) {
				// current maximum element of array[l]
				// to be added to the merged result
				curr_pos[l] = data->st[l];
			}
			for (t = 0; t < object_count; ++t) {
				// find the next best element to be added
				int maxi = -1;
				int pos;
				for (l = 0; l < P; ++l) {
					if (data->current_generation[curr_pos[l]].fitness > maxi) {
						maxi = data->current_generation[curr_pos[l]].fitness;
						pos = l;
					}
				}

				copy_individual(data->current_generation + curr_pos[pos], data->next_generation + t);
				data->next_generation[t].fitness = data->current_generation[curr_pos[pos]].fitness;
				
				// move to the next element of array[pos] 
				++curr_pos[pos];

				if (curr_pos[pos] == data->en[pos]) {
					--curr_pos[pos];
					// don't consider curr_pos[pos] for the
					// next element to be merged
					data->current_generation[curr_pos[pos]].fitness = -1;
				}
			}
		}

		pthread_barrier_wait(data->barrier);

		// keep first 30% children (elite children selection)
		int s, e;
		count = object_count * 3 / 10;

		s = id * ((double) count / P);
		e = MIN(count, ((id + 1) * (double) count / P));

		for (int i = s; i < e; ++i) {
			copy_individual(data->next_generation + i, data->current_generation + i);
		}

		cursor = count;
		// mutate first 20% children with the first version of bit string mutation
		count = object_count * 2 / 10;

		s = id * ((double) count / P);
		e = MIN(count, ((id + 1) * (double) count / P));

		for (int i = s; i < e; ++i) {
			copy_individual(data->next_generation + i, data->current_generation + cursor + i);
			mutate_bit_string_1(data->current_generation + cursor + i, k);
		}
		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		count = object_count * 2 / 10;
		for (int i = s; i < e; ++i) {
			copy_individual(data->next_generation + i + count, data->current_generation + cursor + i);
			mutate_bit_string_2(data->current_generation + cursor + i, k);
		}
		cursor += count;

		pthread_barrier_wait(data->barrier);

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		count = object_count * 3 / 10;
		
		// only do the copying for one thread
		if (count % 2 == 1 && id == 0) {
			copy_individual(data->next_generation + object_count - 1, data->current_generation + cursor + count - 1);
			count--;
		}

		pthread_barrier_wait(data->barrier);

		s = id * ((double) count / P);
		e = MIN(count, ((id + 1) * (double) count / P));
		
		// check the limits' parity
		if (s % 2 == 1) {
			--s;
		}
		if (e % 2 == 1) {
			--e;
		}

		for (int i = s; i < e; i += 2) {
			crossover(data->next_generation + i, data->current_generation + cursor + i, k);
		}

		for (int i = start; i < end; ++i) {
			data->current_generation[i].index = i;
		}
	}

	pthread_barrier_wait(data->barrier);

	compute_fitness_function(data->objects, data->current_generation + start, diff,  data->sack_capacity);
	qsort(data->current_generation + start, (end - start), sizeof(individual), cmpfunc);

	pthread_exit(NULL);
}

void run_genetic_algorithm(int P, const sack_object *objects, int object_count, int generations_count, int sack_capacity)
{
	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
	int i, r;

	int *st = (int *) calloc(P, sizeof(int));
	int *en = (int *) calloc(P, sizeof(int));

	struct th_arg *args = (struct th_arg*) calloc(P, sizeof(struct th_arg));
	
	// create the barrier
	pthread_barrier_t barrier;
	r = pthread_barrier_init(&barrier, NULL, P);

	DIE(r != 0, "Couldn't initialise the barrier");

	// initialise the threads
	pthread_t tid[P];

	for (i = 0; i < P; ++i) {
		st[i] = i * ((double) object_count / P);
		en[i] = MIN(object_count, (i + 1) * ((double) object_count / P));
	}

	for (i = 0; i < P; ++i) {
		args[i].id = i;
		args[i].P = P;
		args[i].barrier = &barrier;

		args[i].current_generation = current_generation;
		args[i].next_generation = next_generation;
		args[i].generations_count = generations_count;

		args[i].object_count = object_count;
		args[i].objects = objects;
		args[i].sack_capacity = sack_capacity;

		args[i].st = st;
		args[i].en = en;

		r = pthread_create(&tid[i], NULL, thread_genetic, &args[i]);
		DIE(r != 0, "Error while creating the thread");
	}

	for (i = 0; i < P; ++i) {
		r = pthread_join(tid[i], NULL);
		DIE(r != 0, "Error while waiting for the thread");
	}

	int l;
		
	// find the current maximum fitness
	int maxi = -1;
	for (l = 0; l < P; ++l) {
		if (current_generation[st[l]].fitness > maxi) {
			maxi = current_generation[st[l]].fitness;
		}
	}

	printf("%d\n", maxi);

	// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);

	r = pthread_barrier_destroy(&barrier);
	DIE(r != 0, "Couldn't destroy the barrier");
}
