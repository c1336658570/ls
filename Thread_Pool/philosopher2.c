#include "philosopher.h"

pthread_mutex_t chopsticks[N];
pthread_mutex_t mutex;
int philosophers[N];

int main(void)
{
    pthread_t tid[N];
    int i = 0;
    for (i = 0; i < N; ++i)
    {
        pthread_mutex_init(&chopsticks[i], NULL);
        philosophers[i] = i;
    }

    for (i = 0; i < 5; ++i)
    {
        int ret = pthread_create(&tid[i], NULL, philosopher2, (void *)&philosophers[i]);
        if (ret != 0)
        {
            perr_exit("pthread_create fails", ret);
        }
    }

    for (i = 0; i < N; ++i)
    {
        pthread_join(tid[i], NULL);
    }
    for (i = 0; i < N; ++i)
    {
        pthread_mutex_destroy(&chopsticks[i]);
    }
    pthread_mutex_destroy(&mutex);

    return 0;
}

void *philosopher2(void *arg)
{
    srand((unsigned)time(NULL));
    int number = *(int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mutex);
        if (!pthread_mutex_trylock(&chopsticks[number]))
        {
            if (!pthread_mutex_trylock(&chopsticks[(number + 1) % N]))
            {
                printf("%d is eating\n", number);

                pthread_mutex_unlock(&chopsticks[number]);
                pthread_mutex_unlock(&chopsticks[(number + 1) % N]);
            }
            else
            {
                pthread_mutex_unlock(&chopsticks[number]);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5);
    }

    return (void *)0;
}

void perr_exit(const char *str, int erron)
{
    fprintf(stderr, "%s:%s\n", str, strerror(erron));
    exit(-1);
}