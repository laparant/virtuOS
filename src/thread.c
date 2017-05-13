#include "thread.h"

#ifndef USE_PTHREAD
#include "retval.h"
#include "define.h"

/*
 * ##############################################################################################
 * ######                             Cleaning processes                                   ######
 * ##############################################################################################
 */
void free_context(thread *th)
{
    STAILQ_REMOVE(&g_to_free, th, thread, to_free_entries);
    /* Free the resources */
    VALGRIND_STACK_DEREGISTER(th->valgrind_stackid);
    free(th->ctx->uc_stack.ss_sp);
    free(th->ctx);
    th->status = ALREADY_FREE;
}

void free_join(thread *th)
{
    if (th->status == TO_FREE)
    {
        free_context(th);
    }
    free_retval(th->rv);
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
        th->priority.value = priority;
        return EXIT_SUCCESS;
    }
}

short thread_get_priority(thread_t thread)
{
    struct thread *th = (struct thread *) thread;
    return th->priority.value;
}

__useconds_t get_priority_timeslice(thread *th)
{
    if (th->priority.value%2 == 1) // priority is an odd value
    {
        return TIMESLICE * (th->priority.value/2 + 1);
    }
    else // priority is an even value so we need to alternate between higher and lower timeslices
    {
        if (th->priority.alternate)
        {
            th->priority.alternate = 0;
            return TIMESLICE * (th->priority.value/2 + 1);
        }
        else
        {
            th->priority.alternate = 1;
            return TIMESLICE * (th->priority.value/2);
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

void reset_timer()
{
    struct itimerval newtimer;
    CHECK(getitimer(ITIMER_PROF, &newtimer), -1, "reset_timer: getitimer")
    newtimer.it_value.tv_usec = get_priority_timeslice(g_current_thread);
    CHECK(setitimer(ITIMER_PROF, &newtimer, NULL), -1, "reset_timer: getitimer")
}

void enable_interruptions()
{
    CHECK(sigprocmask(SIG_UNBLOCK, &set, NULL), -1, "enable_interruptions: sigprocmask")
}

void disable_interruptions()
{
    CHECK(sigprocmask(SIG_BLOCK, &set, NULL), -1, "disable_interruptions: sigprocmask")
}

void alarm_handler(int signal)
{
    disable_interruptions();
    if(!STAILQ_EMPTY(&g_runq)) {thread_yield();}
    enable_interruptions();
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                           Stack overflow utilitary                               ######
 * ##############################################################################################
 */

void stack_overflow()
{
    thread * th = (thread *) thread_self();
    CHECK(mprotect(th->ctx->uc_stack.ss_sp, 2*PAGE_SIZE, PROT_NONE),-1, "init_context : mprotect")
}

void sigsegv_handler(int signum, siginfo_t *info, void *data) {
    printf("/!\\ SEGFAULT (stack overflow) /!\\ thread %p\n",(thread *) thread_self());
    thread_exit(SEGFAULT);
}

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                            Auxiliary processes                                   ######
 * ##############################################################################################
 */

void force_exit(void *(*func)(void *), void *funcarg)
{
    stack_overflow();
    if (func != NULL)
    {
        enable_interruptions();
        void *res = func(funcarg);
        thread_exit(res);
    }
}

void finalize_join(thread *th, void **retval)
{
    /* Collecting the value of retval */
    if (retval) *retval = get_value(th->rv);

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

thread *init_context(void *(*func)(void *), void *funcarg)
{
    thread *th = malloc(sizeof(thread));
    CHECK(th, NULL, "init_context: thread pointer malloc")
    th->ctx = malloc(sizeof(ucontext_t));
    CHECK(th->ctx, NULL, "init_context: context malloc")
    getcontext(th->ctx);

    th->ctx->uc_stack.ss_size = NB_PAGES * PAGE_SIZE;
    th->ctx->uc_stack.ss_sp = valloc(th->ctx->uc_stack.ss_size);
    int valgrind_stackid = VALGRIND_STACK_REGISTER(th->ctx->uc_stack.ss_sp,
                                                   th->ctx->uc_stack.ss_sp + th->ctx->uc_stack.ss_size);
    th->valgrind_stackid=valgrind_stackid;
    th->status = RUNNING;
    th->ctx->uc_link = NULL;

    makecontext(th->ctx, (void (*)(void)) force_exit, 2, func, funcarg);

    return th;
}

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
    disable_interruptions();
    /* Initialization of the context */
    thread *th = init_context(func, funcarg);

    /* Initialization of the return value */
    th->rv = init_retval();

    /* Initialize the thread's sleep queue */
    th->joinq = NULL;

    /* Add the thread to the scheduler */
    add_to_scheduler(th);

    /* Give a default priority of 5 */
    th->priority.value = 5;
    th->priority.alternate = 0;

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

    /* Reset the timer for the new thread */
    reset_timer();
    CHECK(swapcontext(tmp->ctx, new_current->ctx), -1, "thread_yield: swapcontext")
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
    if (th == me->joinq)
        return EDEADLK;

    /* Detecting if the thread is already joined by another thread */
    if (th->joinq != NULL)
        return EINVAL;

    /* If the thread has already finished */
    if (th->status != RUNNING)
    {
        disable_interruptions();
        finalize_join(th, retval);
        enable_interruptions();
        return EXIT_SUCCESS;
    }

    /* If the thread is alive */
    /* Sleeping while the thread hasn't finished */
    disable_interruptions();
    th->joinq = me;
    struct thread *new_current = STAILQ_FIRST(&g_runq);
    STAILQ_REMOVE_HEAD(&g_runq, runq_entries);
    struct thread *tmp = g_current_thread;
    g_current_thread = new_current;
    reset_timer();
    CHECK(swapcontext(tmp->ctx, new_current->ctx), -1, "thread_join: swapcontext")
    enable_interruptions();

    /* When woke up (thread is finished) */
    disable_interruptions();
    finalize_join(th, retval);
    enable_interruptions();
    return EXIT_SUCCESS;
}

__attribute__ ((__noreturn__)) void thread_exit(void *retval)
{
    disable_interruptions();
    thread *me = (thread *) thread_self();
    me->status = TO_FREE;

    /* Set the retval */
    if (retval) me->rv->value = retval;

    /* Waking up the thread waiting for me */
    if (me->joinq != NULL)
        STAILQ_INSERT_TAIL(&g_runq, me->joinq, runq_entries);

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
        CHECK(setcontext(g_current_thread->ctx), -1, "thread_exit: setcontext")
    }
    /* Main */
    else
    {
        CHECK(swapcontext(me->ctx, g_current_thread->ctx), -1, "thread_exit: swapcontext")
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
    th->rv = init_retval();

    /* Initialize the thread's sleep queue */
    th->joinq = NULL;

    /* Give a default priority of 5 */
    th->priority.value = 5;
    th->priority.alternate = 0;

    /* Initialization of the queues */
    STAILQ_INIT(&g_all_threads);
    STAILQ_INIT(&g_runq);
    STAILQ_INIT(&g_to_free);

    /* Add the thread to the scheduler */
    g_current_thread = th;
    STAILQ_INSERT_HEAD(&g_all_threads, th, all_entries);

    /* ---- Setting up the segfault handler ---- */
    stack_t segv_stack;
    segv_stack.ss_sp = valloc(SIGSTKSZ);
    segv_stack.ss_flags = 0;
    segv_stack.ss_size = SIGSTKSZ;
    sigaltstack(&segv_stack, NULL);

    struct sigaction action;
    bzero(&action, sizeof(action));
    action.sa_flags = SA_SIGINFO|SA_STACK;
    action.sa_sigaction = &sigsegv_handler;
    sigaction(SIGSEGV, &action, NULL);


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
    sigemptyset(&set);
    sigaddset(&set, SIGPROF);
    CHECK(setitimer(ITIMER_PROF, &timer, NULL), -1, "thread_create_main: setitimer")
    enable_interruptions();
}

__attribute__ ((destructor)) void thread_exit_main(void)
{
    thread *th;
    thread *me = (thread *) thread_self();
    if (me->status == RUNNING)
    {
        disable_interruptions();
        me->status = TO_FREE;

        /* Set the retval */
        //if (retval) me->rv->value = retval;
        // could not be done because no retval when main just does "return" and we can't use force_exit

        /* Waking up the thread waiting for me */
        if (me->joinq != NULL)
            STAILQ_INSERT_TAIL(&g_runq, me->joinq, runq_entries);

        /* If others threads are running */
        while (!STAILQ_EMPTY(&g_runq))
        {
            thread *new_current = STAILQ_FIRST(&g_runq);
            g_current_thread = new_current;
            STAILQ_REMOVE_HEAD(&g_runq, runq_entries);

            /* Leaving the runqueue */
            reset_timer();
            CHECK(swapcontext(me->ctx, new_current->ctx), -1, "thread_exit_main: swapcontext")
        }
        enable_interruptions();
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

    VALGRIND_STACK_DEREGISTER(main_thread->valgrind_stackid);
    free(main_thread->ctx->uc_stack.ss_sp);
    free(main_thread->ctx);
    free_retval(main_thread->rv);
    free(main_thread);

    STAILQ_INIT(&g_all_threads);
}

/*
 * ______________________________________________________________________________________________
 */

#endif
