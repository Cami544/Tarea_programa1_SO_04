#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "terminal.h"

static Camion         *cola[NUM_CAMIONES];
static int             cola_size  = 0;
static pthread_mutex_t mutex_cola = PTHREAD_MUTEX_INITIALIZER;

void encolar_camion(Camion *c) {
    pthread_mutex_lock(&mutex_cola);
    if (cola_size < NUM_CAMIONES) {
        cola[cola_size++] = c;
    }
    pthread_mutex_unlock(&mutex_cola);
}

static double calcular_espera(Camion **arr, int n) {
    double total = 0.0;
    for (int i = 0; i < n; i++)
        total += (double)(arr[i]->tiempo_inicio - arr[i]->tiempo_llegada);
    return n > 0 ? total / n : 0.0;
}

static double calcular_turnaround(Camion **arr, int n) {
    double total = 0.0;
    for (int i = 0; i < n; i++)
        total += (double)(arr[i]->tiempo_fin - arr[i]->tiempo_llegada);
    return n > 0 ? total / n : 0.0;
}

static void imprimir_comparativa(double ef, double tf, double er, double tr) {
    printf("\n========================================================\n");
    printf("      COMPARATIVA DE ALGORITMOS DE PLANIFICACION\n");
    printf("========================================================\n");
    printf("  %-12s | %-18s | %-18s\n", "Algoritmo", "Espera Promedio", "Turnaround Promedio");
    printf("  -------------+--------------------+--------------------\n");
    printf("  %-12s | %14.2f s   | %14.2f s\n", "FIFO",        ef, tf);
    printf("  %-12s | %14.2f s   | %14.2f s\n", "Round Robin", er, tr);
    printf("========================================================\n");
    printf("  FIFO        - Sin desalojo. Puede sufrir efecto convoy.\n");
    printf("  Round Robin - Mayor equidad; mas cambios de contexto.\n");
    printf("========================================================\n\n");
    fflush(stdout);
}

static void marcar_terminado(Camion *c) {
    pthread_mutex_lock(&c->mutex_turno);
    c->terminado = 1;
    pthread_cond_signal(&c->cond_turno);
    pthread_mutex_unlock(&c->mutex_turno);
}


static void ejecutar_fifo(Camion **arr, int n) {
    printf("\n--- [FIFO] Iniciando despacho ---\n\n");
    fflush(stdout);

    for (int i = 0; i < n; i++) {
        Camion *c = arr[i];

        pthread_mutex_lock(&mutex_log);
        reportar_estado(c, ESTADO_BLOQUEADO);
        pthread_mutex_unlock(&mutex_log);

        sem_wait(&semaforo_muelles);

        c->tiempo_inicio = time(NULL);

        pthread_mutex_lock(&mutex_log);
        reportar_estado(c, ESTADO_EJECUCION);
        pthread_mutex_unlock(&mutex_log);

        sleep(c->burst_restante);
        c->burst_restante = 0;

        sem_post(&semaforo_muelles);

        c->tiempo_fin = time(NULL);
        marcar_terminado(c);
    }
}


static void ejecutar_rr(Camion **arr, int n, int quantum) {
    printf("\n--- [Round Robin] Quantum=%ds - Iniciando despacho ---\n\n", quantum);
    fflush(stdout);

    Camion *rr[NUM_CAMIONES * 20];
    int head = 0, tail = 0;
    for (int i = 0; i < n; i++) rr[tail++] = arr[i];

    while (head < tail) {
        Camion *c = rr[head++];
        if (c->burst_restante <= 0) continue;

        int rodaja = (c->burst_restante < quantum) ? c->burst_restante : quantum;

        pthread_mutex_lock(&mutex_log);
        reportar_estado(c, ESTADO_BLOQUEADO);
        pthread_mutex_unlock(&mutex_log);

        sem_wait(&semaforo_muelles);

        if (c->tiempo_inicio == 0) c->tiempo_inicio = time(NULL);

        pthread_mutex_lock(&mutex_log);
        reportar_estado(c, ESTADO_EJECUCION);
        pthread_mutex_unlock(&mutex_log);

        sleep(rodaja);
        c->burst_restante -= rodaja;

        sem_post(&semaforo_muelles);

        if (c->burst_restante > 0) {
            pthread_mutex_lock(&mutex_log);
            reportar_estado(c, ESTADO_LISTO);
            pthread_mutex_unlock(&mutex_log);
            rr[tail++] = c;
        } else {
            c->tiempo_fin = time(NULL);
            marcar_terminado(c);
        }
    }
}


void iniciar_planificador(const char *algoritmo, int quantum) {
    while (cola_size < NUM_CAMIONES) {
        usleep(50000);
    }

    int    bursts_orig[NUM_CAMIONES];
    time_t llegadas_orig[NUM_CAMIONES];
    for (int i = 0; i < cola_size; i++) {
        bursts_orig[i]   = cola[i]->burst_cpu;
        llegadas_orig[i] = cola[i]->tiempo_llegada;
    }

    double esp_fifo = 0, tur_fifo = 0;
    double esp_rr   = 0, tur_rr   = 0;

    if (strcmp(algoritmo, "rr") == 0) {
        ejecutar_rr(cola, cola_size, quantum);
        esp_rr = calcular_espera(cola, cola_size);
        tur_rr = calcular_turnaround(cola, cola_size);

        time_t cursor = llegadas_orig[0];
        Camion copias[NUM_CAMIONES];
        Camion *ptrs[NUM_CAMIONES];
        for (int i = 0; i < cola_size; i++) {
            copias[i]               = *cola[i];
            copias[i].tiempo_llegada = llegadas_orig[i];
            copias[i].tiempo_inicio  = cursor;
            cursor                  += bursts_orig[i];
            copias[i].tiempo_fin     = cursor;
            ptrs[i]                  = &copias[i];
        }
        esp_fifo = calcular_espera(ptrs, cola_size);
        tur_fifo = calcular_turnaround(ptrs, cola_size);

    } else {
        ejecutar_fifo(cola, cola_size);
        esp_fifo = calcular_espera(cola, cola_size);
        tur_fifo = calcular_turnaround(cola, cola_size);

        int q = (quantum > 0) ? quantum : 3;
        Camion copias[NUM_CAMIONES];
        Camion *ptrs[NUM_CAMIONES];
        int restantes[NUM_CAMIONES];
        for (int i = 0; i < cola_size; i++) {
            copias[i]               = *cola[i];
            copias[i].tiempo_llegada = llegadas_orig[i];
            copias[i].tiempo_inicio  = 0;
            copias[i].tiempo_fin     = 0;
            restantes[i]             = bursts_orig[i];
            ptrs[i]                  = &copias[i];
        }
        time_t t = llegadas_orig[0];
        int pendientes = cola_size;
        while (pendientes > 0) {
            pendientes = 0;
            for (int i = 0; i < cola_size; i++) {
                if (restantes[i] <= 0) continue;
                pendientes++;
                if (copias[i].tiempo_inicio == 0) copias[i].tiempo_inicio = t;
                int r = (restantes[i] < q) ? restantes[i] : q;
                t += r;
                restantes[i] -= r;
                if (restantes[i] == 0) copias[i].tiempo_fin = t;
            }
        }
        esp_rr = calcular_espera(ptrs, cola_size);
        tur_rr = calcular_turnaround(ptrs, cola_size);
    }

    imprimir_comparativa(esp_fifo, tur_fifo, esp_rr, tur_rr);
}