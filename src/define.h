/**
  * \file define.h
  * \brief contains macros and definitions of structures
  */
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
#include <unistd.h>

#define CHECK(val, errval, msg) if ((val) == (errval)) {perror(msg); exit(EXIT_FAILURE);}

#define TIMESLICE 4000 // 4 milliseconds in microseconds (Linux clock tick is 4 milliseconds)

// Values for status
#define TO_FREE 2 /*! status for a thread which has terminated and its resources need to be free'd */
#define ALREADY_FREE 1 /*! status for a thead which has terminated and is destroyed */
#define RUNNING 0 /*! status for a running thread */

#define DESTROYED_MUTEX NULL

/*
 * ##############################################################################################
 * ######                                Structures                                        ######
 * ##############################################################################################
 */

/* If value is even, alternate between higher and lower priority
 * in order to have a intermediate value in mean.
 */
typedef struct priority_t
{
    short value; // Between 1 and 10, timeslice between 4ms and 22ms
    short alternate;
} priority_t;


/**
  * \struct thread
  */
typedef struct thread thread;
typedef struct thread
{
    STAILQ_ENTRY(thread) runq_entries; /*!< entry for the runq */
    STAILQ_ENTRY(thread) all_entries; /*!< entry for the all_threads queue */
    STAILQ_ENTRY(thread) to_free_entries; /*!< entry for the to_free queue */
    STAILQ_ENTRY(thread) mutex_queue_entries; /*!< this entry will be used by the mutex */

    thread *joinq; /*!< thread waiting to be joined */
    struct retval *rv; /*!< return value of the thread after finishing */
    ucontext_t *ctx; /*!< execution context */
    int valgrind_stackid; /*!< nobody knew valgrind could be so complicated */
    int status; /*!< status of the thread; see macros above */
    priority_t priority;
} thread;

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                                Static data                                       ######
 * ##############################################################################################
 */

/**
 * \var g_current_thread the thread that is currently being executed
 */
thread * g_current_thread;

/**
 * \var g_all_threads the list of all the threads that were created
 */
STAILQ_HEAD(thread_list_all, thread) g_all_threads;

/**
  * \var g_runq the run queue
  */
STAILQ_HEAD(thread_list_run, thread) g_runq;

/**
 * \var g_to_free the to free queue
 */
STAILQ_HEAD(thread_list_free, thread) g_to_free;

/**
 * @brief set the signal set of the scheduler
 */
sigset_t set;

/*
 * ______________________________________________________________________________________________
 */

#endif
