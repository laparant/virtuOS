#ifndef DEFINE_H
#define DEFINE_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/queue.h> // Using the singly linked tail queue STAILQ for runq
#include <valgrind/valgrind.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>

#define CHECK(val, errval, msg) if ((val) == (errval)) {perror(msg); exit(EXIT_FAILURE);}

#define TIMESLICE 4000 // 4 milliseconds in microseconds (Linux clock tick is 4 milliseconds)

// Values for status
#define TO_FREE 2
#define ALREADY_FREE 1
#define RUNNING 0

/*
 * ##############################################################################################
 * ######                                Structures                                        ######
 * ##############################################################################################
 */
typedef struct thread_base thread_base;
typedef struct thread thread;

/* If value is even, alternate between higher and lower priority
 * in order to have a intermediate value in mean.
 */
typedef struct priority_t
{
    short value; // Between 1 and 10, timeslice between 4ms and 22ms
    short alternate;
} priority_t;

typedef struct thread
{
    thread_base *addr;
    STAILQ_ENTRY(thread) runq_entries; // This entry will be used for the runq
    STAILQ_ENTRY(thread) all_entries; // This entry will be used for the all_threads queue
    STAILQ_ENTRY(thread) to_free_entries; // This entry will be used for the to_free queue
    STAILQ_ENTRY(thread) mutex_queue_entries; // This entry will be used by the mutex
} thread;

typedef struct thread_base
{
    thread *joinq;
    struct retval *rv;
    ucontext_t *ctx;
    int valgrind_stackid;
    int status;
    priority_t priority;
} thread_base;

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                                Static data                                       ######
 * ##############################################################################################
 */

// The thread that is currently being executed
thread *g_current_thread;

// The list of all the threads that were created
STAILQ_HEAD(thread_list_all, thread) g_all_threads;

// The run queue
STAILQ_HEAD(thread_list_run, thread) g_runq;

// The to free queue
STAILQ_HEAD(thread_list_free, thread) g_to_free;

/*
 * ______________________________________________________________________________________________
 */

#endif
