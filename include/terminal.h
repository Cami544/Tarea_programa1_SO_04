#ifndef TERMINAL_H
#define TERMINAL_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* ── Constantes globales  */
#define NUM_CAMIONES     8
#define NUM_MUELLES      3
#define BURST_MIN        1
#define BURST_MAX        5
#define PRIORIDAD_ALTA   1
#define PRIORIDAD_NORMAL 2

/* ── Estados del hilo     */
typedef enum {
    ESTADO_NUEVO      = 0,
    ESTADO_LISTO      = 1,
    ESTADO_EJECUCION  = 2,
    ESTADO_BLOQUEADO  = 3,
    ESTADO_TERMINADO  = 4
} EstadoHilo;


typedef struct {
    int         id;
    int         prioridad;
    int         burst_cpu;
    int         burst_restante;  
    EstadoHilo  estado;
    time_t      tiempo_llegada;
    time_t      tiempo_inicio;
    time_t      tiempo_fin;
    int         es_perecedero;
} Camion;

/* ── Variables globales compartidas — definidas en main ── */
extern sem_t           semaforo_muelles;
extern pthread_mutex_t mutex_log;
extern int             camiones_completados;

void  inicializar_camion(Camion *c, int id);
void  reportar_estado(Camion *c, EstadoHilo nuevo_estado);
void *rutina_camion(void *arg);
void  imprimir_tabla_camiones(Camion camiones[], int n);
void log_operacion(const char *mensaje);
void encolar_camion(Camion *c);
void iniciar_planificador(const char *algoritmo, int quantum);

#endif