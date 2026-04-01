
#define MATMUL_IMPLEMENTATION
#include "matmul.h"

int main(void) {
 size_t N[] = {100, 500, 1000, 2000} ;
 size_t runs = 3;
 size_t results[4] = {0};

	for (size_t i=0; i<runs;i++) {
        size_t n = N[i];
        init_matrix(n);
		results[0] += run_sim(n);
        results[1] += run_sim(n);
		results[2] += run_sim(n);
		results[3] += run_sim(n);
	}


    for (size_t i = 0; i < 4; i++) {
        double mean = (double)results[i] / (double)runs;
        printf("Matrix[%ld]: %f microseconds\n", N[i], mean);
    }

	return 0;

}
