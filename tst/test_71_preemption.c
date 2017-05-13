#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "../src/thread.h"

struct timeval time1;
int stop = 0;

void *thread1_func()
{
    while (!stop)
    {
        gettimeofday(&time1, NULL);
    }
    return NULL;
}

void *thread2_func()
{
    int i = 0;
    while (i < 100)
    {
        gettimeofday(&time1, NULL);
        printf("[Thread 2] sec : %ld usec : %ld\n", time1.tv_sec, time1.tv_usec);
        i++;
        thread_yield();
    }
    stop = 1;
    return NULL;
}


int main()
{
    thread_t thread1;
    thread_t thread2;
    int err = thread_create(&thread1, thread1_func, NULL);
    assert(!err);
    err = thread_create(&thread2, thread2_func, NULL);
    assert(!err);

    err = thread_join(thread1, NULL);
    assert(!err);
    err = thread_join(thread2,NULL);
    assert(!err);
    printf("The thread 2 has been executed after a while(1) thread\n");

    return 0;
}
