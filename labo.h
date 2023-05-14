#ifndef LABO_H
#define LABO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


/**
 *
 *PRE : zombies: Pointeur vers un tableau de PIDs (pid_t) représentant les processus zombie,
 *numZombies: Nombre de zombies dans le tableau (int).
 *
 *POST : Envoie un signal SIGTERM à chaque processus zombie dans le tableau.
 *
 */
void arreter_zombie(pid_t *zombies, int numZombies);

/**
 * PRÉ:  sig - Le signal reçu par la méthode
 * 
 * POST: Tous les processus zombies sont arrêtés et le contrôleur est tué avec SIGINT si son PID est supérieur à 0
 * 
 * RES:  La méthode termine le programme avec un code de retour 0
 */
void handle_controleD(int sig);

#endif /* LABO_H */
