#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define IDENT 0.0
#define OP +

typedef double data_t;

/* Optimized dot product with loop unrolling and multiple accumulators */
void dot_product_optimized(data_t *A, data_t *B, long int length, data_t *result) {
    long int i;
    long int limit = length - 9; // Unrolling factor of 10
    data_t acc0 = IDENT, acc1 = IDENT, acc2 = IDENT, acc3 = IDENT, acc4 = IDENT;
    data_t acc5 = IDENT, acc6 = IDENT, acc7 = IDENT, acc8 = IDENT, acc9 = IDENT;

    for (i = 0; i < limit; i += 10) {
        acc0 = acc0 OP (A[i] * B[i]);
        acc1 = acc1 OP (A[i+1] * B[i+1]);
        acc2 = acc2 OP (A[i+2] * B[i+2]);
        acc3 = acc3 OP (A[i+3] * B[i+3]);
        acc4 = acc4 OP (A[i+4] * B[i+4]);
        acc5 = acc5 OP (A[i+5] * B[i+5]);
        acc6 = acc6 OP (A[i+6] * B[i+6]);
        acc7 = acc7 OP (A[i+7] * B[i+7]);
        acc8 = acc8 OP (A[i+8] * B[i+8]);
        acc9 = acc9 OP (A[i+9] * B[i+9]);
    }

    /* Reduce accumulators */
    data_t acc = acc0 OP acc1 OP acc2 OP acc3 OP acc4 OP acc5 OP acc6 OP acc7 OP acc8 OP acc9;

    /* Handle remaining elements */
    for (; i < length; i++) {
        acc = acc OP (A[i] * B[i]);
    }

    *result = acc;
}

/* Function to generate random arrays */
void generate_arrays(data_t *A, data_t *B, long int length) {
    for (long int i = 0; i < length; i++) {
        A[i] = (data_t)(rand() % 100) / 10.0;
        B[i] = (data_t)(rand() % 100) / 10.0;
    }
}

/* Timing function */
double interval(struct timespec start, struct timespec end) {
    struct timespec temp;
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    if (temp.tv_nsec < 0) {
        temp.tv_sec -= 1;
        temp.tv_nsec += 1000000000;
    }
    return ((double)temp.tv_sec + (double)temp.tv_nsec * 1.0e-9);
}

/* Main function */
int main() {
    long int length = 1000000; // Size of arrays
    data_t *A = (data_t *)malloc(length * sizeof(data_t));
    data_t *B = (data_t *)malloc(length * sizeof(data_t));
    data_t result = 0.0;

    generate_arrays(A, B, length);

    struct timespec time_start, time_stop;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);

    dot_product_optimized(A, B, length, &result);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    double elapsed_time = interval(time_start, time_stop);

    printf("Dot Product Result: %f\n", result);
    printf("Execution Time: %f seconds\n", elapsed_time);

    free(A);
    free(B);
    return 0;
}
