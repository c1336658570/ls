#include "TaskQueue.h"

MPMCQueue *head, *tail; //队头和队尾
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_product = PTHREAD_COND_INITIALIZER;

void *producer(void *arg);
void *consumer(void *arg);

void perr_exit(const char *str, int erron)
{
    fprintf(stderr, "%s:%s\n", str, strerror(erron));
    exit(-1);
}

int main(void)
{
    int i = 0;
    pthread_t pid[5], cid[5];
    int ret = 0;

    head = MPMCQueueInit(100);

    for (i = 0; i < 5; ++i)
    {
        ret = pthread_create(&pid[i], NULL, producer, NULL);
        if (ret != 0)
        {
            perr_exit("pthread_create fails", ret);
        }
    }
    for (i = 0; i < 5; ++i)
    {
        ret = pthread_create(&cid[i], NULL, consumer, NULL);
        if (ret != 0)
        {
            perr_exit("pthread_create fails", ret);
        }
    }

    for (i = 0; i < 5; ++i)
    {
        pthread_join(pid[i], NULL);
        pthread_join(cid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_product);
    MPMCQueueDestory(head);

    return 0;
}

MPMCQueue *MPMCQueueInit(int threadNumber)
{
    head = (MPMCQueue *)malloc(sizeof(MPMCQueue));
    head->next = NULL;
    tail = head;
    return head;
}
void MPMCQueuePush(MPMCQueue *pool, void *s)
{
    MPMCQueue *new = (MPMCQueue *)malloc(sizeof(MPMCQueue));
    new->num = (int)s;
    new->next = NULL;
    printf(", num = %d\n", new->num);
    tail->next = new;
    tail = new;
}
void *MPMCQueuePop(MPMCQueue *pool)
{
    MPMCQueue *old = NULL;
    if (head->next != NULL)
    {
        if (head->next == tail)
        {
            tail = head;
        }
        old = head->next;
        head->next = head->next->next;
        printf(", num = %d\n", old->num);
        free(old);
        old = NULL;
    }
}
void MPMCQueueDestory(MPMCQueue *pool)
{
    MPMCQueue *next = NULL;
    while (head)
    {
        next = head->next;
        free(head);
        head = next;
    }
    head = tail = NULL;
}

void *producer(void *arg)
{
    srand((unsigned)time(NULL));

    while (1)
    {
        pthread_mutex_lock(&mutex);
        printf("producer:tid = %lu", pthread_self());
        MPMCQueuePush(tail, (void *)(rand() % 100 + 1));
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&has_product);
        sleep(rand() % 3);
    }
}

void *consumer(void *arg)
{
    srand((unsigned)time(NULL));

    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (head->next == NULL)
        {
            pthread_cond_wait(&has_product, &mutex);
        }
        printf("consumer:tid = %lu", pthread_self());
        MPMCQueuePop(head);
        pthread_mutex_unlock(&mutex);

        sleep(rand() % 3);
    }
}