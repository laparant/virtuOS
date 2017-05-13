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
int preempting_time;
int moy; // timeslice obtained with the priority given

int getpreemptingtime(int time) {
    return ((time%12000 + 500)/1000)*1000;
}

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
    moy = (int) S/N - preempting_time; // Removing the architecture constant (cf test 71)
    return NULL;
}

void *mesure_func()
{
    gettimeofday(&time1, NULL);
    thread_yield();
    gettimeofday(&time2, NULL);
    double diff_us = difftime(time2.tv_usec, time1.tv_usec);
    if (diff_us < 0)
        diff_us = 1000000 + diff_us;
    stop = 1;
    preempting_time = getpreemptingtime(diff_us);
    int timeslice_tst = diff_us - preempting_time;
    assert(12000*0.95 < timeslice_tst && timeslice_tst < 12000*1.05);
    return NULL;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
      printf("argument manquant: priorité du thread\n");
      return -1;
    }

    // Calculate the preempting time
    thread_t th_prio5;
    thread_t mesure;
    int err = thread_create(&th_prio5, thread1_func, NULL);
    assert(!err);
    err = thread_create(&mesure, mesure_func,NULL);
    assert(!err);
    err = thread_join(mesure, NULL);
    assert(!err);
    err = thread_join(th_prio5, NULL);
    assert(!err);

    stop = 0;
    //printf("Constante architecture : %d\n",preempting_time);
    // Testing the timeslice of the priority givenr
    thread_t thread1;
    thread_t thread2;
    err = thread_create(&thread1, thread1_func, NULL);
    assert(!err);
    err = thread_set_priority(thread1, atoi(argv[1]));
    assert(!err);
    thread1_priority = thread_get_priority(thread1);
    thread_create(&thread2, thread2_func, NULL);

    // Waiting for the two threads
    thread_join(thread1,NULL);
    thread_join(thread2,NULL);

    // Checking the timeslice
    int wanted_timeslice;
    switch(thread1_priority) {

    case 1  :
        wanted_timeslice = 4000;
        break;
    case 2  :
        wanted_timeslice = 6000;
        break;
    case 3  :
        wanted_timeslice = 8000;
        break;
    case 4  :
        wanted_timeslice = 10000;
        break;
    case 6  :
        wanted_timeslice = 14000;
        break;
    case 7  :
        wanted_timeslice = 16000;
        break;
    case 8  :
        wanted_timeslice = 18000;
        break;
    case 9  :
        wanted_timeslice = 20000;
        break;
    case 10 :
        wanted_timeslice = 22000;
        break;
    default :
        wanted_timeslice = 12000; /* Priority of 5 (used if invalid priority) */
        break;
    }
    assert(wanted_timeslice*0.95 < moy && moy < wanted_timeslice*1.05);
    printf("Priorité du thread: %u\nTimeslice attendue: %d\nTimeslice moyenne: %d\n", thread1_priority, wanted_timeslice, moy);
    return 0;
}
