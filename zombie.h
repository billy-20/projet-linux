#ifndef ZOMBIE_H
#define ZOMBIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>


/** 
 * 
 *PRE :sig: Le signal reçu, doit être SIGINT (interruption de programme),
 *La variable globale "client_sock" doit être initialisée et correspondre à un socket client valide.
 *
 *POST : Envoie le message "FERMER_ZOMBIE" au client connecté.ferme la connexion avec le client.
 */
void sigint(int sig);

/**
 *PRE  : la plage de port acceptés.
 *POST : cherche un port ouvert dans la plage fournie. 
 *RES  : le numéro d'un port ouvert ou -1 si aucun port n'est disponible ou en cas d'erreur.
 */
int getPort(int minPort, int maxPort);

/**
 *PRE  : port, le port désiré.
 *POST : une nouvelle connection est créée. Celle-ci accepte toutes les adresses et écoute sur le port fourni en paramètre.
 *RES  : un fd d'un socket qui écoute sur le port passé en paramètre ou -1 en cas d'erreur.
 */

int createConnection(int port);



#endif /* ZOMBIE_H */
