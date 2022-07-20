#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct ThreadPool ThreadPool;

//创建线程池并初始化
ThreadPool *threadPoolCreate(int min, int max, int queueSize);

//销毁线程池
int threadPoolDestroy(ThreadPool *);

//给线程池添加任务
void threadPoolAdd(ThreadPool *, void (*func)(void *), void *);

//获取线程池中工作线程的个数
int threadPoolBusyNum(ThreadPool *);

//获取线程池中活着的线程的个数
int threadPoolAliveNum(ThreadPool *);
/////////////////////////
void *worker(void *);
void *manager(void *);
void threadExit(ThreadPool *);

#endif
