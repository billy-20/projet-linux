#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "messages.h"
#include "utils_v2.h"
#include "labo.h"

pid_t zombies[NUM_ZOMBIES];
pid_t controller_pid;


void arreter_zombie(pid_t *zombies, int numZombies)
{
    int i;
    // on parcours les zombies et on envoi un SIGTERM a chacun d'entre eux
    for (i = 0; i < numZombies; i++)
    {
        skill(zombies[i], SIGTERM);
    }

    // Attendre la fin de chaque zombie
    for (i = 0; i < numZombies; i++)
    {
        swaitpid(zombies[i], NULL, 0);
    }
}


void handle_controleD(int sig)
{
    arreter_zombie(zombies, NUM_ZOMBIES);
    if (controller_pid > 0)
    {
        skill(controller_pid, SIGINT);
    }

    exit(0);
}

int main()
{
    char buffer[BUF_SIZE];
    controller_pid = -1;

    // Creation des processus zombies
    for (int i = 0; i < NUM_ZOMBIES; i++)
    {
        pid_t pid = sfork();
        if (pid == -1)
        {
            printf("Erreur dans le fork pour le zombie\n");
            arreter_zombie(zombies, i);
            return 1;
        }
        else if (pid == 0)
        {
            // ZOMBIE 

            execl("./zombie", "zombie", NULL);
            printf("Erreur lors de l'exécution du zombie\n");
            exit(1);
        }
        else
        {

            // CONTROLLER

            zombies[i] = pid;
        }
    }

    signal(SIGINT, handle_controleD);

    // l'id du controller
    controller_pid = getpid();

    while (1)
    {
        // CTRL-D
        if (fgets(buffer, BUF_SIZE, stdin) == NULL || feof(stdin))
        {
            printf("CTRL-D, fin du labo\n");
            handle_controleD(SIGINT);
            break;
        }
    }

    // on attend la fin de chaque zombie
    for (int i = 0; i < NUM_ZOMBIES; i++)
    {
        int status;
        swaitpid(zombies[i], &status, 0);
    }

    return 0;
}