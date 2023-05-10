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

#define BUF_SIZE 1024

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

void envoyer_commandes(int *socks, int num_zombies) {
    char buffer[BUF_SIZE];

    while (1) {
        printf("Entrez votre commande à envoyer aux zombies:\n");
        if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
            
            printf("Arrêt du programme\n");
            for (int i = 0; i < num_zombies; i++) {
                if (socks[i] >= 0) {
                    close(socks[i]);
                }
            }
            free(socks);
            exit(0);
        }

        for (int i = 0; i < num_zombies; i++) {
            int sock = socks[i];

            if (sock >= 0) {
                ssize_t message_envoye = write(sock, buffer, strlen(buffer));
                if (message_envoye < 0) {
                    printf("Erreur lors de l'envoi de la commande\n");
                    break;
                }
            }
        }
    }
}

void recevoir_commandes(int *socks, int num_zombies) {
    struct pollfd *fds = malloc(num_zombies * sizeof(struct pollfd));

    for (int i = 0; i < num_zombies; i++) {
        fds[i].fd = socks[i];
        fds[i].events = POLLIN;
    }

    char buffer[BUF_SIZE];

    while (1) {
        int res = poll(fds, num_zombies, 0);

        if (res < 0) {
            printf("Erreur lors de la réception des réponses\n");
            break;
        } else if (res > 0) {
            for (int i = 0; i < num_zombies; i++) {
                if (fds[i].revents & POLLIN) {
                    int sock = fds[i].fd;

                    ssize_t message_recu = read(sock, buffer, BUF_SIZE);
                    if (message_recu < 0) {
                        printf("Erreur lors de la réception du message\n");
                        break;
                    }

                    buffer[message_recu] = '\0';
                    printf("Réponse du zombie %d:\n%s\n", i+1, buffer);
                }
            }
        }
    }

    free(fds);
}



int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Adresse IP(s) manquante(s)\n");
        return 1;
    }

    char **ips = &argv[1];
    int num_zombies = argc - 1;

    int *socks = malloc(num_zombies * sizeof(int));



    for (int i = 0; i < num_zombies; i++) {
        socks[i] = -1;
    }

    
    int nbrConnexion = 0;
    int connectedPort = -1;

    for (int i = 0; i < num_zombies; i++) {
        for (int port = MIN_PORT; port <= MAX_PORT; port++) {
            int sock = createConnection(ips[i], port);

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
            
            socks[nbrConnexion] = sock;
            nbrConnexion++;

            if (nbrConnexion == 2 ) {
                     break;
                 }
        }
        
        if (nbrConnexion == 2 ) {
            break;
        }
    }
    
    
    pid_t send_pid = fork();

    if (send_pid == 0) {
        // FILS
        envoyer_commandes(socks, nbrConnexion);
        exit(0);
    } else if (send_pid > 0) {
        // PARENT
        recevoir_commandes(socks, nbrConnexion);
        
        int sender_status;
        waitpid(send_pid, &sender_status, 0);

        if (WIFEXITED(sender_status)) {
            printf("Erreur dans l'envoi ou la réception des commandes\n");
        } else {
            printf("Erreur du processus\n");
        }

        
        for (int i = 0; i < nbrConnexion; i++) {
            close(socks[i]);
        }

        free(socks);
        return 0;
    } else {
        perror("fork");
        return 1;
    }
}

            
