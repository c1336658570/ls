#ifndef _THREADPOOL_H
#define _THREADPOOL_H

typedef struct ThreadPool ThreadPool;
// 创建线程池并初始化
ThreadPool *thread_pool_create(int min, int max, int queueSize);

// 销毁线程池
int thread_pool_destroy(ThreadPool* pool);

// 给线程池添加任务
void thread_pool_add(ThreadPool* pool, void(*func)(void*), void* arg);

// 获取线程池中工作的线程的个数
int thread_pool_busy_num(ThreadPool* pool);

// 获取线程池中活着的线程的个数
int thread_pool_alive_num(ThreadPool* pool);

//////////////////////
// 工作的线程(消费者线程)任务函数
void* worker(void* arg);
// 管理者线程任务函数
void* manager(void* arg);
// 单个线程退出
void thread_exit(ThreadPool* pool);
// 线程池关闭
void shutdown_exit();
#endif