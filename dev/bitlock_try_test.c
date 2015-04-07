/*
 dev/bitlock_try_test.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Sep 2014
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "bit_macros.h"

// Use the bitlock_try_acquire() macro to visit each number exactly once

typedef struct {
  pthread_t th;
  size_t id, result;
  uint8_t *locks;
} TestThread;

#define NWORKERS 10
#define LIMIT 10000

void* worker(void *ptr)
{
  TestThread *wrkr = (TestThread*)ptr;
  size_t i, locked;
  for(i = 0; i < LIMIT; i++) {
    bitlock_try_acquire(wrkr->locks, i, &locked);
    wrkr->result += locked*i;
    if((i&0xff) == 0xff) { usleep(50); sched_yield(); }
  }
  return NULL;
}

// sum of 0 up to num (inclusive)
#define cumm_sum(num) ((num)*(((num)+1)/2)+(((num)&1) ? 0 : (num)/2))

int main(int argc, char **argv)
{
  (void)argc; (void)argv;

  size_t i; int rc;
  uint8_t *locks = (uint8_t*)calloc((LIMIT+7)/8, sizeof(uint8_t));
  TestThread *workers = (TestThread*)calloc(NWORKERS, sizeof(TestThread));

  // Create threads
  for(i = 0; i < NWORKERS; i++) {
    workers[i] = (TestThread){.id = i, .result = 0, .locks = locks};
    rc = pthread_create(&workers[i].th, NULL, worker, &workers[i]);
    if(rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); exit(-1); }
  }

  // Wait for threads to finish
  for(i = 0; i < NWORKERS; i++) {
    rc = pthread_join(workers[i].th, NULL);
    if(rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); exit(-1); }
  }

  size_t sum = 0, expsum = cumm_sum(LIMIT-1);
  for(i = 0; i < NWORKERS; i++) sum += workers[i].result;

  bool pass = (sum == expsum);

  for(i = 0; i < sizeof(locks) && locks[i] == 0xff; i++) {}
  if(i < sizeof(locks)) {
    printf("locks not all ones!\n");
    pass = false;
  }

  printf("sum: %zu exp: %zu\n", sum, expsum);
  printf("%s.\n\n", pass ? "Pass" : "Fail");

  free(workers);
  free(locks);

  return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
