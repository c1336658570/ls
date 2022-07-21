#include <iostream>
#include "threadpool.hpp"

void taskFunc(void *arg)
{
    int num = *(int *)arg;
    printf("thread %lu is working, number = %d\n", pthread_self(), num);
}

int main(void)
{
    //创建线程池
    ThreadPool t(20, 30);
    Task task;

    for (int i = 0; i < 100; ++i)
    {
        int *num = (int *)malloc(sizeof(int));
        task.arg = num;
        task.function = taskFunc;
        t.addTask(task);
        *num = i + 100;
    }

    sleep(1);

    return 0;
}