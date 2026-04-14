#ifndef SIM_H
#define SIM_H

/*
 * Use this file for the simulation exercise, after the
 * simulation create a copy with the exercise name
 * cp sim.h exercise_name.h
 */

#include <stdio.h>
#include <sys/time.h>
#define RIO_STB_ALLOC_IMPLEMENTATION
#include "rio_stb_alloc.h"


typedef struct {
    uint64_t rows;
    uint64_t cols;
    double* data;
} matrix;

typedef struct {
    double* data;
    uint64_t n;
} vector;

size_t run_sim_seq(arena_t allocator[static 1], uint64_t n);
size_t run_sim_parallel(arena_t allocator[static 1], uint64_t n, size_t threads);
vector* init_random_vector(arena_t allocator[static 1], uint64_t n);
// populates the matrix with the same value
matrix* init_matrix(arena_t allocator[static 1], uint64_t n, double value);
size_t measure(arena_t allocator[static 1], uint64_t n, size_t threads);
double mean(size_t total, size_t runs);

#endif // SIM_H

#ifdef SIM_IMPLEMENTATION

size_t run_sim_seq(arena_t allocator[static 1], uint64_t n) {
	struct timeval  tstart, tend;
    vector* x = init_random_vector(allocator, n);

	gettimeofday(&tstart, NULL);

    double max_local = 0.0;
    uint64_t maxloc_local = 0;
	for (uint64_t i = 0; i < n; ++i) {
        if(x->data[i] > max_local) {
            max_local = x->data[i];
            maxloc_local = i;
        }
	}

	gettimeofday(&tend, NULL);
	arena_reset_ptr(allocator);
	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

size_t run_sim_parallel(arena_t allocator[static 1], uint64_t n, size_t threads) {
    struct timeval  tstart, tend;
    vector* x = init_random_vector(allocator, n);
    double maxval = 0.0;
    uint64_t maxloc = 0;
    uint64_t count = 0;

    gettimeofday(&tstart, NULL);
#pragma omp parallel
{
    double max_local = 0.0;
    uint64_t maxloc_local = 0;
    #pragma omp for
    for (uint64_t i = 0; i < n; ++i) {
        if(x->data[i] > max_local) {
            max_local = x->data[i];
            maxloc_local = i;
        }
	}

    #pragma omp critical
    {
        if(max_local > maxval) {
            maxval = max_local;
            maxloc = maxloc_local;
        }
    }

    #pragma omp for reduction(+:count)
    for (uint64_t i = 0; i < n; ++i)
        if (x->data[i] == maxval) count++;
}
    gettimeofday(&tend, NULL);
	arena_reset_ptr(allocator);
    printf("\t Max count: %ld\n", count);
	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

vector* init_random_vector(arena_t allocator[static 1], uint64_t n) {
    vector* vec = arena_alloc(allocator, vector, 1);
    double* data = arena_alloc(allocator, double, n);
    assert(data);

    *vec = (vector){
        .data = data,
        .n = n
    };

    for (uint64_t i = 0; i < vec->n; ++i) {
		vec->data[i] = rand() % 999;
	}

    return vec;
}

matrix* init_matrix(arena_t allocator[static 1], uint64_t n, double value) {
    matrix* m = arena_alloc(allocator, matrix, 1);
    double* data = arena_alloc(allocator, double, n * n);
    assert(data);

    *m = (matrix){
        .rows = n,
        .cols = n,
        .data = data
    };

    for (uint64_t i = 0; i < m->rows; ++i) {
		for(uint64_t j = 0; j < m->cols; ++j) {
            m->data[j + i * m->cols] = value;
		}
	}

    return m;
}

size_t measure(arena_t allocator[static 1], uint64_t n, size_t threads) {
    if (threads == 1) {
        printf("Matrix[%ld] Sequential Execution (Threads: %d)\n", n, 1);
        return run_sim_seq(allocator, n);
    }

    printf("Matrix[%ld] Parallel Execution (Threads: %ld)\n", n, threads);
    return run_sim_parallel(allocator, n, threads);
}

double mean(size_t total, size_t runs) {
    return (double)total / (double)runs;
}

#endif // SIM_IMPLEMENTATION
