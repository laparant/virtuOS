#include "../src/retval.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ITER 20

int tst__init_retval() {
  struct retval *rv = NULL;
  rv = init_retval();
  if((rv != NULL) && (rv->counter == 0)) {
    free(rv); //free value if value is m'allocated
    return EXIT_SUCCESS;
  }
  free(rv);
  return EXIT_FAILURE;
}

//TODO find a test for free

int tst__inc_counter() {
  struct retval * rv = init_retval();
  int i = 1;
  for(; i <= ITER ; ++i) {
    inc_counter(rv);
    if(rv->counter != i) {
      free_retval(rv);
      return EXIT_FAILURE;
    }
  }
  free_retval(rv);
  return EXIT_SUCCESS;
}

int tst__dec_counter() {
  struct retval * rv = init_retval();
  int i = ITER;
  rv->counter = ITER + 1;
  for(; i >= 0 ; --i) {
    dec_counter(rv);
    if(rv->counter != i) {
      free_retval(rv);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int main() {
  assert((tst__init_retval() == EXIT_SUCCESS) && "test init_retval failed\n");
  assert((tst__inc_counter() == EXIT_SUCCESS) && "test inc_counter failed\n");
  assert((tst__dec_counter() == EXIT_SUCCESS) && "test dec_counter failed\n");

  return 0;
}
