#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "terminal.h"

static const char *NOMBRE_ESTADO[] = {
    "NUEVO", "LISTO", "EJECUCION", "BLOQUEADO", "TERMINADO"
};

void reportar_estado(Camion *c, EstadoHilo nuevo_estado) {
    c->estado = nuevo_estado;
    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);

    printf("[%02d:%02d:%02d] Camion #%02d | Prioridad: %-7s | Estado: %-10s",
           t->tm_hour, t->tm_min, t->tm_sec, c->id,
           (c->prioridad == PRIORIDAD_ALTA) ? "ALTA" : "NORMAL",
           NOMBRE_ESTADO[nuevo_estado]);

    if (nuevo_estado == ESTADO_NUEVO) {
        printf(" | Burst: %ds", c->burst_cpu);
    } else if (nuevo_estado == ESTADO_EJECUCION) {
        printf(" | Muelle asignado - cargando...");
    } else if (nuevo_estado == ESTADO_TERMINADO) {
        int espera  = (int)(c->tiempo_inicio - c->tiempo_llegada);
        int retorno = (int)(c->tiempo_fin    - c->tiempo_llegada);
        printf(" | Espera: %ds | Retorno: %ds", espera, retorno);
    }
    printf("\n");
    fflush(stdout);
}

void *rutina_camion(void *arg) {
    Camion *c = (Camion *)arg;

    c->tiempo_llegada = time(NULL);
    pthread_mutex_lock(&mutex_log);
    reportar_estado(c, ESTADO_NUEVO);
    pthread_mutex_unlock(&mutex_log);

    encolar_camion(c);

    pthread_mutex_lock(&mutex_log);
    reportar_estado(c, ESTADO_LISTO);
    pthread_mutex_unlock(&mutex_log);

    pthread_mutex_lock(&c->mutex_turno);
    while (!c->terminado) {
        pthread_cond_wait(&c->cond_turno, &c->mutex_turno);
    }
    pthread_mutex_unlock(&c->mutex_turno);

    pthread_mutex_lock(&mutex_log);
    reportar_estado(c, ESTADO_TERMINADO);
    pthread_mutex_unlock(&mutex_log);

    pthread_exit(NULL);
    return NULL;
}

void inicializar_camion(Camion *c, int id) {
    c->id             = id;
    c->prioridad      = (id % 3 == 0) ? PRIORIDAD_ALTA : PRIORIDAD_NORMAL;
    c->burst_cpu      = (rand() % (BURST_MAX - BURST_MIN + 1)) + BURST_MIN;
    c->burst_restante = c->burst_cpu;
    c->estado         = ESTADO_NUEVO;
    c->es_perecedero  = (c->prioridad == PRIORIDAD_ALTA) ? 1 : 0;
    c->tiempo_llegada = 0;
    c->tiempo_inicio  = 0;
    c->tiempo_fin     = 0;
    c->turno          = 0;
    c->terminado      = 0;
    pthread_mutex_init(&c->mutex_turno, NULL);
    pthread_cond_init(&c->cond_turno, NULL);
}

void imprimir_tabla_camiones(Camion camiones[], int n) {
    printf("\n========================================================\n");
    printf("         TERMINAL DE CARGA - CAMIONES REGISTRADOS\n");
    printf("========================================================\n");
    printf("  ID  | Prioridad | Burst CPU | Perecedero\n");
    printf("------+-----------+----------+-----------\n");
    for (int i = 0; i < n; i++) {
        printf("  %02d  | %-9s | %3d s     | %s\n",
               camiones[i].id,
               (camiones[i].prioridad == PRIORIDAD_ALTA) ? "ALTA" : "NORMAL",
               camiones[i].burst_cpu,
               camiones[i].es_perecedero ? "Si" : "No");
    }
    printf("========================================================\n\n");
    fflush(stdout);
}