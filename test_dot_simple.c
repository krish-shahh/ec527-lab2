#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define IDENT 0.0
#define OP +

typedef double data_t;

/* Function to compute dot product */
void dot_product_simple(data_t *A, data_t *B, long int length, data_t *result) {
    long int i;
    data_t acc = IDENT;
    for (i = 0; i < length; i++) {
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

    dot_product_simple(A, B, length, &result);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    double elapsed_time = interval(time_start, time_stop);

    printf("Dot Product Result: %f\n", result);
    printf("Execution Time: %f seconds\n", elapsed_time);

    free(A);
    free(B);
    return 0;
}
