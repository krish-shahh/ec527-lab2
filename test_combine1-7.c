/****************************************************************************

 gcc -O1 test_combine1-7.c -lrt -o test_combine

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

/* We want to test a range of work sizes. We will generate these
   using the quadratic formula:  A x^2 + B x + C */
#define A   2   /* coefficient of x^2 */
#define B   3   /* coefficient of x */
#define C   20  /* constant term */

#define NUM_TESTS 10   /* Number of different sizes to test */

#define OUTER_LOOPS 2000

#define CPNS 2.0    /* Cycles per nanosecond */

#define OPTIONS 7

/* Type of operation. This can be multiplication or addition. */
#define IDENT 0.0
#define OP +

typedef double data_t;

/* Create abstract data type for an array in memory */
typedef struct {
  long int len;
  data_t *data;
} array_rec, *array_ptr;

/* Prototypes */
array_ptr new_array(long int len);
int init_array(array_ptr v, long int len);
long int get_array_length(array_ptr v);
data_t *get_array_start(array_ptr v);
void combine1(array_ptr v, data_t *dest);
void combine2(array_ptr v, data_t *dest);
void combine3(array_ptr v, data_t *dest);
void combine4(array_ptr v, data_t *dest);
void combine5_unrolled(array_ptr v, data_t *dest);
void combine6_unrolled(array_ptr v, data_t *dest);
void combine7_unrolled(array_ptr v, data_t *dest);

/* Timing functions */
double interval(struct timespec start, struct timespec end) {
  struct timespec temp;
  temp.tv_sec = end.tv_sec - start.tv_sec;
  temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  if (temp.tv_nsec < 0) {
    temp.tv_sec -= 1;
    temp.tv_nsec += 1000000000;
  }
  return temp.tv_sec + temp.tv_nsec * 1.0e-9;
}

/* Prevent CPU power-saving mode */
double wakeup_delay() {
  double meas = 0;
  int i, j = 100;
  struct timespec time_start, time_stop;
  double quasi_random = 0;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
  while (meas < 1.0) {
    for (i = 1; i < j; i++) {
      quasi_random = quasi_random * quasi_random - 1.923432;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
    meas = interval(time_start, time_stop);
    j *= 2;
  }
  return quasi_random;
}

/*****************************************************************************/
int main() {
  int OPTION;
  struct timespec time_start, time_stop;
  double time_stamp[OPTIONS][NUM_TESTS];
  double final_answer = 0;
  long int x, n, k, alloc_size;
  data_t *result;

  printf("Vector reduction (combine) examples\n");

  wakeup_delay();
  final_answer = wakeup_delay();

  x = NUM_TESTS - 1;
  alloc_size = A * x * x + B * x + C;

  array_ptr v0 = new_array(alloc_size);
  init_array(v0, alloc_size);
  result = (data_t *) malloc(sizeof(data_t));

  printf("Testing %d variants of combine(),\n", OPTIONS);
  printf("  on arrays of %d sizes from %d to %ld\n", NUM_TESTS, C, alloc_size);

  void (*combine_functions[OPTIONS])(array_ptr, data_t *) = {
    combine1, combine2, combine3, combine4, combine5_unrolled, combine6_unrolled, combine7_unrolled
  };

  for (OPTION = 0; OPTION < OPTIONS; OPTION++) {
    printf("testing option %d\n", OPTION);
    for (x = 0; x < NUM_TESTS && (n = A * x * x + B * x + C, n <= alloc_size); x++) {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
      for (k = 0; k < OUTER_LOOPS; k++) {
        combine_functions[OPTION](v0, result);
        final_answer += *result;
      }
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
      time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
  }

  printf("size, combine1, combine2, combine3, combine4, combine5, combine6, combine7\n");
  for (int i = 0; i < NUM_TESTS; i++) {
    printf("%d,  ", (A * i * i + B * i + C) * OUTER_LOOPS);
    for (int j = 0; j < OPTIONS; j++) {
      printf("%ld%s", (long int)(CPNS * 1.0e9 * time_stamp[j][i]), j == OPTIONS - 1 ? "\n" : ", ");
    }
  }

  printf("Sum of all results: %g\n", final_answer);
  free(result);
  return 0;
}

/**********************************************/
/* Create array of specified length */
array_ptr new_array(long int len) {
  array_ptr result = (array_ptr) malloc(sizeof(array_rec));
  if (!result) return NULL;
  result->len = len;
  result->data = (data_t *) calloc(len, sizeof(data_t));
  return result;
}

/* Get array length */
long int get_array_length(array_ptr v) {
  return v->len;
}

/* Initialize array */
int init_array(array_ptr v, long int len) {
  if (len > 0) {
    for (long int i = 0; i < len; i++) {
      v->data[i] = (data_t)(i + 1);
    }
    return 1;
  }
  return 0;
}

/* Get array start */
data_t *get_array_start(array_ptr v) {
  return v->data;
}

/*************************************************/
/* Combine1: Standard implementation */
void combine1(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  *dest = IDENT;
  for (long int i = 0; i < length; i++) {
    *dest = *dest OP v->data[i];
  }
}

/* Combine2: Direct access */
void combine2(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  data_t *data = get_array_start(v);
  *dest = IDENT;
  for (long int i = 0; i < length; i++) {
    *dest = *dest OP data[i];
  }
}

/* Combine3: Accumulate in local variable */
void combine3(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  data_t acc = IDENT;
  data_t *data = get_array_start(v);
  for (long int i = 0; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}

/* Combine4: Standard accumulation */
void combine4(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  data_t acc = IDENT;
  data_t *data = get_array_start(v);
  for (long int i = 0; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}

/* Combine5: Unroll loop by 10 */
void combine5_unrolled(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  data_t acc = IDENT;
  data_t *data = get_array_start(v);
  for (long int i = 0; i < length - 9; i += 10) {
    acc = acc OP data[i] OP data[i+1] OP data[i+2] OP data[i+3] OP data[i+4] OP
          data[i+5] OP data[i+6] OP data[i+7] OP data[i+8] OP data[i+9];
  }
  *dest = acc;
}

/* Combine6: Multiple accumulators */
void combine6_unrolled(array_ptr v, data_t *dest) {
  long int length = get_array_length(v);
  data_t *data = get_array_start(v);
  data_t acc0 = IDENT, acc1 = IDENT;
  for (long int i = 0; i < length - 9; i += 10) {
    acc0 = acc0 OP data[i];
    acc1 = acc1 OP data[i+1];
  }
  *dest = acc0 OP acc1;
}

/* Combine7: Reassociative transformation */
void combine7_unrolled(array_ptr v, data_t *dest) {
  combine5_unrolled(v, dest);
}
