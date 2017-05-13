#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include "../src/thread.h"

void * recursivity(void* arg) {
    printf("%p\n", &arg);
    return recursivity(NULL);
}

int main() {
  thread_t th;
  int err = thread_create(&th, recursivity, NULL);
  assert(!err);
  thread_yield();

  void *res = NULL;
  err = thread_join(th, res);
  assert(strcmp(res,"segfault"));

  return 0;
}
