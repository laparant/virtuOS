#include "thread.h"

#ifndef USE_PTHREAD
#include "retval.h"
#include "define.h"

/*
 * ##############################################################################################
 * ######                             Cleaning processes                                   ######
 * ##############################################################################################
 */
/*!
 * \brief frees context of a thread.
 * \fn void free_context(thread *th)
 * \param th thread
 */
void free_context(thread *th)
{
    STAILQ_REMOVE(&g_to_free, th, thread, to_free_entries);
    /* Free the resources */
    VALGRIND_STACK_DEREGISTER(th->addr->valgrind_stackid);
    free(th->addr->ctx->uc_stack.ss_sp);
    free(th->addr->ctx);
    th->addr->status = ALREADY_FREE;
}

/*!
 * \brief frees all the remaining resources of a thread in thread_join
 * \fn void free_join(thread *th)
 * \param th thread
 */
void free_join(thread *th)
{
    if (th->addr->status == TO_FREE)
    {
        free_context(th);
    }
    free_retval(th->addr->rv);
    free(th->addr);
    free(th);
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                             Priority management                                  ######
 * ##############################################################################################
 */

/*!
 * \brief
 * \fn int thread_set_priority(thread_t thread, short priority)
 * \param thread
 * \param priority
 * \return EXIT_FAILURE on failure, EXIT_SUCCESS otherwise
 */
int thread_set_priority(thread_t thread, short priority)
{
    /* If priority is not valid, exit */
    if (priority < 1 || priority > 10)
    {
        return EXIT_FAILURE;
    }
    else
    {
        struct thread *th = (struct thread *) thread;
        th->addr->priority.value = priority;
        return EXIT_SUCCESS;
    }
}

/*!
 * \brief
 * \fn short thread_get_priority(thread_t thread)
 * \param thread
 * \return
 */
short thread_get_priority(thread_t thread)
{
    struct thread *th = (struct thread *) thread;
    return th->addr->priority.value;
}

/*!
 * \brief
 * \fn __useconds_t get_priority_timeslice(thread *th)
 * \param th
 * \return
 */
__useconds_t get_priority_timeslice(thread *th)
{
    if (th->addr->priority.value%2 == 1) // priority is an odd value
    {
        return TIMESLICE * (th->addr->priority.value/2 + 1);
    }
    else // priority is an even value so we need to alternate between higher and lower timeslices
    {
        if (th->addr->priority.alternate)
        {
            th->addr->priority.alternate = 0;
            return TIMESLICE * (th->addr->priority.value/2 + 1);
        }
        else
        {
            th->addr->priority.alternate = 1;
            return TIMESLICE * (th->addr->priority.value/2);
        }
    }
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                             Preemption utilitary                                 ######
 * ##############################################################################################
 */

/*!
 * \brief reset_timer resets the current timer in order to count to the next interruption
 */
void reset_timer()
{
    struct itimerval newtimer;
    CHECK(getitimer(ITIMER_PROF, &newtimer), -1, "reset_timer: getitimer")
    newtimer.it_value.tv_usec = get_priority_timeslice(g_current_thread);
    CHECK(setitimer(ITIMER_PROF, &newtimer, NULL), -1, "reset_timer: getitimer")
}

/*!
 * \brief enable_interruptions
 */
void enable_interruptions()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPROF);
    CHECK(sigprocmask(SIG_UNBLOCK, &set, NULL), -1, "enable_interruptions: sigprocmask")
}

/*!
 * \brief disable_interruptions
 */
void disable_interruptions()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPROF);
    CHECK(sigprocmask(SIG_BLOCK, &set, NULL), -1, "disable_interruptions: sigprocmask")
}

/*!
 * \brief alarm_handler
 * \param signal
 */
