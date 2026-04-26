#ifndef SIM_H
#define SIM_H

/*
 * Use this file for the simulation exercise, after the
 * simulation create a copy with the exercise name
 * cp sim.h exercise_name.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

#define DIM 3

size_t run_sim_seq(uint64_t n_unsed);
size_t run_sim_parallel(uint64_t n_unsed, size_t threads);
size_t measure(uint64_t n, size_t threads);
double mean(size_t total, size_t runs);
void read_input(int k, int n, double *mean, double *x, int *cluster);
void allocate_memory(int k, int n, double **x, double **mean, double **sum, int **cluster, int **count);
void free_memory(double *x, double *mean, double *sum, int *cluster, int *count);

#endif // SIM_H

#ifdef SIM_IMPLEMENTATION

size_t run_sim_seq(uint64_t n_unsed) {
	struct timeval  tstart, tend;

    int k, n;
	double dmin, dx;
	double *x, *mean, *sum;
	int *cluster, *count, color;
	int flips;

    freopen("input.txt", "r", stdin);
	scanf("%d", &k);
	scanf("%d", &n);

	allocate_memory(k, n, &x, &mean, &sum, &cluster, &count);

	read_input(k, n, mean, x, cluster);

    flips = n;

	gettimeofday(&tstart, NULL);
	
	while (flips>0) {
		flips = 0;

		for (int j = 0; j < k; j++) {
			count[j] = 0; 
			for (int i = 0; i < DIM; i++) 
				sum[j*DIM+i] = 0.0;
		}

		for (int i = 0; i < n; i++) {
			dmin = -1; color = cluster[i];
			for (int c = 0; c < k; c++) {
				dx = 0.0;
				for (int j = 0; j < DIM; j++) 
					dx +=  (x[i*DIM+j] - mean[c*DIM+j])*(x[i*DIM+j] - mean[c*DIM+j]);
				if (dx < dmin || dmin == -1) {
					color = c;
					dmin = dx;
				}
			}
		
            if (cluster[i] != color) {
				flips++;
				cluster[i] = color;
	      	}
		}

	    for (int i = 0; i < n; i++) {
			count[cluster[i]]++;
			for (int j = 0; j < DIM; j++) 
				sum[cluster[i]*DIM+j] += x[i*DIM+j];
		}
		
        for (int i = 0; i < k; i++) {
			for (int j = 0; j < DIM; j++) {
				mean[i*DIM+j] = sum[i*DIM+j]/count[i];
  			}
		}
	}
	gettimeofday(&tend, NULL);

    for (int i = 0; i < k; i++) {
		for (int j = 0; j < DIM; j++)
			printf("%5.2f ", mean[i*DIM+j]);
		printf("\n");
	}
	#ifdef DEBUG
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < DIM; j++)
			printf("%5.2f ", x[i*DIM+j]);
		printf("%d\n", cluster[i]);
	}
	#endif

	free_memory(x, mean, sum, cluster, count);

	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}

size_t run_sim_parallel(uint64_t n_unsed, size_t threads) {
	struct timeval  tstart, tend;


    int k, n;
	double dmin, dx;
	double *x, *mean, *sum;
	int *cluster, *count, color;
	int flips;
	
    freopen("input.txt", "r", stdin);

    scanf("%d", &k);
	scanf("%d", &n);

	allocate_memory(k, n, &x, &mean, &sum, &cluster, &count);

	double *local_sum = malloc(threads * k * DIM * sizeof(double));
	int *local_count = malloc(threads * k * sizeof(int));
	
	read_input(k, n, mean, x, cluster);

    flips = n;
    omp_set_num_threads(threads);

	gettimeofday(&tstart, NULL);
	
	while (flips>0) {
		flips = 0;
		// init centroids
		#pragma omp parallel for
        for (int j = 0; j < k; j++) {
			count[j] = 0; 
			for (int i = 0; i < DIM; i++) 
				sum[j*DIM+i] = 0.0;
		}

        #pragma omp parallel for private( dx, dmin, color) schedule(dynamic, 1000) reduction(+:flips)
        for (int i = 0; i < n; i++) {
			dmin = -1; color = cluster[i];
			for (int c = 0; c < k; c++) {
				dx = 0.0;
				for (int j = 0; j < DIM; j++) 
					dx +=  (x[i*DIM+j] - mean[c*DIM+j])*(x[i*DIM+j] - mean[c*DIM+j]);
				if (dx < dmin || dmin == -1) {
					color = c;
					dmin = dx;
				}
			}
			if (cluster[i] != color) {
				flips++;
				cluster[i] = color;
	      	}
		}

		memset(local_sum, 0, sizeof(double) * threads * k * DIM);
		memset(local_count, 0, sizeof(int) * threads * k);

		#pragma omp parallel 
		{
			int tid = omp_get_thread_num();

			#pragma omp for schedule(dynamic, 1000)
			for (int i = 0; i < n; i++) {
				int c = cluster[i];
				local_count[tid*k+c]++;

				for (int j = 0; j < DIM; j++) 
					local_sum[tid*k*DIM + c*DIM + j] += x[i*DIM + j];
			}
		}


		for (int t = 0; t < threads; t++) {
			for (int c = 0; c < k; c++) {
				count[c] += local_count[t*k + c];

				for (int j = 0; j < DIM; j++) {
					sum[c*DIM + j] += local_sum[t*k*DIM + c*DIM + j];
				}
			}
		}

        #pragma omp parallel for
		for (int i = 0; i < k; i++) {
			for (int j = 0; j < DIM; j++) {
				mean[i*DIM+j] = sum[i*DIM+j]/count[i];
  			}
		}
	}
	gettimeofday(&tend, NULL);

    for (int i = 0; i < k; i++) {
		for (int j = 0; j < DIM; j++)
			printf("%5.2f ", mean[i*DIM+j]);
		printf("\n");
	}

	free_memory(x, mean, sum, cluster, count);	
	free(local_sum);
	free(local_count);
	
	return ((tend.tv_sec*1000000 + tend.tv_usec) - (tstart.tv_sec * 1000000 + tstart.tv_usec));
}


size_t measure(uint64_t n, size_t threads) {
    if (threads == 1) {
        return run_sim_seq(n);
    }

    return run_sim_parallel(n, threads);
}

double mean(size_t total, size_t runs) {
    return (double)total / (double)runs;
}


void read_input(int k, int n, double *mean, double *x, int *cluster) {
    for (int i = 0; i < n; i++) 
        cluster[i] = 0;

    for (int i = 0; i < k; i++)
        scanf("%lf %lf %lf",
              mean + i*DIM,
              mean + i*DIM + 1,
              mean + i*DIM + 2);

    for (int i = 0; i < n; i++)
        scanf("%lf %lf %lf",
              x + i*DIM,
              x + i*DIM + 1,
              x + i*DIM + 2);
}

void allocate_memory(int k, int n,
                     double **x, double **mean, double **sum,
                     int **cluster, int **count) {
    
    *x = (double *)malloc(sizeof(double) * DIM * n);
    *mean = (double *)malloc(sizeof(double) * DIM * k);
    *sum = (double *)malloc(sizeof(double) * DIM * k);
    *cluster = (int *)malloc(sizeof(int) * n);
    *count = (int *)malloc(sizeof(int) * k);
}

void free_memory(double *x, double *mean, double *sum, int *cluster, int *count) {
	free(x);
	free(mean);
	free(sum);
	free(cluster);
	free(count);
}

#endif // SIM_IMPLEMENTATION
