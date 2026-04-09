#define SIM_IMPLEMENTATION
#include "sim.h"


int main(int argc, char* argv[]) {
    const uint64_t N[] = {100, 500, 1000, 2000, 4000} ;
    size_t diff_sizes = sizeof(N) / sizeof(N[0]);
    arena_t allocator = mk_arena_allocator(MB(600));
    size_t runs;
    if (argc <= 0) runs = 1;

    runs = atoi(argv[1]);

    const size_t max_threads = 6;

    FILE* f = fopen("speedup.csv", "w");
    fprintf(f, "n_row,n_col,threads,average_time,speedup,total_runs\n");

    for (size_t i = 0; i < diff_sizes; ++i) {
        uint64_t n = N[i];

        // baseline (1 thread)
        size_t total_t1 = 0;
        for (size_t r = 0; r < runs; ++r)
            total_t1 += measure(&allocator, n, 1);

        double t1 = mean(total_t1, runs);
        fprintf(f, "%ld,%ld,%d,%f,%f,%ld\n", n, n, 1, t1, 1.0, runs);
        for (size_t t = 2; t <= max_threads; t++) {
            size_t total_tn = 0;

            for (size_t r = 0; r < runs; ++r)
                total_tn += measure(&allocator, n, t);

            double tn = mean(total_tn, runs);
            double speedup = t1/tn;
            fprintf(f, "%ld,%ld,%ld,%f,%f,%ld\n", n, n, t, tn, speedup, runs);
        }
    }

    fclose(f);
    arena_destroy(&allocator);

	return 0;
}
