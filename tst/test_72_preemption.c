#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "../src/thread.h"

struct timeval time1;
struct timeval time2;
int i;
int stop = 0;

void *thread1_func()
{
    while (!stop)
    {

    }
}

void *thread2_func()
{
    for (i = 0; i < 10000; i++)
    {
        gettimeofday(&time1, NULL);
        thread_yield();
        gettimeofday(&time2, NULL);
        double diff = difftime(time2.tv_usec, time1.tv_usec);
        if (diff < 8 && diff > 0)
            printf("%f\n", diff);
        //printf("%d\n", i);
    }
    stop = 1;
    printf("DONE\n");
}


int main()
{
    thread_t thread1;
    thread_t thread2;
    void *res;
    thread_create(&thread1, thread1_func, NULL);
    thread_create(&thread2, thread2_func, NULL);
    return 0;
}
