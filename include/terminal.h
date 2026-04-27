#ifndef TERMINAL_H
#define TERMINAL_H

#include <pthread.h>

typedef struct {
    int inicio;
    int fin;
} RangoHilo;

/* Funciones exportadas desde hilos.c */
void *imprimir_pares(void *arg);
void *imprimir_impares(void *arg);
void ejecutar_hijo_pares(void);
void ejecutar_hijo_impares(void);

#endif /* TERMINAL_H */