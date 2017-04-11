#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ucontext.h>
#include <sys/queue.h>
#include "../src/retval.h"
#include "../src/thread.h"

typedef struct thread
{
    struct sleep_queue *sleepq;
    struct retval *rv;
    ucontext_t *ctx;

    STAILQ_ENTRY(thread) entries;
} thread;

static thread_t g_current_thread;


///////*test_function*/////////

int test_thread_self(){
  g_current_thread = malloc (sizeof(thread));
  //printf("%p\n",g_current_thread);
  //printf("%p\n",thread_self());
  if (g_current_thread != thread_self()){
    free(g_current_thread);
    return EXIT_FAILURE;
  }
  free(g_current_thread);
  return EXIT_SUCCESS;
}


void * fc_test_create (void * arg){
  int *i = (int *) arg;
  *i ++;
  return (void *) i;
}

int test_thread_create(){
  printf("test_tread_create : ");
  thread_t th;
  int * arg;
  *arg = 42;
  thread_create(th, fc_test_create,(void *) arg);
 
  if (*arg != 43)
    return EXIT_FAILURE;
    
  printf(" OK\n");
  return EXIT_SUCCESS;
}

int test_thread_yield(){

  return EXIT_SUCCESS;
}

int test_thread_join(){

  return EXIT_SUCCESS;
}

int test_thread_exit(){

  return EXIT_SUCCESS;
}

int test_force_exit(){

  return EXIT_SUCCESS;
}

/////*MAIN*//////

int main(){
  printf("main");
  //assert(test_thread_self() == EXIT_SUCCESS &&"test_thread_self failed");
  assert(test_thread_create()== EXIT_SUCCESS &&"test_thread_create failed");
  assert(test_thread_yield()== EXIT_SUCCESS &&"test_thread_yield failed");
  assert(test_thread_join()==EXIT_SUCCESS &&"test_thread_join failed");
  assert(test_thread_exit()==EXIT_SUCCESS &&"test_thread_exit failed");
  assert(test_force_exit()==EXIT_SUCCESS &&"test_force_exit failed");

  return EXIT_SUCCESS;
}




//  assert(()&&());
