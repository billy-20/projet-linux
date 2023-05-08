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

// obtenir un port dispo
int getPort(int minPort, int maxPort) {
    int sock;
    int port = randomIntBetween(minPort , maxPort);
    struct sockaddr_in addr;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            close(sock);
            return port;
        }
        close(sock);
    
    return -1;
}

int createConnection(int port) {
    int sock;
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    if (listen(sock, 1) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

int toArgv(char * commande, char *** arguments) {
    return 0;
}

int main() {
    int port = getPort(MIN_PORT, MAX_PORT);
    if (port == -1) {
        printf("Impossible de trouver un port disponible\n");
        return 1;
    }
    int sock = createConnection(port);
    if (sock < 0) {
        printf("Impossible de créer une connexion\n");
        return 1;
    }
    printf("Le zombie tourne sur ce port :  %d...\n", port);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock < 0) {
        printf("Impossible d'accepter la connexion\n");
        return 1;
    }
    //printf("conexion zombie temp");
    close(sock);
    char buf[BUF_SIZE];
    int n;
    while (1) {
        memset(buf, 0, BUF_SIZE);
        n = recv(client_sock, buf, BUF_SIZE, 0);
        if (n <= 0) {
            break;
        }
           //char ** args = NULL;


    printf("Commande reçue par le controlleur : %s\n", buf);

        int pid = fork();
        if (pid == 0) {
            // FILS    
            dup2(client_sock, STDOUT_FILENO); 

            char *args[] = {"/bin/bash", "-c", buf, NULL};
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            // PARENT
            int status;
            swaitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("OK\n");
            } else {
                printf("KO\n");
            }
        } else {
            perror("fork");
            return 1;
        }

    }   
    

        

close(client_sock);

return 0;
}


           
