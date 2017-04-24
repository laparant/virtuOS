#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/queue.h> // Using the singly linked tail queue STAILQ for runq

#include "thread.h"
#include "retval.h"

#define SUCCESS 0
#define FAILURE -1

#define CHECK(val, errval, msg) if ((val) == (errval)) {perror(msg); exit(EXIT_FAILURE);}

typedef struct thread_base thread_base;

typedef struct thread
{
    thread_base *addr;
    STAILQ_ENTRY(thread) entries;
} thread;

/* The thread that is currently being executed */
static thread * g_current_thread;

/* The list of all the threads that were created */
static STAILQ_HEAD(thread_list_all, thread) g_all_threads;

/* The run queue */
static STAILQ_HEAD(thread_list_run, thread) g_runq;

typedef struct thread_base
{
    STAILQ_HEAD(thread_list_sleep, thread) sleepq;
    struct retval *rv;
    ucontext_t *ctx;
} thread_base;

thread_t thread_self(void)
{
    return g_current_thread->addr;
}

void force_exit(void *(*func)(void *), void *funcarg)
{
    void *res = func(funcarg);
    thread_exit(res);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    /* Initialization of the context */
    thread *th = malloc(sizeof(thread));
    CHECK(th, NULL, "thread_create: thread pointer malloc")
    th->addr = malloc(sizeof(thread_base));
    CHECK(th->addr, NULL, "thread_create: thread_base malloc")
    th->addr->ctx = malloc(sizeof(ucontext_t));
    CHECK(th->addr->ctx, NULL, "thread_create: context malloc")
    getcontext(th->addr->ctx);

    th->addr->ctx->uc_stack.ss_size = 64 * 1024;
    th->addr->ctx->uc_stack.ss_sp = malloc(th->addr->ctx->uc_stack.ss_size);
    th->addr->ctx->uc_link = NULL;
    makecontext(th->addr->ctx, (void (*)(void)) force_exit, 2, func, funcarg);

    /* Get the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    STAILQ_INIT(&(th->addr->sleepq));

    /* Insert the current thread in the run queue the list of all the threads */
    STAILQ_INSERT_TAIL(&g_runq, th, entries);
    STAILQ_INSERT_TAIL(&g_all_threads, th, entries);
    *newthread = th;

    return SUCCESS;
}

int thread_yield(void)
{
    STAILQ_INSERT_TAIL(&g_runq, g_current_thread, entries);
    thread *new_current = STAILQ_FIRST(&g_runq);

    STAILQ_REMOVE_HEAD(&g_runq, entries);

    CHECK(swapcontext(g_current_thread->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")
    g_current_thread = new_current;

    return EXIT_SUCCESS;
}

int thread_join(thread_t thread, void **retval)
{

}

void thread_exit(void *retval)
{
}

/*#############################################################################################*/

__attribute__ ((constructor)) void first_thread (void)
{
    /* Initialization of the current thread */
    /* Initialization of the context */
    thread *th = malloc(sizeof(thread));
    CHECK(th, NULL, "thread_create: thread pointer malloc")
    th->addr = malloc(sizeof(thread_base));
    CHECK(th->addr, NULL, "thread_create: thread_base malloc")
    th->addr->ctx = malloc(sizeof(ucontext_t));
    CHECK(th->addr->ctx, NULL, "thread_create: context malloc")
    getcontext(th->addr->ctx);

    th->addr->ctx->uc_stack.ss_size = 64 * 1024;
    th->addr->ctx->uc_stack.ss_sp = malloc(th->addr->ctx->uc_stack.ss_size);
    th->addr->ctx->uc_link = NULL;
    makecontext(th->addr->ctx, (void (*)(void)) force_exit, 2, NULL, NULL);

    /* Get the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    STAILQ_INIT(&(th->addr->sleepq));

    /* Initializing the current thread */
    g_current_thread = th;

    /* Initialization of the queues */
    STAILQ_INIT(&g_all_threads);
    STAILQ_INIT(&g_runq);
}

__attribute__ ((destructor)) void cleaner (void)
{
    thread * th;

    /* Free g_all_threads */
    STAILQ_FOREACH(th, &g_all_threads, entries)
    {
        free(th->addr->ctx->uc_stack.ss_sp);
        free(th->addr->ctx);
        free_retval(th->addr->rv);
        free(th->addr);
        free(th);
    }

    /* Free g_runq */
    STAILQ_FOREACH(th, &g_runq, entries)
    {
        free(th->addr->ctx->uc_stack.ss_sp);
        free(th->addr->ctx);
        free_retval(th->addr->rv);
        free(th->addr);
        free(th);
    }

    /* Free the current thread */
    th=g_current_thread;
    free(th->addr->ctx->uc_stack.ss_sp);
    free(th->addr->ctx);
    free_retval(th->addr->rv);
    free(th->addr);
    free(th);
}
