/*
 dev/bitlock_test.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Aug 2014
*/

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
char *locks, *data;
pthread_mutex_t *mutexes;

void* worker_bitlock(void *ptr)
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

void* worker_bitlock_spin(void *ptr)
{
  TestThread *wrkr = (TestThread*)ptr;
  size_t i;
  for(i = 0; i < NUM_LOOPS; i++) {
    bitlock_acquire(locks, i);
    wrkr->result += i + *(volatile char *)&data[i];
    data[i] = wrkr->id;
    usleep(5);
    bitlock_release(locks, i);
    usleep(5);
  }

  return NULL;
}

void* worker_mutex(void *ptr)
{
  TestThread *wrkr = (TestThread*)ptr;
  size_t i;
  for(i = 0; i < NUM_LOOPS; i++) {
    pthread_mutex_lock(&mutexes[0]);
    wrkr->result += i + *(volatile char *)&data[i];
    data[i] = wrkr->id;
    usleep(5);
    pthread_mutex_unlock(&mutexes[0]);
    usleep(5);
  }

  return NULL;
}

void* worker_mutexes(void *ptr)
{
  TestThread *wrkr = (TestThread*)ptr;
  size_t i;
  for(i = 0; i < NUM_LOOPS; i++) {
    pthread_mutex_lock(&mutexes[i]);
    wrkr->result += i + *(volatile char *)&data[i];
    data[i] = wrkr->id;
    usleep(5);
    pthread_mutex_unlock(&mutexes[i]);
    usleep(5);
  }

  return NULL;
}

// sum of 0 up to num (inclusive)
#define cumm_sum(num) ((num)*(((num)+1)/2)+(((num)&1) ? 0 : (num)/2))

int main(int argc, char **argv)
{
  char *method = "Bitlocks";
  void* (*func)(void*) = worker_bitlock;

  if(argc == 2 && strcmp(argv[1],"bits") == 0) {}
  else if(argc == 2 && strcmp(argv[1],"mutex") == 0) {
    method = "Mutex";
    func = worker_mutex;
  }
  else if(argc == 2 && strcmp(argv[1],"mutexes") == 0) {
    method = "Mutexes";
    func = worker_mutexes;
  }
  else if(argc == 2 && strcmp(argv[1],"spin") == 0) {
    method = "Bitlocks-Spin";
    func = worker_bitlock_spin;
  }
  else if(argc != 1) {
    fprintf(stderr, "usage: ./bitlock_test <bits|mutex|spin>\n");
    exit(-1);
  }

  printf("\nTesting %s\n\n", method);

  int rc;
  size_t i, num_threads = 30;
  TestThread workers[num_threads];

  locks = (char*)calloc(1, (NUM_LOOPS+7)/8);
  data = (char*)calloc(1, NUM_LOOPS);
  mutexes = (pthread_mutex_t*)calloc(NUM_LOOPS, sizeof(pthread_mutex_t));

  for(i = 0; i < NUM_LOOPS; i++)
    pthread_mutex_init(&mutexes[i], NULL);

  // for(i = 0; i < num_threads; i++)
  //   printf("cummulative sum %zu: %zu\n", i, cumm_sum(i));

  // Create threads
  for(i = 0; i < num_threads; i++) {
    workers[i] = (TestThread){.id = i+1, .result = 0};
    rc = pthread_create(&workers[i].th, NULL, func, &workers[i]);
    if(rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); exit(-1); }
  }

  // Wait for threads to finish
  for(i = 0; i < num_threads; i++) {
    rc = pthread_join(workers[i].th, NULL);
    if(rc) { fprintf(stderr, "pthread error: %s\n", strerror(rc)); exit(-1); }
  }

  for(i = 0; i < NUM_LOOPS; i++)
    pthread_mutex_destroy(&mutexes[i]);

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

  free(mutexes);
  free(data);
  free(locks);

  return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
