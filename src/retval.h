#ifndef RETVAL_H
#define RETVAL_H

struct retval
{
    unsigned int counter;
    void *value;
};

struct retval *init_retval();

/**
 * @fn      free_retval
 * @brief   frees the memory m'allocated for struct retval
 * @param   pointer to retval
 * TODO     set the pointer to null after the call of free_retval
 */
void free_retval(struct retval *rv);

void *get_value(struct retval *rv);

int inc_counter(struct retval *rv);

int dec_counter(struct retval *rv);

#endif // RETVAL_H
