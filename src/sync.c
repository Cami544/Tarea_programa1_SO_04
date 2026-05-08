#include <stdio.h>
#include <pthread.h>
#include "terminal.h"


void log_operacion(const char *mensaje) {
    pthread_mutex_lock(&mutex_log);
    printf("[LOG] %s\n", mensaje);
    pthread_mutex_unlock(&mutex_log);
}