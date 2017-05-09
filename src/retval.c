#include <stdlib.h>
#include <stdio.h>
#include "retval.h"

struct retval *init_retval()
{
    struct retval *rv = malloc(sizeof( struct retval));
    //rv->counter = 0;
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

/*
int inc_counter(struct retval *rv)
{
    if (rv != NULL)
    {
        rv->counter++;
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int dec_counter(struct retval *rv) {
    if ((rv != NULL) && (rv->counter != 0))
    {
        rv->counter--;
        if (rv->counter == 0) free_retval(rv);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
*/
