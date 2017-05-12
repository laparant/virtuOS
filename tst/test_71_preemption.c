#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "../src/thread.h"

struct timeval time1;

void *thread1_func()
{
    while (1)
    {
        gettimeofday(&time1, NULL);
    }
    return NULL;
}

void *thread2_func()
{
    while (1)
    {
        gettimeofday(&time1, NULL);
        printf("%ld  %ld\n", time1.tv_sec, time1.tv_usec);
        thread_yield();
    }
    return NULL;
}


int main()
{
    thread_t thread1;
    thread_t thread2;
    thread_create(&thread1, thread1_func, NULL);
    thread_create(&thread2, thread2_func, NULL);
    return 0;
}