void alarm_handler(int signal)
{
    thread_yield();
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                            Auxiliary processes                                   ######
 * ##############################################################################################
 */

/*!
 * \brief force_exit
 * \param funcarg
 */
void force_exit(void *(*func)(void *), void *funcarg)
{
    if (func != NULL)
    {
        void *res = func(funcarg);
        thread_exit(res);
    }
}

/*!
 * \brief finalize_join
 * \param th
 * \param retval
 */
void finalize_join(thread *th, void **retval)
{
    /* Collecting the value of retval */
    if (retval) *retval = get_value(th->addr->rv);

    /* If not thread main free the resources */
    if (th != STAILQ_FIRST(&g_all_threads))
    {
        // Removing the thread from the threads joignable
        STAILQ_REMOVE(&g_all_threads, th, thread, all_entries);
        free_join(th);
    }
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                         Initialisation processes                                 ######
 * ##############################################################################################
 */

/*!
 * \brief init_context
 * \param funcarg
 * \return
 */
thread *init_context(void *(*func)(void *), void *funcarg)
{
    thread *th = malloc(sizeof(thread));
    CHECK(th, NULL, "init_context: thread pointer malloc")
    th->addr = malloc(sizeof(thread_base));
    CHECK(th->addr, NULL, "init_context: thread_base malloc")
    th->addr->ctx = malloc(sizeof(ucontext_t));
    CHECK(th->addr->ctx, NULL, "init_context: context malloc")
    getcontext(th->addr->ctx);

    th->addr->ctx->uc_stack.ss_size = 64 * 1024;
    th->addr->ctx->uc_stack.ss_sp = malloc(th->addr->ctx->uc_stack.ss_size);
    int valgrind_stackid = VALGRIND_STACK_REGISTER(th->addr->ctx->uc_stack.ss_sp,
                                                   th->addr->ctx->uc_stack.ss_sp + th->addr->ctx->uc_stack.ss_size);
    th->addr->valgrind_stackid=valgrind_stackid;
    th->addr->status = RUNNING;
    th->addr->ctx->uc_link = NULL;
    makecontext(th->addr->ctx, (void (*)(void)) force_exit, 2, func, funcarg);

    return th;
}

/*!
 * \brief add_to_scheduler
 * \param th
 */
void add_to_scheduler(thread *th)
{
    /* Insert the current thread in the run queue */
    STAILQ_INSERT_TAIL(&g_runq, th, runq_entries);
    /* Put the thread in g_all_threads so we can free it later */
    STAILQ_INSERT_TAIL(&g_all_threads, th, all_entries);
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                         Errors testing functions                                 ######
 * ##############################################################################################
 */

int is_existing(thread *th)
{
    struct thread *th_i;
    STAILQ_FOREACH(th_i, &g_all_threads, all_entries)
    {
        if (th_i == th)
            break;
        if (STAILQ_NEXT(th_i, all_entries) == NULL)
            return ESRCH;
    }

    return EXIT_SUCCESS;
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                             Thread functions                                     ######
 * ##############################################################################################
 */

thread_t thread_self(void)
{
    return g_current_thread;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    /* Initialization of the context */
    thread *th = init_context(func, funcarg);

    /* Initialization of the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    th->addr->joinq = NULL;

    /* Add the thread to the scheduler */
    add_to_scheduler(th);

    /* Give a default priority of 5 */
    th->addr->priority.value = 5;
    th->addr->priority.alternate = 0;

    /* Giving the return value */
    *newthread = (thread_t) th;

    enable_interruptions();

    return EXIT_SUCCESS;
}

int thread_yield(void)
{
    disable_interruptions();

    /* Free the processes waiting to be freed */
    struct thread *th_i;
    STAILQ_FOREACH(th_i, &g_to_free, to_free_entries)
    {
        free_context(th_i);
    }

    /* Update scheduler */
    STAILQ_INSERT_TAIL(&g_runq, g_current_thread, runq_entries);
    thread *new_current = STAILQ_FIRST(&g_runq);
    STAILQ_REMOVE_HEAD(&g_runq, runq_entries);

    /* Swapping contexes */
    thread *tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_yield: swapcontext")

    /* Reset the timer for the new thread */
    reset_timer();

    enable_interruptions();

    return EXIT_SUCCESS;
}

int thread_join(thread_t thread, void **retval)
{
    struct thread *th = (struct thread *) thread;

    /* Search if the thread does exist => error : ESRCH */
    // is_existing(th);

    /* Detecting the deadlock (the thread is waiting for me) => error : EDEADLK */
    struct thread *me = thread_self();
    if (th == me->addr->joinq)
        return EDEADLK;

    /* Detecting if the thread is already joined by another thread */
    if (th->addr->joinq != NULL)
        return EINVAL;

    /* If the thread has already finished */
    if (th->addr->status != RUNNING)
    {
        finalize_join(th, retval);
        return EXIT_SUCCESS;
    }

    /* If the thread is alive */
    /* Sleeping while the thread hasn't finished */
    th->addr->joinq = me;
    struct thread *new_current = STAILQ_FIRST(&g_runq);
    STAILQ_REMOVE_HEAD(&g_runq, runq_entries);
    struct thread *tmp = g_current_thread;
    g_current_thread = new_current;
    CHECK(swapcontext(tmp->addr->ctx, new_current->addr->ctx), -1, "thread_join: swapcontext")

    /* When woke up (thread is finished) */
    finalize_join(th, retval);
    return EXIT_SUCCESS;
}

__attribute__ ((__noreturn__)) void thread_exit(void *retval)
{
    thread *me = (thread *) thread_self();
    me->addr->status = TO_FREE;

    /* Set the retval */
    if (retval) me->addr->rv->value = retval;

    /* Waking up the thread waiting for me */
    if (me->addr->joinq != NULL)
        STAILQ_INSERT_TAIL(&g_runq, me->addr->joinq, runq_entries);

    /* Yielding to next thread if others threads are running*/
    if (!STAILQ_EMPTY(&g_runq))
    {
        thread *new_current = STAILQ_FIRST(&g_runq);
        g_current_thread = new_current;
        STAILQ_REMOVE_HEAD(&g_runq, runq_entries);
    }
    /* Yielding to the thread_main, which is exiting */
    else
    {
        g_current_thread = STAILQ_FIRST(&g_all_threads);
    }
    reset_timer();

    /* Leaving the runqueue */
    if (me != STAILQ_FIRST(&g_all_threads))
    {
        STAILQ_INSERT_TAIL(&g_to_free, me, to_free_entries);
        CHECK(setcontext(g_current_thread->addr->ctx), -1, "thread_exit: setcontext")
    }
    /* Main */
    else
    {
        CHECK(swapcontext(me->addr->ctx, g_current_thread->addr->ctx), -1, "thread_exit: swapcontext")
    }
    exit(EXIT_SUCCESS);
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                            Specific main functions                               ######
 * ##############################################################################################
 */

__attribute__ ((constructor)) void thread_create_main(void)
{ 
    /* Initialization of the context */
    thread *th = init_context(NULL, NULL);

    /* Initialization of the return value */
    th->addr->rv = init_retval();

    /* Initialize the thread's sleep queue */
    th->addr->joinq = NULL;

    /* Initialization of the queues */
    STAILQ_INIT(&g_all_threads);
    STAILQ_INIT(&g_runq);
    STAILQ_INIT(&g_to_free);

    /* Add the thread to the scheduler */
    g_current_thread = th;
    STAILQ_INSERT_HEAD(&g_all_threads, th, all_entries);

    /* ---- Setting up the alarm for preemption ---- */

    struct sigaction sa;
    struct itimerval timer;

    /* Install alarm_handler as the signal handler for SIGVTALRM */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &alarm_handler;
    CHECK(sigaction(SIGPROF, &sa, NULL), -1, "thread_create_main: sigaction")

    /* Configure the timer to expire after the timeslice */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIMESLICE;
    /* ... and after every timeslice after that */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIMESLICE;
    /* Start a virtual timer. It counts down whenever this process is executing */
    enable_interruptions();
    CHECK(setitimer(ITIMER_PROF, &timer, NULL), -1, "thread_create_main: setitimer")
}

__attribute__ ((destructor)) void thread_exit_main(void)
{
    thread *th;
    thread *me = (thread *) thread_self();
    if (me->addr->status == RUNNING)
    {
        me->addr->status = TO_FREE;

        /* Set the retval */
        //if (retval) me->addr->rv->value = retval;
        // could not be done because no retval : find a solution

        /* Waking up the thread waiting for me */
        if (me->addr->joinq != NULL)
            STAILQ_INSERT_TAIL(&g_runq, me->addr->joinq, runq_entries);

        /* If others threads are running */
        while (!STAILQ_EMPTY(&g_runq))
        {
            thread *new_current = STAILQ_FIRST(&g_runq);
            g_current_thread = new_current;
            STAILQ_REMOVE_HEAD(&g_runq, runq_entries);

            /* Leaving the runqueue */
            reset_timer();
            CHECK(swapcontext(me->addr->ctx, new_current->addr->ctx), -1, "thread_exit_main: swapcontext")
        }
    }

    disable_interruptions();
    /* Clean everything */
    thread *main_thread = g_current_thread;
    thread *th2;

    /* Free the context of the threads exited remaing */
    th = STAILQ_FIRST(&g_to_free);
    while (th != NULL)
    {
        th2 = STAILQ_NEXT(th, to_free_entries);
        if (th != main_thread)
        {
            free_context(th);
        }
        th = th2;
    }
    STAILQ_INIT(&g_to_free);


    /* Free the remaining things */
    th = STAILQ_FIRST(&g_all_threads);
    while (th != NULL)
    {
        th2 = STAILQ_NEXT(th, all_entries);
        if (th != main_thread)
        {
            free_join(th);
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

/*
 * ______________________________________________________________________________________________
 */

#endif
