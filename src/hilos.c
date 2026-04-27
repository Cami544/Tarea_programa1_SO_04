#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "terminal.h"

void *imprimir_pares(void *arg) {
    RangoHilo *r = (RangoHilo *)arg;

    for (int n = r->inicio; n <= r->fin; n += 2) {
        printf("[Hijo PARES   | hilo 0x%lx] %d\n",
               (unsigned long)pthread_self(), n);
    }

    pthread_exit(NULL);
}


void *imprimir_impares(void *arg) {
    RangoHilo *r = (RangoHilo *)arg;

    for (int n = r->inicio; n <= r->fin; n += 2) {
        printf("[Hijo IMPARES | hilo 0x%lx] %d\n",
               (unsigned long)pthread_self(), n);
    }

    pthread_exit(NULL);
}


void ejecutar_hijo_pares(void) {
    pthread_t hilos[2];
    RangoHilo rangos[2];

    rangos[0].inicio =  2; rangos[0].fin =  50;
    rangos[1].inicio = 52; rangos[1].fin = 100;

    for (int i = 0; i < 2; i++) {
        if (pthread_create(&hilos[i], NULL, imprimir_pares, &rangos[i]) != 0) {
            perror("pthread_create (pares)");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(hilos[i], NULL);
    }
}


void ejecutar_hijo_impares(void) {
    pthread_t hilos[2];
    RangoHilo rangos[2];

    rangos[0].inicio =  1; rangos[0].fin =  49;
    rangos[1].inicio = 51; rangos[1].fin =  99;

    for (int i = 0; i < 2; i++) {
        if (pthread_create(&hilos[i], NULL, imprimir_impares, &rangos[i]) != 0) {
            perror("pthread_create (impares)");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(hilos[i], NULL);
    }
}