#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void task_func(void *arg)
{
    int num = *(int *)arg;
    printf("thread %lu is working, number = %d\n", pthread_self(), num);
    //sleep(1);
}

void *add_task(void *arg) {
    ThreadPool * pool = arg;
    for (int i = 0; i < 1000; ++i)
    {
        int *num = (int *)malloc(sizeof(int));
        *num = i + 100;
        thread_pool_add(pool, task_func, (void *)num);
    }

    return NULL;
}

int main(void)
{
    //创建线程池
    ThreadPool *pool = thread_pool_create(3, 10, 100);
    pthread_t tid;

    for (int i = 0; i < 100; ++i)
    {
        pthread_create(&tid, NULL, add_task, pool);
    }

    sleep(10);

    thread_pool_destroy(pool);
    while(1);
    return 0;
}