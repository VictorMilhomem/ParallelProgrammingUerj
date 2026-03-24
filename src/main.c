#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1000000000


int main(int argc, char **argv) {
    int i;
    double mypi, h, sum, x;

    h = 1.0/(double) N;
    sum = 0.0;

    #pragma omp parallel private(x, i) shared(h) reduction(+:sum) 
    {
        int num_threads = omp_get_num_threads();
        int id = omp_get_thread_num();
        int start = id * (N/num_threads) + 1;
        int end = (id == num_threads - 1) ? N : (id + 1) * (N/num_threads);

        for (i = start; i <= end; i++) {
            x = h * ((double) i - 0.5);
            sum += 4.0 / (1.0 + x*x);
        }
    }
    mypi = h * sum;

    printf("PI: %.16f\n", mypi);
    return 0;
}
