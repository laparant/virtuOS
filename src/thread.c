#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/queue.h> // Using the singly linked tail queue STAILQ for runq
#include <valgrind/valgrind.h>
#include "thread.h"

#ifndef USE_PTHREAD
#include "retval.h"
#include "errors.h"

#define CHECK(val, errval, msg) if ((val) == (errval)) {perror(msg); exit(EXIT_FAILURE);}

typedef struct thread_base thread_base;

typedef struct thread
{
    thread_base *addr;
    STAILQ_ENTRY(thread) runq_entries; // This entry will be used for the runq
    STAILQ_ENTRY(thread) all_entries; // This entry will be used for the all_threads queue
} thread;

/* The thread that is currently being executed */
static thread * g_current_thread;

/* The list of all the threads that were created */
static STAILQ_HEAD(thread_list_all, thread) g_all_threads;

/* The run queue */
static STAILQ_HEAD(thread_list_run, thread) g_runq;

typedef struct thread_base
{
    thread *sleepq;
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
    if(func != NULL)
    {
        void *res = func(funcarg);
        thread_exit(res);
    }
}


/* Clean the process exiting */
void free_resources(thread *th)
{
    /* Free the resources */
    VALGRIND_STACK_DEREGISTER(th->addr->valgrind_stackid);
    free(th->addr->ctx->uc_stack.ss_sp);
    free(th->addr->ctx);
    free_retval((thread_t)th->addr->rv);
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
    th->addr->sleepq = NULL;

    /* Insert the current thread in the run queue */
    STAILQ_INSERT_TAIL(&g_runq, th, runq_entries);
    /* Put the thread in g_all_threads so we can free it later */
    STAILQ_INSERT_TAIL(&g_all_threads, th, all_entries);
    *newthread = (thread_t) th;

    return EXIT_SUCCESS;
}

int thread_yield(void)
{
    STAILQ_INSERT_TAIL(&g_runq, g_current_thread, runq_entries);
    thread *new_current = STAILQ_FIRST(&g_runq);

    STAILQ_REMOVE_HEAD(&g_runq, runq_entries);

    thread *tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")

    return EXIT_SUCCESS;
}

int thread_join(thread_t thread, void **retval)
{
    /* Joining the threads waiting for the end of the thread */
    struct thread *th = (struct thread *) thread;

    /* Search if the thread does exist => error : ESRCH */
    struct thread *th_i;
    STAILQ_FOREACH(th_i, &g_all_threads, all_entries)
    {
        if(th_i == th)
            break;
        if(STAILQ_NEXT(th_i,all_entries) == NULL)
            return ESRCH;
    }

    /* Detecting the deadlock (the thread is waiting for me) => error : EDEADLK */
    struct thread *me = thread_self();
    if (th == me->addr->sleepq)
        return EDEADLK;

    /* Detecting if the thread is already joined by another thread */
    if (th->addr->sleepq != NULL)
        return EINVAL;

    /* If the thread has already finished: an error might occur but we won't check it */
    if (th->addr->exited)
    {
        if (retval) *retval = get_value(th->addr->rv);
        return EXIT_SUCCESS;
    }

    /* If the thread is alive */
    th->addr->sleepq = me;

    /* Sleeping while the thread hasn't finished */
    struct thread *new_current = STAILQ_FIRST(&g_runq);

    STAILQ_REMOVE_HEAD(&g_runq, runq_entries); //SEGFAULT ICI

    struct thread *tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")

    /* Collecting the value of retval */
    if (retval) *retval = get_value(th->addr->rv);

    /* If not thread main free the resources */
    if(th != STAILQ_FIRST(&g_all_threads)) {
      // Removing the thread from the threads joignable
      STAILQ_REMOVE(&g_all_threads, th, thread, all_entries);
      free_resources(th);
    }
    return EXIT_SUCCESS;
}

__attribute__ ((__noreturn__)) void thread_exit(void *retval)
{
    thread *me = (thread *) thread_self();
    me->addr->exited = 1;

    /* Set the retval */
    if (retval) me->addr->rv->value = retval;

    /* Waking up the thread waiting for me */
    if(me->addr->sleepq != NULL)
        STAILQ_INSERT_TAIL(&g_runq, me->addr->sleepq, runq_entries);

    /* Yielding to next thread if others threads are running*/
    if (!STAILQ_EMPTY(&g_runq))
    {
        thread *new_current = STAILQ_FIRST(&g_runq);
        g_current_thread = new_current;
        STAILQ_REMOVE_HEAD(&g_runq, runq_entries);
    }
    /* Yielding to the thread_main,which is exiting */
    else
    {
        g_current_thread = STAILQ_FIRST(&g_all_threads);
    }

    /* Leaving the runqueue */
    if (me != STAILQ_FIRST(&g_all_threads))
    {
        CHECK(setcontext(g_current_thread->addr->ctx), -1, "thread_exit: setcontext")
    }
    /* Main */
    else
    {
        CHECK(swapcontext(me->addr->ctx, g_current_thread->addr->ctx), -1, "thread_exit_main: swapcontext")
    }
    exit(EXIT_SUCCESS);
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
    th->addr->exited = 0;
    th->addr->ctx->uc_link = NULL;
    makecontext(th->addr->ctx, (void (*)(void)) force_exit, 2, NULL, NULL);

    /* Get the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    th->addr->sleepq=NULL;

    /* Initializing the current thread */
    g_current_thread = th;

    /* Initialization of the queues */
    STAILQ_INIT(&g_all_threads);
    STAILQ_INIT(&g_runq);

    /* Insert in the global queue */
    STAILQ_INSERT_HEAD(&g_all_threads, th, all_entries);
}

__attribute__ ((destructor)) void thread_exit_main (void)
{

    thread *th;
    thread *me = (thread *) thread_self();
    if (me->addr->exited == 0)
    {
        me->addr->exited = 1;

        /* Set the retval */
        //if (retval) me->addr->rv->value = retval;
        //inc_counter(me->addr->rv);
        // could not be done because no retval : find a solution

        /* Waking up the thread waiting for me */
        if(me->addr->sleepq != NULL)
            STAILQ_INSERT_TAIL(&g_runq, me->addr->sleepq, runq_entries);

        /* If others threads are running */
        while (!STAILQ_EMPTY(&g_runq))
        {
            thread *new_current = STAILQ_FIRST(&g_runq);
            g_current_thread = new_current;
            STAILQ_REMOVE_HEAD(&g_runq, runq_entries);

            /* Leaving the runqueue */
            CHECK(swapcontext(me->addr->ctx, new_current->addr->ctx), -1, "thread_exit_main: swapcontext")
        }
    }

    /* Clean everything */
    thread *th2;
    thread *main_thread = g_current_thread;
    //STAILQ_REMOVE_HEAD(&g_all_threads, all_entries);
    th = STAILQ_FIRST(&g_all_threads);
    while (th != NULL)
    {
        th2 = STAILQ_NEXT(th, all_entries);
        if (th != main_thread)
        {
            VALGRIND_STACK_DEREGISTER(th->addr->valgrind_stackid);
            free(th->addr->ctx->uc_stack.ss_sp);
            free(th->addr->ctx);
            free_retval(th->addr->rv);
            free(th->addr);
            free(th);
        }
        th = th2;
    }

    VALGRIND_STACK_DEREGISTER(main_thread->addr->valgrind_stackid);
    free(main_thread->addr->ctx->uc_stack.ss_sp);
    free(main_thread->addr->ctx);
    free_retval(main_thread->addr->rv);
    free(main_thread->addr);
    free(main_thread);

    STAILQ_INIT(&g_all_threads);
}

#endif
