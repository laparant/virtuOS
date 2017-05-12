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

#define CHECK(val, errval, msg) if ((val) == (errval)) {perror(msg); exit(EXIT_FAILURE);}

#define TIMESLICE 8000 // 8 milliseconds in microseconds (Linux clock tick is 4milliseconds)

// Values for status
#define TO_FREE 2 /*! status for a thread which has terminated and its resources need to be free'd */
#define ALREADY_FREE 1 /*! status for a thead which has terminated and is destroyed */
#define RUNNING 0 /*! status for a running thread */

/*
 * ##############################################################################################
 * ######                                Structures                                        ######
 * ##############################################################################################
 */
typedef struct thread_base thread_base;
typedef struct thread thread;

/**
  * \struct thread
  */
typedef struct thread
{
    thread_base *addr; /*!< address of the thread */
    STAILQ_ENTRY(thread) runq_entries; /*!< entry for the runq */
    STAILQ_ENTRY(thread) all_entries; /*!< entry for the all_threads queue */
    STAILQ_ENTRY(thread) to_free_entries; /*!< entry for the to_free queue */
    STAILQ_ENTRY(thread) mutex_queue_entries; /*!< this entry will be used by the mutex */
} thread;

/**
  * \struct thread_base
  */
typedef struct thread_base
{
    thread *joinq; /*!< thread waiting to be joined */
    struct retval *rv; /*!< return value of the thread after finishing */
    ucontext_t *ctx; /*!< execution context */
    int valgrind_stackid; /*!< nobody knew valgrind could be so complicated */
    int status; /*!< status of the thread; see macros above */
} thread_base;

/*
 * ______________________________________________________________________________________________
 */

/*
 * ##############################################################################################
 * ######                                Static datas                                      ######
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

/*
 * ______________________________________________________________________________________________
 */

#endif
