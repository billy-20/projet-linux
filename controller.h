#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


//void handle_controleD(int sig);

/**
 *PRE : ip, j'adresse ip.port, le port désiré.
 *POST : une nouvelle connection est créée. Celle-ci communique avec l'adresse ip fournie, sur le port fourni.
 *RES : un fd d'un socket sur l'adresse et le port passé en paramètre ou -1 en cas d'erreur.
 */
int createConnection(char *ip, int port);


/**
 *PRE : socks: Pointeur vers un tableau d'entiers représentant les sockets des zombies connectés, num_zombies: Nombre de zombies dans le tableau,
 *user_id: ID utilisateur (uid_t) à envoyer aux zombies.
 *
 *POST : Envoie l'ID utilisateur aux zombies connectés. Attend les commandes de l'utilisateur et les envoie aux zombies connectés.
 */
void envoyer_commandes(int *socks, int num_zombies, uid_t user_id);

/**
 *PRE : socks: Pointeur vers un tableau d'entiers représentant les sockets des zombies connectés,num_zombies: Nombre de zombies dans le tableau (int).
 *
 *POST : Ecoute les messages provenant des zombies connectés, Affiche les messages reçus des zombies
 *
 */
void recevoir_commandes(int *socks, int num_zombies);

#endif /* CONTROLLER_H */
