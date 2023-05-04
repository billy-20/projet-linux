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
    if (argc < 3) {
        printf("faut mettre l'ip et le port ");
        return 1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int sock = createConnection(ip, port);
    if (sock < 0) {
        printf("Impossible de créer une connexion\n");
        return 1;
    }
    printf("Connexion établie avec %s:%d\n", ip, port);
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

        printf(" reponse =    %s",buffer);
    }
    close(sock);
    return 0;
}
