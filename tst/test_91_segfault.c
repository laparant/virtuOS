#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include "../src/thread.h"

void * recursivity(void* arg) {
    return recursivity(NULL);
}

void * func(void * arg) {
    printf("Thread 2 is running\n");
    return (void *) 0xfeedbeef;
}

int main() {
  printf("Je suis le main %p\n",thread_self());
  thread_t th1;
  thread_t th2;
  int err = thread_create(&th1, recursivity, NULL);
  assert(!err);
  err = thread_create(&th2, func, NULL);
  assert(!err);
  thread_yield();

  void *res = NULL;
  err = thread_join(th1, &res);
  assert(!err);
  assert(res == (void *) 0xdead);
  printf("Thread segfaulting exited correctly\n");

  err = thread_join(th2, &res);
  assert(!err);
  assert(res == (void *) 0xfeedbeef);
  printf("Other thread could process normally\n");

  return 0;
}
