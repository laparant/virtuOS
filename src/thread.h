#ifndef __THREAD_H__
#define __THREAD_H__

#ifndef USE_PTHREAD

#ifndef VALGRIND
#define VALGRIND 0
#endif

#include <sys/queue.h>
__attribute__ ((constructor)) void thread_create_main (void);
__attribute__ ((destructor)) void thread_exit_main (void);

/* identifiant de thread
 * NB: pourra être un entier au lieu d'un pointeur si ca vous arrange,
 *     mais attention aux inconvénient des tableaux de threads
 *     (consommation mémoire, cout d'allocation, ...).
 */
/*!
 * \brief thread_t thread identifier
 */
typedef void *thread_t;

/* recuperer l'identifiant du thread courant.
 */
/*!
 * \fn extern thread_t thread_self(void)
 * \brief get the identifier of the current thread
 * \return identifier of the current thread
 */
extern thread_t thread_self(void);

/* creer un nouveau thread qui va exécuter la fonction func avec l'argument funcarg.
 * renvoie 0 en cas de succès, -1 en cas d'erreur.
 */
/*!
 * \brief creates a new thread running the function func with arg funcarg
 * \fn extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg);
 * \param newthread
 * \param func the function to run
 * \param funcarg the arguments to func
 * \return 0 on success, -1 on error
 */
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg);

/* passer la main à un autre thread.
 */
/*!
 * \brief yield to another thread
 * \fn extern int thread_yield(void);
 * \return ?
 */
extern int thread_yield(void);

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 * /!\on peut join qu'un thread
 * si on join un thread non initialisé avec thread_create ou non existant le comportement sera indéfini
 */
/*!
 * \brief wait for a thread to finish
 * only one join can be done per thread. If a join is made on a non-existing thread (i.e not created with thread_create) the comportment will be undefined.
 * \fn extern int thread_join(thread_t thread, void **retval);
 * \param thread
 * \param retval
 * \return
 */
extern int thread_join(thread_t thread, void **retval);

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 *
 * L'attribut noreturn aide le compilateur à optimiser le code de
 * l'application (élimination de code mort). Attention à ne pas mettre
 * cet attribut dans votre interface tant que votre thread_exit()
 * n'est pas correctement implémenté (il ne doit jamais retourner).
 */
/*!
 * \brief exiting the current thread and returning retval
 * \fn extern void thread_exit(void *retval) __attribute__ ((__noreturn__));
 * \param retval
 */
extern void thread_exit(void *retval) __attribute__ ((__noreturn__));

/* Interface possible pour les mutex */
/*!
 * \struct thread_mutex
 */
typedef struct thread_mutex
{
    thread_t possessor;
    STAILQ_HEAD(thread_list_mutex, thread) sleep_queue;
} thread_mutex_t;

/*!
 * \brief initiate a mutex
 * \fn int thread_mutex_init(thread_mutex_t *mutex)
 * \param mutex
 * \return
 */
int thread_mutex_init(thread_mutex_t *mutex);

/*!
 * \brief
 * \fn int thread_mutex_destroy(thread_mutex_t *mutex)
 * \param mutex
 * \return
 */
int thread_mutex_destroy(thread_mutex_t *mutex);

/*!
 * \brief
 * \fn int thread_mutex_lock(thread_mutex_t *mutex)
 * \param mutex
 * \return
 */
int thread_mutex_lock(thread_mutex_t *mutex);

/*!
 * \brief
 * \fn int thread_mutex_unlock(thread_mutex_t *mutex)
 * \param mutex
 * \return
 */
int thread_mutex_unlock(thread_mutex_t *mutex);

/* Fonction permettant à l'utilisateur de paramétrer la priorité d'un thread
 * L'argument priority doit être compris entre 1 et 10
 * La priorité influe sur le temps d'exécution du thread:
 * -> plus la priorité est élevée, plus la timeslice sera grande
 * retourne 0 en cas de succès, 1 si la priorité n'est pas valide
 */
extern int thread_set_priority(thread_t thread, short priority);

/* Retourne la valeur actuelle de la priorité du thread */
extern short thread_get_priority(thread_t thread);

#else /* USE_PTHREAD */

/* Si on compile avec -DUSE_PTHREAD, ce sont les pthreads qui sont utilisés */
#include <sched.h>
#include <pthread.h>
#define thread_t pthread_t
#define thread_self pthread_self
#define thread_create(th, func, arg) pthread_create(th, NULL, func, arg)
#define thread_yield sched_yield
#define thread_join pthread_join
#define thread_exit pthread_exit

/* Interface possible pour les mutex */
#define thread_mutex_t            pthread_mutex_t
#define thread_mutex_init(_mutex) pthread_mutex_init(_mutex, NULL)
#define thread_mutex_destroy      pthread_mutex_destroy
#define thread_mutex_lock         pthread_mutex_lock
#define thread_mutex_unlock       pthread_mutex_unlock

#endif /* USE_PTHREAD */

#endif /* __THREAD_H__ */
