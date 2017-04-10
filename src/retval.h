#ifndef RETVAL_H
#define RETVAL_H

struct retval
{
    unsigned int counter;
    void *value;
};

struct retval *init_retval();

void free_retval(struct retval *rv);

void *get_value(struct retval *rv);

int inc_counter(struct retval *rv);

int dec_counter(struct retval *rv);

#endif // RETVAL_H
