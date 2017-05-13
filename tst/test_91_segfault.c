#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "../src/thread.h"

#define SEGFAULT -1 // comme dans define.h

void * recursivity(void* arg) {
  //printf("%p\n",&arg);
  return recursivity(NULL);
}

int main() {
  thread_t th;
  int err = thread_create(&th, recursivity, NULL);
  assert(!err);
  thread_yield();

  thread_t th2;
  err = thread_create(&th2, recursivity, NULL);
  assert(!err);

  void *res = NULL;
  err = thread_join(th, res);
  assert(err == SEGFAULT);

  return 0;
}
