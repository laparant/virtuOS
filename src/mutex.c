#include "thread.h"
#include "define.h"

/**
 * @brief thread_mutex_init initializes a mutex
 * @param mutex the mutex to initialize
 * @return EXIT_SUCCESS on success
 */
int thread_mutex_init(thread_mutex_t *mutex)
{
    mutex->possessor = NULL;
    STAILQ_INIT(&(mutex->sleep_queue));
    return EXIT_SUCCESS;
}

/**
 * @brief thread_mutex_destroy frees the mutex if possible
 * @param mutex the mutex to free
 * @return EXIT_FAILURE if the mutex is waited by others threads
 *         EXIT_SUCCESS on success
 */
int thread_mutex_destroy(thread_mutex_t *mutex)
{
    if (STAILQ_EMPTY(&(mutex->sleep_queue)))
    {
        mutex = DESTROYED_MUTEX;
        return EXIT_SUCCESS;
    }
    else
        return EXIT_FAILURE;
}

/**
 * @brief thread_mutex_lock locks the mutex for others threads, except the possessor
 * @param mutex the mutex to lock
 * @return EXIT_SUCCESS on success
 *         EXIT FAILURE if the mutex is destroyed
 */
int thread_mutex_lock(thread_mutex_t *mutex)
{
    // Detecting destroyed mutex
    if (mutex == DESTROYED_MUTEX)
        return EXIT_FAILURE;

    /* Unavailable mutex : waiting for the mutex */
    while (mutex->possessor != NULL)
    {
        thread *me = thread_self();
        STAILQ_INSERT_TAIL(&(mutex->sleep_queue), me, mutex_queue_entries);
        thread *new_current = STAILQ_FIRST(&g_runq);
        STAILQ_REMOVE_HEAD(&g_runq, runq_entries);
        thread *tmp = g_current_thread;
        g_current_thread = new_current;
        CHECK(swapcontext(tmp->ctx, new_current->ctx), -1, "thread_mutex_lock: swapcontext")
    }
    /* Available mutex */
    mutex->possessor = thread_self();
    return EXIT_SUCCESS;
}

/**
 * @brief thread_mutex_unlock unlocks the mutex is the thread is the possessor
 * @param mutex the mutex to unlock
 * @return EXIT_FAILURE if the mutex is destroyed or possessed by another thread
 *         EXIT_SUCCESS on success
 */
int thread_mutex_unlock(thread_mutex_t *mutex)
{
    // Detecting destroyed mutex
    if (mutex == DESTROYED_MUTEX)
        return EXIT_FAILURE;

    /* I'm not the possessor */
    if (mutex->possessor != thread_self()) return EXIT_FAILURE;

    /* I'm the possessor */
    // Waking up the next thread waiting for the mutex */
    if (!STAILQ_EMPTY(&(mutex->sleep_queue)))
    {
        thread *next = STAILQ_FIRST(&(mutex->sleep_queue));
        STAILQ_REMOVE_HEAD(&(mutex->sleep_queue), mutex_queue_entries);
        STAILQ_INSERT_HEAD(&g_runq, next, runq_entries);
    }
    // Get the mutex available
    mutex->possessor = NULL;

    return EXIT_SUCCESS;
}
