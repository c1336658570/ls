#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define NUM 10
typedef int produce;

void perr_exit(const char *str, int erron)
{
    fprintf(stderr, "%s:%s\n", str, strerror(erron));
    exit(-1);
}

produce queue[NUM];
sem_t black_num, product_num;

void *producer(void *arg)
{
    srand((unsigned)time(NULL));
    int i = 0;

    while (1)
    {
        sem_wait(&black_num);
        queue[i] = rand() % 100 + 1;
        printf("----producer:tid = %lu, num = %d\n", pthread_self(), queue[i]);
        i = (i + 1) % NUM;
        sem_post(&product_num);

        sleep(rand() % 3);
    }
}

void *consumer(void *arg)
{
    int i = 0;

    while (1)
    {
        sem_wait(&product_num);
        printf("----consumer:tid = %lu, num = %d\n", pthread_self(), queue[i]);
        queue[i] = 0;
        i = (i + 1) % NUM;
        sem_post(&black_num);

        sleep(rand() % 3);
    }
}

int main(void)
{
    int ret = 0;
    pthread_t pid, cid;

    sem_init(&black_num, PTHREAD_PROCESS_PRIVATE, NUM);
    sem_init(&product_num, PTHREAD_PROCESS_PRIVATE, 0);

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

    sem_destroy(&black_num);
    sem_destroy(&product_num);

    return 0;
}