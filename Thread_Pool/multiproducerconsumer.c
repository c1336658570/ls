#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef int produce;

struct goods
{
    produce num;
    struct goods *next;
};

struct goods *head;
pthread_mutex_t mutex;
pthread_cond_t has_product;

void perr_exit(const char *str, int erron)
{
    fprintf(stderr, "%s:%s\n", str, strerror(erron));
    exit(-1);
}

void *producer(void *arg)
{
    srand((unsigned)time(NULL));
    struct goods *new = NULL;

    while (1)
    {
        new = (struct goods *)malloc(sizeof(struct goods));
        new->num = rand() % 100 + 1;
        printf("----producer:tid = %lu, num = %d\n", pthread_self(), new->num);

        pthread_mutex_lock(&mutex);
        new->next = head;
        head = new;
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&has_product);

        sleep(rand() % 3);
    }

    return NULL;
}

void *consumer(void *arg)
{
    srand((unsigned)time(NULL));
    struct goods *old;

    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (head == NULL)
        {
            pthread_cond_wait(&has_product, &mutex);
        }
        old = head;
        head = head->next;
        pthread_mutex_unlock(&mutex);

        printf("----consumer:tid = %lu, num = %d\n", pthread_self(), old->num);
        free(old);
        old = NULL;

        sleep(rand() % 3);
    }

    return NULL;
}

int main(void)
{
    int ret = 0, i = 0;
    pthread_t pid[5], cid[5];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&has_product, NULL);

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

    pthread_exit((void *)0);
}