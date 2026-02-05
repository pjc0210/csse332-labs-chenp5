#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int max;
volatile int counter = 0; // shared global variable

void *
mythread(void *arg)
{
  char *letter = arg;
  int i; // stack (private per thread)
  printf("%s: begin [addr of i: %p]\n", letter, &i);
  printf("%s: begin [addr of count: %p]\n", letter, &counter);
  for(i = 0; i < max; i++) {
    counter = counter + 1; // shared: only one
  }
  printf("%s: done\n", letter);
  return NULL;
}

int
main(int argc, char *argv[])
{
  #define MAX_THREADS 5
  char *ids[MAX_THREADS] = {"A", "B", "C", "D", "E"};
  if(argc != 2) {
    fprintf(stderr, "usage: %s <loopcount>\n", argv[0]);
    exit(1);
  }
  max = atoi(argv[1]);

  pthread_t pthreads[MAX_THREADS];
  printf("main: begin [counter = %d]\n", counter);
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_create(&pthreads[i], NULL, mythread, ids[i]);
  }
  // join waits for the threads to finish
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_join(pthreads[i], NULL);
  }
  printf("main: done\n [counter: %d]\n [should: %d]\n", counter, max * MAX_THREADS);
  return 0;
}
