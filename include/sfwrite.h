#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>



void sfwrite(pthread_mutex_t *lock, FILE* stream, char *fmt, ...);