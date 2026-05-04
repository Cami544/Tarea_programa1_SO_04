#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "terminal.h"


/* ── Variables globales compartidas  */
sem_t           semaforo_muelles;  
pthread_mutex_t mutex_log;          
int             camiones_completados = 0;

int main(void) {
    srand((unsigned int)time(NULL));

    printf("========================================================\n");
    printf("  EIF212 — SIMULADOR TERMINAL DE CARGA\n");
    printf("  Muelles: %d  |  Camiones: %d\n", NUM_MUELLES, NUM_CAMIONES);
    printf("========================================================\n\n");

    Camion    camiones[NUM_CAMIONES];
    pthread_t hilos[NUM_CAMIONES];

    for (int i = 0; i < NUM_CAMIONES; i++) {
        inicializar_camion(&camiones[i], i + 1);
    }

    imprimir_tabla_camiones(camiones, NUM_CAMIONES);

    printf("--- Iniciando simulación ---\n\n");

    for (int i = 0; i < NUM_CAMIONES; i++) {
        if (pthread_create(&hilos[i], NULL, rutina_camion, &camiones[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        usleep(200000); 
    }

    for (int i = 0; i < NUM_CAMIONES; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\n--- Simulación finalizada ---\n");

    printf("========================================================\n");
    printf("  Todos los camiones completaron su ciclo de vida.\n");
    printf("========================================================\n");

    return EXIT_SUCCESS;
}
