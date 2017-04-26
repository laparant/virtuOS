#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/queue.h> // Using the singly linked tail queue STAILQ for runq
#include <valgrind/valgrind.h>

#include "thread.h"
#include "retval.h"

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
    int valgrind_stackid;
    int exited;
} thread_base;

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
    int valgrind_stackid = VALGRIND_STACK_REGISTER(th->addr->ctx->uc_stack.ss_sp,
                                                   th->addr->ctx->uc_stack.ss_sp + th->addr->ctx->uc_stack.ss_size);
    th->addr->valgrind_stackid=valgrind_stackid;
    th->addr->exited = 0;
    th->addr->ctx->uc_link = NULL;
    makecontext(th->addr->ctx, (void (*)(void)) force_exit, 2, func, funcarg);

    /* Get the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    STAILQ_INIT(&(th->addr->sleepq));

    /* Insert the current thread in the run queue the list of all the threads */
    STAILQ_INSERT_TAIL(&g_runq, th, entries);
    STAILQ_INSERT_TAIL(&g_all_threads, th, entries);
    *newthread = (thread_t) th;

    return EXIT_SUCCESS;
}

int thread_yield(void)
{
    STAILQ_INSERT_TAIL(&g_runq, g_current_thread, entries);
    thread *new_current = STAILQ_FIRST(&g_runq);

    STAILQ_REMOVE_HEAD(&g_runq, entries);

    thread * tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")

    return EXIT_SUCCESS;
}

int thread_join(thread_t thread, void **retval)
{
    /* Joining the threads waiting for the end of the thread */
    struct thread *th = (struct thread *) thread;

    /* If the thread has already finished : just collect the retval and continue */
    if(th->addr->exited)
    {
        if(retval) *retval = get_value(th->addr->rv);
    }

    return EXIT_SUCCESS;

    /* If the thread is alive */
    inc_counter(th->addr->rv);
    STAILQ_INSERT_TAIL(&(th->addr->sleepq),(struct thread *) thread_self(),entries);

    /* Sleeping while the thread hasn't finished */
    struct thread *new_current = STAILQ_FIRST(&g_runq);

    STAILQ_REMOVE_HEAD(&g_runq, entries);

    struct thread * tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")

    /* Collecting the value of retval */
    if(retval) *retval = get_value(th->addr->rv);
    dec_counter(th->addr->rv);

    return EXIT_SUCCESS;
}

void thread_exit(void *retval)
{
    thread * th;
    thread * me = (thread *) thread_self();

    /* Set the retval */
    if(retval) me->addr->rv->value = retval;

    /* Waking up all the threads waiting for me */
    STAILQ_FOREACH(th, &(me->addr->sleepq), entries)
    {
        STAILQ_INSERT_TAIL(&g_runq, th, entries);
    }

    /* Free the ressources */
    VALGRIND_STACK_DEREGISTER(me->addr->valgrind_stackid);
    free(me->addr->ctx->uc_stack.ss_sp);
    free(me->addr->ctx);
    //free(me->addr);
    //free(me);

    /* Yielding and leaving the runqueue */
    thread *new_current = STAILQ_FIRST(&g_runq);
    g_current_thread = new_current;
    STAILQ_REMOVE_HEAD(&g_runq, entries);

    CHECK(setcontext(g_current_thread->addr->ctx), -1, "thread_exit: setcontext")
}

/*#############################################################################################*/

__attribute__ ((constructor)) void thread_create_main (void)
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
    int valgrind_stackid = VALGRIND_STACK_REGISTER(th->addr->ctx->uc_stack.ss_sp,
                                                   th->addr->ctx->uc_stack.ss_sp + th->addr->ctx->uc_stack.ss_size);
    th->addr->valgrind_stackid=valgrind_stackid;
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

__attribute__ ((destructor)) void thread_exit_main (void)
{
    thread * th;

    /* Free the current thread which is the main */
    th=g_current_thread;
    VALGRIND_STACK_DEREGISTER(th->addr->valgrind_stackid);
    free(th->addr->ctx->uc_stack.ss_sp);
    free(th->addr->ctx);
    //free_retval(th->addr->rv);
    //free(th->addr);
    //free(th);

    /* Yield to let other threads continue */

    /* Clean everything */
}
