#include "thread.h"
#include "queue.h"

typedef struct thread
{
    struct ucontext* ctx;

} thread;

thread_t thread_self(void)
{

}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{

}

int thread_yield(void)
{

}

int thread_join(thread_t thread, void **retval)
{

}

void thread_exit(void *retval)
{

}
