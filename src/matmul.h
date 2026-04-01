#ifndef MATMUL_H
#define MATMUL_H

#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <sys/time.h>


#define MAX_MAT_SIZE 2500
double a[MAX_MAT_SIZE][MAX_MAT_SIZE];
double b[MAX_MAT_SIZE][MAX_MAT_SIZE];
double c[MAX_MAT_SIZE][MAX_MAT_SIZE];

size_t run_sim(size_t N);
void init_matrix(size_t N);
#endif // MATMUL_H

#ifdef MATMUL_IMPLEMENTATION

size_t run_sim(size_t N) {
	struct timeval  tstart, tend;

	#pragma omp parallel	
	{
		gettimeofday(&tstart, NULL);
		#pragma omp for
		for (size_t i=0; i<N;i++) {
			for(size_t j=0; j<N;j++) {
				for (size_t k=0; k<N; k++){
					c[i][j] = c[i][j] + a[i][k] * b[k][j];
				}

			}
		}
		gettimeofday(&tend, NULL);
	}

	#ifndef _OPENMP
		gettimeofday(&tstart, NULL);
		for (size_t i=0; i<N;i++) {
			for(size_t j=0; j<N;j++) {
				for (size_t k=0; k<N; k++){
					c[i][j] = c[i][j] + a[i][k] * b[k][j];
				}

			}
		}
		gettimeofday(&tend, NULL);
	#endif

	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

void init_matrix(size_t N) {
	// init matrix
	for (size_t i=0; i<N;i++) {
		for(size_t j=0; j<N;j++) {
			c[i][j] = 0;
			a[i][j] = 1;
			b[i][j] = 1;
		}
	}
}

#endif MATMUL_IMPLEMENTATION
