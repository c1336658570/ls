#include "philosopher.h"

sem_t chopsticks[N];
pthread_mutex_t mutex;
int philosophers[N];
int count = 0;

int main(void)
{
    pthread_t tid[N];
    int i = 0;
    for (i = 0; i < N; ++i)
    {
        sem_init(&philosophers[i], PTHREAD_PROCESS_SHARED, 1);
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
        sem_dsetroy(&chopsticks[i]);
    }

    return 0;
}

void *philosopher2(void *arg)
{
    int number = *(int *)arg;

    while (1)
    {
        count++;
    }
}

void perr_exit(const char *str, int erron)
{
    fprintf("%s:%s\n", str, strerror(erron));
    exit(-1);
}