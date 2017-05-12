#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "../src/thread.h"

void * recursivity(void* arg) {
  int *i;
  *i = 4 + (int)(recursivity((void*)i));
  return *i;
}

int main() {
  thread_t th;
  int err = thread_create(&th, recursivity, NULL);
  assert(!err);


  return 0;
}
