#ifndef __THREADPOOL__
#define __THREADPOOL__
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <queue>
#include <unistd.h>
using namespace std;
using callback = void (*)(void *);
//任务结构体
struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void *arg)
    {
        function = f;
        this->arg = arg;
    }
    callback function;
    void *arg;
};
//任务队列

// 任务队列
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    // 添加任务
    void addTask(Task &task);
    void addTask(callback func, void *arg);

    // 取出一个任务
    Task takeTask();

    // 获取当前队列中任务个数
    inline int taskNumber()
    {
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;  // 互斥锁
    std::queue<Task> m_queue; // 任务队列
};
TaskQueue::TaskQueue()
{
    pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task &task)
{
    pthread_mutex_lock(&m_mutex);
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void TaskQueue::addTask(callback func, void *arg)
{
    pthread_mutex_lock(&m_mutex);
    Task task;
    task.function = func;
    task.arg = arg;
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::takeTask()
{
    Task t;
    pthread_mutex_lock(&m_mutex);
    if (m_queue.size() > 0)
    {
        t = m_queue.front();
        m_queue.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}
class ThreadPool
{
public:
    ThreadPool(int num);
    ~ThreadPool();
    // 添加任务
    void addTask(Task task);

private:
    // 工作的线程的任务函数
    static void *worker(void *arg);

private:
    TaskQueue *m_TaskQ;        //任务队列
    int threadNUM;             //线程数量
    int tasksize;              //任务数量
    pthread_mutex_t mutexpool; //锁整个线程池
    pthread_cond_t notempty;   //任务队列是不是空
    int shutdown;              //是不是要销毁线程池，销毁为1，不销毁为0
};
ThreadPool::ThreadPool(int num)
{
    shutdown = 0;
    // 实例化任务队列
    m_TaskQ = new TaskQueue;
    do
    {
        // 初始化线程池
        threadNUM = num;
        // 初始化互斥锁,条件变量
        if (pthread_mutex_init(&mutexpool, NULL) != 0 ||
            pthread_cond_init(&notempty, NULL) != 0)
        {
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        /////////////////// 创建线程 //ask.arg = nullptr;////////////////
        // 根据数, 创建线程
        for (int i = 0; i < threadNUM; ++i)
        {
            pthread_t tid;
            pthread_create(&tid, NULL, worker, this);
        }

    } while (0);
}
ThreadPool::~ThreadPool()
{
    shutdown = 1;

    // 唤醒所有消费者线程
    for (int i = 0; i < threadNUM; ++i)
    {
        pthread_cond_signal(&notempty);
    }
    //销毁任务队列
    if (m_TaskQ)
        delete m_TaskQ;
    pthread_mutex_destroy(&mutexpool);
    pthread_cond_destroy(&notempty);
}

//往任务队列里面添加任务,当任务添加成后，唤醒线程，来拿任务
void ThreadPool::addTask(Task task)
{
    if (shutdown)
    {
        return;
    }
    // 添加任务，不需要加锁，任务队列中有锁
    m_TaskQ->addTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&notempty);
}

// 工作线程任务函数
void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    // 一直不停的工作
    while (true)
    {
        // 访问任务队列(共享资源)加锁
        pthread_mutex_lock(&pool->mutexpool);
        // 判断任务队列是否为空, 如果为空工作线程阻塞
        while (pool->m_TaskQ->taskNumber() == 0 && pool->shutdown == 0)
        {
            cout << "thread  waiting..." << endl;
            // 阻塞线程
            pthread_cond_wait(&pool->notempty, &pool->mutexpool);
        }
        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexpool);
            pthread_exit(NULL);
        }
        // cout<<"adasdas"<<endl;
        //  从任务队列中取出一个任务
        Task task = pool->m_TaskQ->takeTask();
        // 线程池解锁
        pthread_mutex_unlock(&pool->mutexpool);
        // cout << "adasdas" << endl;
        //  执行任务
        cout << "thread  start working..." << endl;
        task.function(task.arg);

        // 任务处理结束
        cout << "thread  end working...";
    }

    return nullptr;
}

#endif