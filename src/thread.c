#include <stdlib.h>
#include <ucontext.h>

#include "thread.h"
#include "retval.h"
#include <sys/queue.h> // Using the singly linked tail queue STAILQ for runq

typedef struct thread thread;

/* The thread that is currently being executed */
static thread *g_current_thread;

/* The list of all the threads that were created */
STAILQ_HEAD(thread_list, thread);
struct thread_list g_all_threads;

/* The run queue */
STAILQ_HEAD(run_queue, thread *);
static struct run_queue g_runq;

/* Declaration of the sleep_queue structure */
STAILQ_HEAD(sleep_queue, thread *);

typedef struct thread
{
    struct sleep_queue *sleepq;
    struct retval *rv;
    ucontext_t *ctx;

    STAILQ_ENTRY(thread) entries;
} thread;




thread_t thread_self(void)
{
    return g_current_thread;
}

void force_exit(void *(*func)(void *), void *funcarg)
{
    void *res = func(funcarg);
    thread_exit(res);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    /* First thread will initialize all_threads and runqueue */
    if (STAILQ_EMPTY(&g_all_threads))
    {
        STAILQ_INIT(&g_all_threads);
    }
    if (STAILQ_EMPTY(&g_runq))
    {
        STAILQ_INIT(&g_runq);
    }

    /* Initialization of the context */
    thread *th = malloc(sizeof(thread));
    th->ctx = malloc(sizeof(ucontext_t));
    getcontext(th->ctx);

    th->ctx->uc_stack.ss_size = 64 * 1024;
    th->ctx->uc_stack.ss_sp = malloc(th->ctx->uc_stack.ss_size);
    th->ctx->uc_link = NULL;
    makecontext(th->ctx, force_exit, 2, func, funcarg);

    /* Get the return value */
    th->rv = init_retval();

    /* Initialize the thread's sleep queue */
    STAILQ_INIT(th->sleepq);

    /* Insert the current thread in the run queue the list of all the threads */
    STAILQ_INSERT_TAIL(&g_runq, th, entries);
    STAILQ_INSERT_TAIL(&g_all_threads, th, entries);
    newthread = th;
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
