#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define N 5

void perr_exit(const char *str, int erron);
void *philosopher1(void *arg);
void *philosopher2(void *arg);
void *philosopher3(void *arg);