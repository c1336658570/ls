#include "TaskQueue.h"

SPSCQueue *head, *tail; //队头和队尾
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
    pthread_t pid, cid;
    int ret = 0;

    head = SPSCQueueInit();

    ret = pthread_create(&pid, NULL, producer, NULL);
    if (ret != 0)
    {
        perr_exit("pthread_create fails", ret);
    }
    ret = pthread_create(&cid, NULL, consumer, NULL);
    if (ret != 0)
    {
        perr_exit("pthread_create fails", ret);
    }

    pthread_join(pid, NULL);
    pthread_join(cid, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_product);
    SPSCQueueDestory(head);

    return 0;
}

SPSCQueue *SPSCQueueInit()
{
    head = (SPSCQueue *)malloc(sizeof(SPSCQueue));
    head->next = NULL;
    tail = head;
    return head;
}
void SPSCQueuePush(SPSCQueue *pool, void *s)
{
    SPSCQueue *new = (SPSCQueue *)malloc(sizeof(SPSCQueue));
    new->num = (int)s;
    new->next = NULL;
    printf("producer:num = %d\n", new->num);
    tail->next = new;
    tail = new;
}
void *SPSCQueuePop(SPSCQueue *pool)
{
    SPSCQueue *old = NULL;
    if (head->next != NULL)
    {
        if (head->next == tail)
        {
            tail = head;
        }
        old = head->next;
        head->next = head->next->next;
        printf("consumer:num = %d\n", old->num);
        free(old);
        old = NULL;
    }
}
void SPSCQueueDestory(SPSCQueue *pool)
{
    SPSCQueue *next = NULL;
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
        SPSCQueuePush(tail, (void *)(rand() % 100 + 1));
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
        SPSCQueuePop(head);
        pthread_mutex_unlock(&mutex);

        sleep(rand() % 3);
    }
}