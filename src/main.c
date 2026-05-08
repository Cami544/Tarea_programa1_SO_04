#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include "terminal.h"

sem_t           semaforo_muelles;
pthread_mutex_t mutex_log;
int             camiones_completados = 0;

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    const char *algoritmo = "fifo";
    int         quantum   = 3;

    if (argc >= 2) {
        algoritmo = argv[1];
    }
    if (argc >= 3) {
        quantum = atoi(argv[2]);
        if (quantum <= 0) {
            fprintf(stderr, "Error: el quantum debe ser un entero positivo.\n");
            return EXIT_FAILURE;
        }
    }

    printf("========================================================\n");
    printf("  EIF212 — SIMULADOR TERMINAL DE CARGA\n");
    printf("  Muelles: %d  |  Camiones: %d\n", NUM_MUELLES, NUM_CAMIONES);
    printf("  Algoritmo: %s", algoritmo);
    if (strcmp(algoritmo, "rr") == 0) printf("  |  Quantum: %ds", quantum);
    printf("\n");
    printf("========================================================\n\n");

    Camion    camiones[NUM_CAMIONES];
    pthread_t hilos[NUM_CAMIONES];

    for (int i = 0; i < NUM_CAMIONES; i++) {
        inicializar_camion(&camiones[i], i + 1);
    }

    imprimir_tabla_camiones(camiones, NUM_CAMIONES);

    if (sem_init(&semaforo_muelles, 0, NUM_MUELLES) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&mutex_log, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }

    printf("--- Iniciando simulación ---\n\n");

    for (int i = 0; i < NUM_CAMIONES; i++) {
        if (pthread_create(&hilos[i], NULL, rutina_camion, &camiones[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        usleep(200000);
    }

    iniciar_planificador(algoritmo, quantum);

    for (int i = 0; i < NUM_CAMIONES; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\n--- Simulación finalizada ---\n");

    sem_destroy(&semaforo_muelles);
    pthread_mutex_destroy(&mutex_log);

    printf("========================================================\n");
    printf("  Todos los camiones completaron su ciclo de vida.\n");
    printf("========================================================\n");

    return EXIT_SUCCESS;
}