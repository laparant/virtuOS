#include "../src/retval.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int tst__init_retval() {
  struct retval *rv = init_retval();
  if(rv != NULL) {
    free(rv); //free value if value is m'allocated
    return EXIT_SUCCESS;
  }
  free(rv);
  return EXIT_FAILURE;
}

//TODO find a test for free

int main() {
  assert((tst__init_retval() == EXIT_SUCCESS) && "test init_retval failed\n");

  return 0;
}
