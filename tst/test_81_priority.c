#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "../src/thread.h"

#define N 100

struct timeval time1;
struct timeval time2;
int i;
int stop = 0;
short thread1_priority;
int timeslice = 4000;

void *thread1_func()
{
    while (!stop)
    {

    }
    return NULL;
}

void *thread2_func()
{
    long int S = 0;
    for (i = 0; i < N; i++)
    {
        gettimeofday(&time1, NULL);
        thread_yield();
        gettimeofday(&time2, NULL);
        double diff_us = difftime(time2.tv_usec, time1.tv_usec);
        if (diff_us < 0)
            diff_us = 1000000 + diff_us;
        S += diff_us;
    }
    stop = 1;
    int moy = (int) S/N - 1000; // On retire 4000 car le système préempte
    printf("Priorité du thread: %u, Timeslice moyenne: %d\n", thread1_priority, moy);
    int t,t1;
    int switch_cond = thread1_priority%2;
    switch(switch_cond){
    case 1:
      t = timeslice * (thread1_priority/2+1);
      assert(t*0.95 < moy && moy < t*1.05);
      break;
    default:
      t = timeslice * (thread1_priority/2+1);
      t1 = timeslice * (thread1_priority/2);
      t = (t+t1)/2;
      assert(t*0.95 < moy && moy < t*1.05);
      break;
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
      printf("argument manquant: priorité du thread\n");
      return -1;
    }
    thread_t thread1;
    thread_t thread2;
    thread_create(&thread1, thread1_func, NULL);
    thread_set_priority(thread1, atoi(argv[1]));
    thread1_priority = thread_get_priority(thread1);
    thread_create(&thread2, thread2_func, NULL);
    return 0;
}
