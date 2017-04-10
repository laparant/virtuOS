#include <stdlib.h>
#include "retval.h"

struct retval *init_retval() {
  struct retval * rv = malloc(sizeof( struct retval));
  rv->counter = 0;
  //rv->value = malloc(sizeof(int *));
  return rv;
}

void free_retval(struct retval *rv) {
  //free(rv->value);
  free(rv);
}

void *get_value(struct retval *rv) {
  return rv->value;
}

int inc_counter(struct retval *rv) {
  if(rv != NULL) {
    rv->counter++;
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

int dec_counter(struct retval *rv) {
  if((rv != NULL) && (rv->counter != 0)) {
    rv->counter--;
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
