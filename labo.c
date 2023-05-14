#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "messages.h"
#include "utils_v2.h"

#define NUM_ZOMBIES 2

pid_t zombies[NUM_ZOMBIES];
pid_t controller_pid;

#include "messages.h"
#include "utils_v2.h"


/**
 *
 * PRE : zombies: Pointeur vers un tableau de PIDs (pid_t) représentant les processus zombie,
 *  numZombies: Nombre de zombies dans le tableau (int).
 *
 * POST : Envoie un signal SIGTERM à chaque processus zombie dans le tableau.
 * 
 */
void arreter_zombie(pid_t *zombies, int numZombies) {
    int i;
    for (i = 0; i < numZombies; i++) {
        kill(zombies[i], SIGTERM);
    }

    for (i = 0; i < numZombies; i++) {
        waitpid(zombies[i], NULL, 0);
    }
}


/*
* Gerer le CTRL-D dans le labo
*/
void handle_controleD(int sig) {
    arreter_zombie(zombies, NUM_ZOMBIES);
    if (controller_pid > 0) {
        kill(controller_pid, SIGINT);
    }

    exit(0);
}

int main() {
    char buffer[BUF_SIZE];
    controller_pid = -1;

    for (int i = 0; i < NUM_ZOMBIES; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            printf("Erreur dans le fork pour le zombie\n");
            arreter_zombie(zombies, i);
            return 1;
        } else if (pid == 0) {
            execl("./zombie", "zombie", NULL);
            printf("Erreur lors de l'exécution du zombie\n");
            exit(1);
        } else {
            zombies[i] = pid;
        }
    }

    signal(SIGINT, handle_controleD);

    controller_pid = getpid();

    while (1) {
        if (fgets(buffer, BUF_SIZE, stdin) == NULL || feof(stdin)) {
            printf("CTRL-D, fin du labo\n");
            handle_controleD(SIGINT);
            break;
        }
    }

    for (int i = 0; i < NUM_ZOMBIES; i++) {
        int status;
        waitpid(zombies[i], &status, 0);
    }

    return 0;
}
