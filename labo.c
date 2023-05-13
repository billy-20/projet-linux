#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define NUM_ZOMBIES 2
#include "messages.h"
#include "utils_v2.h"

int client_sock;

void handle_sigquit(int sig)
{
    char *shutdown_message = "LABO_SHUTDOWN\n";
    write(client_sock, shutdown_message, strlen(shutdown_message));
    close(client_sock);
    exit(0);
}

void arreter_zombie(pid_t *zombies, int numZombies)
{
    int i;
    for (i = 0; i < numZombies; i++)
    {
        kill(zombies[i], SIGTERM);
    }

    for (i = 0; i < numZombies; i++)
    {
        waitpid(zombies[i], NULL, 0);
    }
}

int main()
{
    struct sigaction sa_quit;

    sa_quit.sa_handler = handle_sigquit;
    sigemptyset(&sa_quit.sa_mask);
    sigaction(SIGQUIT, &sa_quit, NULL);

    pid_t zombies[NUM_ZOMBIES];

    int ports_dispo[] = { 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034 };

    for (int i = 0; i < NUM_ZOMBIES; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            printf("error fils");
            arreter_zombie(zombies, i);
            return 1;
        }
        else if (pid == 0)
        {
            char portArg[10];
            sprintf(portArg, "%d", ports_dispo[i]);
            execl("./zombie", "zombie", NULL);

            printf("Erreur dans le zombie");
            exit(1);
        }
        else
        {
            zombies[i] = pid;
        }
    }

    for (int i = 0; i < NUM_ZOMBIES; i++)
    {
        int status;
        waitpid(zombies[i], &status, 0);
    }

    return 0;
}