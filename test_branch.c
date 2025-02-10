/***************************************************************************

  gcc -O1 test_branch.c -lrt -o test_branch

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define CPNS 2.0    /* Cycles per nanosecond */

#define A   1  /* coefficient of x^2 */
#define B   1  /* coefficient of x */
#define C   10 /* constant term */

#define NUM_TESTS 10   /* Number of different sizes to test */
#define OUTER_LOOPS 2000
#define OPTIONS 4
#define IDENT 1.0
#define OP *

typedef float data_t;

/* Create abstract data type for an array in memory */
typedef struct {
  long int len;
  data_t *data;
} array_rec, *array_ptr;

/* Function prototypes */
array_ptr new_array(long int len);
int set_array_length(array_ptr v, long int index);
int init_array_pred(array_ptr v, long int len);
int init_array_unpred(array_ptr v, long int len);
double fRand(double fMin, double fMax);
void branch1(array_ptr v0, array_ptr v1, array_ptr v2, long int outer_limit);
void branch2(array_ptr v0, array_ptr v1, array_ptr v2, long int outer_limit);

/* Time measurement function */
double interval(struct timespec start, struct timespec end) {
  struct timespec temp;
  temp.tv_sec = end.tv_sec - start.tv_sec;
  temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  if (temp.tv_nsec < 0) {
    temp.tv_sec -= 1;
    temp.tv_nsec += 1000000000;
  }
  return ((double)temp.tv_sec + (double)temp.tv_nsec) * 1.0e-9;
}

/* Create a new array of the specified length */
array_ptr new_array(long int len) {
  array_ptr result = (array_ptr) malloc(sizeof(array_rec));
  if (!result) return NULL;
  result->len = len;

  if (len > 0) {
    result->data = (data_t *) calloc(len, sizeof(data_t));
    if (!result->data) {
      free(result);
      return NULL;
    }
  } else {
    result->data = NULL;
  }
  return result;
}

/* Set length of the array */
int set_array_length(array_ptr v, long int index) {
  v->len = index;
  return 1;
}

/* Initialize an array with a "predictable" pattern */
int init_array_pred(array_ptr v, long int len) {
  if (len > 0) {
    v->len = len;
    for (long int i = 0; i < len; i++) {
      v->data[i] = 5.0;  /* Constant value ensures predictable behavior */
    }
    return 1;
  }
  return 0;
}

/* Initialize an array with an "unpredictable" pattern */
int init_array_unpred(array_ptr v, long int len) {
  if (len > 0) {
    v->len = len;
    for (long int i = 0; i < len; i++) {
      v->data[i] = fRand(0.0, 10.0);  /* Random values ensure unpredictability */
    }
    return 1;
  }
  return 0;
}

/* Returns a random number in the range [fMin, fMax) */
double fRand(double fMin, double fMax) {
  double f = (double)rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

/*************************************************/
/* branch1: Implements branch-based conditional execution */
void branch1(array_ptr v0, array_ptr v1, array_ptr v2, long int outer_limit) {
  long int i, j;
  long int length = v0->len;
  data_t *data0 = v0->data;
  data_t *data1 = v1->data;
  data_t *data2 = v2->data;

  for (j = 0; j < outer_limit; j++) {
    for (i = 0; i < length; i++) {
      if (data0[i] > data1[i]) {
        data2[i] = data0[i];
      } else {
        data2[i] = data1[i];
      }
    }
  }
}

/*************************************************/
/* branch2: Uses conditional move instead of branching */
void branch2(array_ptr v0, array_ptr v1, array_ptr v2, long int outer_limit) {
  long int i, j;
  long int length = v0->len;
  data_t *data0 = v0->data;
  data_t *data1 = v1->data;
  data_t *data2 = v2->data;

  for (j = 0; j < outer_limit; j++) {
    for (i = 0; i < length; i++) {
      data2[i] = (data0[i] > data1[i]) ? data0[i] : data1[i];
    }
  }
}

/*****************************************************************************/
int main(int argc, char *argv[]) {
  int OPTION;
  struct timespec time_start, time_stop;
  double time_stamp[OPTIONS][NUM_TESTS];
  long int x, n, alloc_size;

  printf("Branch optimization examples\n");

  x = NUM_TESTS - 1;
  alloc_size = A * x * x + B * x + C;

  printf("Testing 4 different ways, on arrays of %d sizes from %d to %ld\n",
    NUM_TESTS, C, alloc_size);

  /* Create array data structures */
  array_ptr v0 = new_array(alloc_size);
  array_ptr v1 = new_array(alloc_size);
  array_ptr v2 = new_array(alloc_size);

  /* Execute and time all options */

  // Branch1 with predictable data
  OPTION = 0;
  init_array_pred(v0, alloc_size);
  init_array_pred(v1, alloc_size);
  printf("testing option %d: branch1() on predictable data\n", OPTION);
  for (x = 0; x < NUM_TESTS && (n = A * x * x + B * x + C, n <= alloc_size); x++) {
    set_array_length(v0, n);
    set_array_length(v1, n);
    set_array_length(v2, n);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    branch1(v0, v1, v2, OUTER_LOOPS);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    time_stamp[OPTION][x] = interval(time_start, time_stop);
  }

  // Branch1 with unpredictable data
  OPTION++;
  init_array_unpred(v0, alloc_size);
  init_array_unpred(v1, alloc_size);
  printf("testing option %d: branch1() on unpredictable data\n", OPTION);
  for (x = 0; x < NUM_TESTS && (n = A * x * x + B * x + C, n <= alloc_size); x++) {
    set_array_length(v0, n);
    set_array_length(v1, n);
    set_array_length(v2, n);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    branch1(v0, v1, v2, OUTER_LOOPS);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    time_stamp[OPTION][x] = interval(time_start, time_stop);
  }

  // Branch2 with predictable data
  OPTION++;
  init_array_pred(v0, alloc_size);
  init_array_pred(v1, alloc_size);
  printf("testing option %d: branch2() on predictable data\n", OPTION);
  for (x = 0; x < NUM_TESTS && (n = A * x * x + B * x + C, n <= alloc_size); x++) {
    set_array_length(v0, n);
    set_array_length(v1, n);
    set_array_length(v2, n);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    branch2(v0, v1, v2, OUTER_LOOPS);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    time_stamp[OPTION][x] = interval(time_start, time_stop);
  }

  // Branch2 with unpredictable data
  OPTION++;
  init_array_unpred(v0, alloc_size);
  init_array_unpred(v1, alloc_size);
  printf("testing option %d: branch2() on unpredictable data\n", OPTION);
  for (x = 0; x < NUM_TESTS && (n = A * x * x + B * x + C, n <= alloc_size); x++) {
    set_array_length(v0, n);
    set_array_length(v1, n);
    set_array_length(v2, n);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    branch2(v0, v1, v2, OUTER_LOOPS);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    time_stamp[OPTION][x] = interval(time_start, time_stop);
  }

  /* Output results */
  printf("\nsize, pred.branch1, unpred.branch1, pred.branch2, unpred.branch2\n");
  for (int i = 0; i < NUM_TESTS; i++) {
    printf("%ld,  ", (long)((A * i * i + B * i + C) * OUTER_LOOPS));
    for (int j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPNS) * 1.0e9 * time_stamp[j][i]));
    }
    printf("\n");
  }

  return 0;
}
