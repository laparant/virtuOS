#include <stdlib.h>
#include <stdio.h>
#include "retval.h"

struct retval *init_retval()
{
    struct retval *rv = malloc(sizeof( struct retval));
    rv->value = NULL;
    return rv;
}

void free_retval(struct retval *rv)
{
    free(rv);
    rv = NULL;
}

void *get_value(struct retval *rv)
{
    return rv->value;
}
