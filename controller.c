/**
 *El Haddadi Haddouchene Bilal
 *Bekkari Ibrahim
 *
 */
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

#include "messages.h"
#include "utils_v2.h"

pid_t send_pid;

int controleC = 0;


/*void handle_controleD(int sig){
    printf("Arrêt du programme controller lancé par le labo\n");
    if (send_pid > 0) {
        kill(send_pid, SIGINT);
    }

    exit(0);
}

*/

/**
 *PRE : ip, j'adresse ip.port, le port désiré.
 *POST : une nouvelle connection est créée. Celle-ci communique avec l'adresse ip fournie, sur le port fourni.
 *RES : un fd d'un socket sur l'adresse et le port passé en paramètre ou -1 en cas d'erreur.
 */

int createConnection(char *ip, int port)
{
  int sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("Erreur lors de la création du socket");
    return -1;
  }

  struct sockaddr_in addr = { 0 };

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  connect(sock, (struct sockaddr *) &addr, sizeof(addr));

  return sock;
}

/**
 *PRE : socks: Pointeur vers un tableau d'entiers représentant les sockets des zombies connectés, num_zombies: Nombre de zombies dans le tableau,
 *user_id: ID utilisateur (uid_t) à envoyer aux zombies.
 *
 *POST : Envoie l'ID utilisateur aux zombies connectés. Attend les commandes de l'utilisateur et les envoie aux zombies connectés.
 */

void envoyer_commandes(int *socks, int num_zombies, uid_t user_id)
{
  char buffer[BUF_SIZE];
  char uid_message[BUF_SIZE];

  snprintf(uid_message, BUF_SIZE, "UID %d\n", user_id);
  for (int i = 0; i < num_zombies; i++)
  {
    int sock = socks[i];

    if (sock >= 0)
    {
      ssize_t message_envoye = swrite(sock, uid_message, strlen(uid_message));
      if (message_envoye < 0)
      {
        printf("Erreur lors de l'envoi de l'ID utilisateur\n");
        break;
      }
    }
  }

  while (!controleC)
  {
    printf("Entrez votre commande à envoyer aus zombie(s):\n");
    if (fgets(buffer, BUF_SIZE, stdin) == NULL)
    {
      printf("Arrêt du programme\n");
      controleC = 1;
      kill(send_pid, SIGINT);
      break;
    }

    for (int i = 0; i < num_zombies; i++)
    {
      int sock = socks[i];

      if (sock >= 0)
      {
        ssize_t message_envoye = swrite(sock, buffer, strlen(buffer));
        if (message_envoye < 0)
        {
          printf("Erreur lors de l'envoi de la commande\n");
          break;
        }
      }
    }
  }
}

/**
 *PRE : socks: Pointeur vers un tableau d'entiers représentant les sockets des zombies connectés,num_zombies: Nombre de zombies dans le tableau (int).
 *
 *POST : Ecoute les messages provenant des zombies connectés, Affiche les messages reçus des zombies
 *
 */

void recevoir_commandes(int *socks, int num_zombies)
{
  struct pollfd *fds = malloc(num_zombies* sizeof(struct pollfd));

  for (int i = 0; i < num_zombies; i++)
  {
    fds[i].fd = socks[i];
    fds[i].events = POLLIN;
  }

  char buffer[BUF_SIZE];

  while (!controleC)
  {
    int res = poll(fds, num_zombies, 0);

    if (res < 0)
    {
      printf("Erreur lors de la réception des réponses\n");
      break;
    }
    else if (res > 0)
    {
      for (int i = 0; i < num_zombies; i++)
      {
        if (fds[i].revents &POLLIN)
        {
          int sock = fds[i].fd;

          ssize_t message_recu = sread(sock, buffer, BUF_SIZE);

          if (message_recu < 0)
          {
            printf("Erreur lors de la réception du message\n");
            break;
          }

          buffer[message_recu] = '\0';

          if (strcmp(buffer, "FERMER_ZOMBIE\n") == 0)
          {
            if (num_zombies >= 2)
            {
              printf("Les %d zombies se sont arrêté. Arrêt du programme.\n", num_zombies);
            }
            else
            {
              printf("Le zombies s'est arrêté. Arrêt du programme.\n");
            }

            kill(send_pid, SIGINT);
            break;
          }
          else
          {
            printf("Réponse du zombie %d:\n%s\n", i + 1, buffer);
          }
        }
      }
    }
  }

  free(fds);
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Adresse IP(s) manquante(s) ou UID manquant\n");
    return 1;
  }

  uid_t user_id = atoi(argv[1]);
  char **ips = &argv[2];
  int num_zombies = argc - 2;

  int *socks = malloc(num_zombies* sizeof(int));

  for (int i = 0; i < num_zombies; i++)
  {
    socks[i] = -1;
  }

  int nbrConnexion = 0;
  int connectedPort = -1;
  for (int i = 0; i < num_zombies; i++)
  {
    for (int port = MIN_PORT; port <= MAX_PORT; port++)
    {
      int sock = createConnection(ips[i], port);

      if (sock < 0)
      {
        continue;
      }

      struct sockaddr_in addr;
      socklen_t addr_len = sizeof(addr);
      getpeername(sock, (struct sockaddr *) &addr, &addr_len);
      connectedPort = ntohs(addr.sin_port);

      if (connectedPort != port)
      {
        close(sock);
        continue;
      }

      socks[nbrConnexion] = sock;
      nbrConnexion++;
      printf("connecting to  %s:%d OK \n", ips[i], port);

      if (nbrConnexion == 2)
      {
        break;
      }
    }

    if (nbrConnexion == 2)
    {
      break;
    }
  }

  /*le CTRL-D du labo (ne marche pas)
  signal(SIGINT, handle_controleD);
  signal(SIGTSTP, handle_controleD);
  */

  printf("nombre de conexions : %d \n", nbrConnexion);
  printf("send uid %d to zombie \n", user_id);
  pid_t send_pid = fork();

  if (send_pid == 0)
  {
    // FILS
    // Après la connexion avec les zombies

    envoyer_commandes(socks, nbrConnexion, user_id);
    exit(0);
  }
  else if (send_pid > 0)
  {
    // PARENT
    recevoir_commandes(socks, nbrConnexion);
    int sender_status;
    waitpid(send_pid, &sender_status, 0);

    if (WIFEXITED(sender_status))
    {
      printf("erreur dans recevoir_commandes\n");
    }
    else
    {
      printf("Erreur du processus\n");
    }

    for (int i = 0; i < nbrConnexion; i++)
    {
      close(socks[i]);
    }

    free(socks);
    return 0;
  }
  else
  {
    perror("fork");
    return 1;
  }
}