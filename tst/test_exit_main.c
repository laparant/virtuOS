#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../src/thread.h"

int main() {
  int i = 4;
  printf("test exit in main...\n");
  pthread_exit((void *)&i);

  return EXIT_SUCCESS;
}
