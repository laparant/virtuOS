#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "../src/thread.h"

int stop;

void *thread1_func()
{
    while (1)//;
        printf("thread1\n");
}

void *thread2_func()
{
    struct timeval time;
    while (1)
    {
        printf("preemption\n");
        gettimeofday(&time, NULL);
        printf("%lds   %ldus\n", time.tv_sec, time.tv_usec);
        thread_yield();
    }
}


int main()
{
    thread_t thread1;
    thread_t thread2;
    void *res;
    thread_create(&thread1, thread1_func, NULL);
    thread_create(&thread2, thread2_func, NULL);
    thread_join(thread2, &res);
    return 0;
}
