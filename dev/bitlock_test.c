#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "bit_macros.h"

typedef struct {
  pthread_t th;
  size_t id, result;
} TestThread;

// Do a million loops
#define NUM_LOOPS 10000
char locks[(NUM_LOOPS+7)/8] = {0};
char data[NUM_LOOPS] = {0};

void* worker(void *ptr)
{
  TestThread *wrkr = (TestThread*)ptr;
  size_t i;
  for(i = 0; i < NUM_LOOPS; i++) {
    bitlock_yield_acquire(locks, i);
    wrkr->result += i + *(volatile char *)&data[i];
    data[i] = wrkr->id;
    usleep(5);
    bitlock_release(locks, i);
    usleep(5);
  }

  return NULL;
}

// sum of 0 up to num (inclusive)
#define cumm_sum(num) ((num)*(((num)+1)/2)+(((num)&1) ? 0 : (num)/2))

int main()
{
  printf("\nTesting Multithreaded Bitlocks\n\n");

  int rc;
  size_t i, num_threads = 30;
  TestThread workers[num_threads];
  memset(data, 0, sizeof(data));

  // for(i = 0; i < num_threads; i++)
  //   printf("cummulative sum %zu: %zu\n", i, cumm_sum(i));

  // Create threads
  for(i = 0; i < num_threads; i++) {
    workers[i] = (TestThread){.id = i+1, .result = 0};
    rc = pthread_create(&workers[i].th, NULL, worker, &workers[i]);
    if(!rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); }
  }

  // Wait for threads to finish
  for(i = 0; i < num_threads; i++) {
    rc = pthread_join(workers[i].th, NULL);
    if(!rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); }
  }

  size_t sum = 0, expsum = 0;
  for(i = 0; i < NUM_LOOPS; i++) sum += data[i];
  for(i = 0; i < num_threads; i++) sum += workers[i].result;

  // for(i = 0; i < num_threads; i++) printf("got: %zu %zu\n", i, workers[i].result);

  // 0 1 2 3 4 5
  // 0 1 3 6 10
  expsum = cumm_sum(NUM_LOOPS-1)*num_threads + cumm_sum(num_threads)*NUM_LOOPS;
  // expsum = cumm_sum(NUM_LOOPS-1) * num_threads;

  bool pass = (sum == expsum);

  for(i = 0; i < sizeof(locks) && locks[i] == 0; i++) {}
  if(i < sizeof(locks)) {
    printf("locks not zeroed!\n");
    pass = false;
  }

  printf("sum: %zu exp: %zu\n", sum, expsum);
  printf("%s.\n\n", pass ? "Pass" : "Fail");
  return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
