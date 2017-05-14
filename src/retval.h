#ifndef RETVAL_H
#define RETVAL_H

struct retval
{
    void *value;
};

struct retval *init_retval();

/**
 * @fn      free_retval
 * @brief   frees the memory m'allocated for struct retval
 * @param   pointer to retval
 */
void free_retval(struct retval *rv);

void *get_value(struct retval *rv);

#endif // RETVAL_H
