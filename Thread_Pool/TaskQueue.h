#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// Single-producer , single-consumer Queue
struct SPSCQueue
{
    int num;
    struct SPSCQueue *next;
    /* Define Your Data Here */

} typedef SPSCQueue;

SPSCQueue *SPSCQueueInit();
void SPSCQueuePush(SPSCQueue *pool, void *s);
void *SPSCQueuePop(SPSCQueue *pool);
void SPSCQueueDestory(SPSCQueue *);

// Multi-producer , Multi-consumer Queue
struct MPMCQueue
{
    int num;
    struct MPMCQueue *next;
    /* Define Your Data Here */
} typedef MPMCQueue;

MPMCQueue *MPMCQueueInit(int threadNumber);
void MPMCQueuePush(MPMCQueue *pool, void *s);
void *MPMCQueuePop(MPMCQueue *pool);
void MPMCQueueDestory(MPMCQueue *);