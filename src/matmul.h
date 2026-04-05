#ifndef MATMUL_H
#define MATMUL_H

#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <sys/time.h>
#define RIO_STB_ALLOC_IMPLEMENTATION
#include "rio_stb_alloc.h"

#define MAX_MAT_SIZE 2500

typedef struct {
    uint64_t rows;
    uint64_t cols;
    double* data;
} matrix;

size_t run_sim_seq(arena_t allocator[static 1], uint64_t n);
size_t run_sim_parallel(arena_t allocator[static 1], uint64_t n, size_t threads);
matrix init_matrix(arena_t allocator[static 1], uint64_t n, double value);
size_t measure(arena_t allocator[static 1], uint64_t n, size_t threads);

#endif // MATMUL_H

#ifdef MATMUL_IMPLEMENTATION

size_t run_sim_seq(arena_t allocator[static 1], uint64_t n) {
	struct timeval  tstart, tend;
    matrix a = init_matrix(allocator, n, 1);
    matrix b = init_matrix(allocator, n, 1);
    matrix c = init_matrix(allocator, n, 0);

	gettimeofday(&tstart, NULL);

	for (uint64_t i = 0; i < n; ++i) {
		for(uint64_t j = 0; j < n; ++j) {
			for (uint64_t k = 0; k < n; ++k){
                c.data[j + i * n] = c.data[j + i * n] + a.data[k + i * n] * b.data[j + k * n];
					// c[i][j] = c[i][j] + a[i][k] * b[k][j];
			}
		}
	}
	gettimeofday(&tend, NULL);
	arena_reset_ptr(allocator);
	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

size_t run_sim_parallel(arena_t allocator[static 1], uint64_t n, size_t threads) {
	struct timeval  tstart, tend;
    matrix a = init_matrix(allocator, n, 1);
    matrix b = init_matrix(allocator, n, 1);
    matrix c = init_matrix(allocator, n, 0);

	gettimeofday(&tstart, NULL);
	#pragma omp parallel num_threads(threads)
	{
        #pragma omp for
		for (uint64_t i = 0; i < n; ++i) {
			for(uint64_t j = 0; j < n; ++j) {
				for (uint64_t k = 0; k < n; ++k){
                    c.data[j + i * n] = c.data[j + i * n] + a.data[k + i * n] * b.data[j + k * n];
					// c[i][j] = c[i][j] + a[i][k] * b[k][j];
				}

			}
		}
		
	}
	gettimeofday(&tend, NULL);
	arena_reset_ptr(allocator);
	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

matrix init_matrix(arena_t allocator[static 1], uint64_t n, double value) {
    double* data = arena_alloc(allocator, .n_elements = n*n, .size_of = sizeof(double));
    matrix m = (matrix){
        .rows = n,
        .cols = n,
        .data = data
    };

	for (uint64_t i = 0; i < m.rows; ++i) {
		for(uint64_t j = 0; j < m.cols; ++j) {
            data[j + i * m.cols] = value;
		}
	}

    return m;
}

size_t measure(arena_t allocator[static 1], uint64_t n, size_t threads) {
    if (threads == 1) {
        printf("Matrix[%ld] Sequential Execution (Threads: %d)\n", n, 1);
        return run_sim_seq(allocator, n);
    }

    printf("Matrix[%ld] Parallel Execution (Threads: %d)\n", n, threads);
    return run_sim_parallel(allocator, n, threads);
}

#endif // MATMUL_IMPLEMENTATION
