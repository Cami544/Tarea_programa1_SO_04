#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "terminal.h"

int main(void) {
    pid_t pid1, pid2;

     pid1 = fork();

    if (pid1 < 0) {
        perror("fork (hijo pares)");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        /* Soy el hijo 1 */
        printf("[Hijo PARES   | PID %d] Iniciando hilos de números pares...\n",
               getpid());
        ejecutar_hijo_pares();
        printf("[Hijo PARES   | PID %d] Terminado.\n", getpid());
        exit(EXIT_SUCCESS);
    }

    pid2 = fork();

    if (pid2 < 0) {
        perror("fork (hijo impares)");
        kill(pid1, SIGKILL);
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        printf("[Hijo IMPARES | PID %d] Iniciando hilos de números impares...\n",
               getpid());
        ejecutar_hijo_impares();
        printf("[Hijo IMPARES | PID %d] Terminado.\n", getpid());
        exit(EXIT_SUCCESS);
    }

    printf("[Padre        | PID %d] Esperando a los hijos %d y %d...\n",
           getpid(), pid1, pid2);

    int status;

    waitpid(pid1, &status, 0);
    printf("[Padre        | PID %d] Hijo pares   (%d) terminó con estado %d.\n",
           getpid(), pid1, WEXITSTATUS(status));

    waitpid(pid2, &status, 0);
    printf("[Padre        | PID %d] Hijo impares (%d) terminó con estado %d.\n",
           getpid(), pid2, WEXITSTATUS(status));

    printf("[Padre        | PID %d] Todos los procesos e hilos han finalizado.\n",
           getpid());

    return EXIT_SUCCESS;
}
