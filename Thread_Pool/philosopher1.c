#include "philosopher.h"

sem_t chopsticks[N];
int philosophers[N];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void perr_exit(const char *str, int erron)
{
    fprintf("%s:%s\n", str, strerror(erron));
    exit(-1);
}

//方法1
void *philosopher1(void *arg)
{
    srand((unsigned)time(NULL));
    int number = *(int *)arg;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        if (number % 2 == 0)
        {
            sem_wait(&chopsticks[(number + 1) % N]);
            sem_wait(&chopsticks[number]);
            printf("%d is eating\n", number);
            sem_post(&chopsticks[(number)]);
            sem_post(&chopsticks[(number + 1) % N]);
        }
        else
        {
            sem_wait(&chopsticks[number]);
            sem_wait(&chopsticks[(number + 1) % N]);
            printf("%d is eating\n", number);
            sem_post(&chopsticks[number]);
            sem_post(&chopsticks[(number + 1) % N]);
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5);
    }

    return (void *)0;
}

int main(void)
{
    int i = 0;
    pthread_t tid[5];
    for (i = 0; i < 5; ++i)
    {
        philosophers[i] = i;
        sem_init(&chopsticks[i], PTHREAD_PROCESS_PRIVATE, 1);
    }

    for (i = 0; i < 5; ++i)
    {
        pthread_create(&tid[i], NULL, philosopher1, (void *)&philosophers[i]);
    }

    for (i = 0; i < 5; ++i)
    {
        pthread_join(tid[i], NULL);
    }
    for (i = 0; i < 5; ++i)
    {
        sem_destroy(&chopsticks[i]);
    }

    return 0;
}