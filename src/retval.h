#ifndef RETVAL_H
#define RETVAL_H
/*!
 * \file retval.h
 * \brief contains the definitions and utilitaries of the retval structure
 */

/*!
 * \struct retval
 * \brief this structure aims at stocking the return value of a thread
 */
struct retval
{
    void *value; /*! value of the return value */
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
