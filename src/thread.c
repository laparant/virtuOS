#include <stdlib.h>
#include <ucontext.h>

#include "thread.h"
#include "queue.h"

typedef struct thread
{
    thread *sleepq;
    void *data; /* We need to choose between data and counter */
    struct ucontext *ctx;
} thread;

static thread *g_current_thread;
static thread *g_all_threads;

thread_t thread_self(void)
{
    return current_thread;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    all_threads = realloc(all_threads, sizeof(all_threads) + sizeof(thread));

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
