#define SIM_IMPLEMENTATION
#include "sim.h"


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <runs> <n>\n", argv[0]);
        return 1;
    }

    size_t runs = atoi(argv[1]);
    uint64_t n = atoll(argv[2]);

    const size_t max_threads = 6;

    FILE* f = fopen("speedup.csv", "w");
    fprintf(f, "n_row,n_col,threads,average_time,speedup,total_runs\n");

    // baseline (1 thread)
    size_t total_t1 = 0;
    for (size_t r = 0; r < runs; ++r)
        total_t1 += measure(n, 1);

    double t1 = mean(total_t1, runs);
    fprintf(f, "%ld,%ld,%d,%f,%f,%ld\n", n, n, 1, t1, 1.0, runs);

    for (size_t t = 2; t <= max_threads; t++) {
        size_t total_tn = 0;

        for (size_t r = 0; r < runs; ++r)
            total_tn += measure(n, t);

        double tn = mean(total_tn, runs);
        double speedup = t1 / tn;

        fprintf(f, "%ld,%ld,%ld,%f,%f,%ld\n", n, n, t, tn, speedup, runs);
    }

    fclose(f);
    return 0;
}
