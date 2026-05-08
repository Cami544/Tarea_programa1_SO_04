#include <stdio.h>
#include <pthread.h>
#include "terminal.h"

/*
 * log_operacion — imprime un mensaje en consola de forma thread-safe.
 * Usa mutex_log para evitar que dos hilos escriban al mismo tiempo
 * (race condition en stdout).
 */
void log_operacion(const char *mensaje) {
    pthread_mutex_lock(&mutex_log);
    printf("[LOG] %s\n", mensaje);
    pthread_mutex_unlock(&mutex_log);
}