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
#include "controller.h"

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

void envoyer_commandes(int *socks, int num_zombies, uid_t user_id)
{
  char buffer[BUF_SIZE];
  char uid_message[BUF_SIZE];

  // c'est ici qu'on cree le message ID a envoyer aux zombies
  snprintf(uid_message, BUF_SIZE, "UID %d\n", user_id);
  for (int i = 0; i < num_zombies; i++)
  {
    int sock = socks[i];

    if (sock >= 0)
    {
      //Envoi du message d'ID sur le socket
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
      skill(send_pid, SIGINT);
      break;
    }

    // on envoi la commande a tout les zombies
    for (int i = 0; i < num_zombies; i++)
    {
      int sock = socks[i];

      if (sock >= 0)
      {
          // Envoyer la commande sur le socket
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


void recevoir_commandes(int *socks, int num_zombies)
{
  struct pollfd *fds = malloc(num_zombies* sizeof(struct pollfd));

  // initialisation pollfd avec les sockets des zombies
  for (int i = 0; i < num_zombies; i++)
  {
    fds[i].fd = socks[i];
    fds[i].events = POLLIN;
  }

  char buffer[BUF_SIZE];

  while (!controleC)
  {
    int res = spoll(fds, num_zombies, 0);

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

          // Lire le message reçu sur le socket
          ssize_t message_recu = sread(sock, buffer, BUF_SIZE);

          if (message_recu < 0)
          {
            printf("Erreur lors de la réception du message\n");
            break;
          }

          buffer[message_recu] = '\0';

          // Verification du message recu pour fermer le zombie
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

            // on arrete l'envoi des commandes avec SIGINT
            skill(send_pid, SIGINT);
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
  // on libere l'espace alloue 
  free(fds);
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Adresse IP(s) manquante(s) ou UID manquant\n");
    return 1;
  }

  // l'id de l'utilisateur premier argument
  uid_t user_id = atoi(argv[1]);

  // adresse IP 2eme argument
  char **ips = &argv[2];
  int num_zombies = argc - 2;

  // alouer de l'espace pour les sockets des zombies
  int *socks = malloc(num_zombies* sizeof(int));

  for (int i = 0; i < num_zombies; i++)
  {
    socks[i] = -1;
  }

  int nbrConnexion = 0;
  int connectedPort = -1;

  // Connexion aux zombies
  for (int i = 0; i < num_zombies; i++)
  {
    for (int port = MIN_PORT; port <= MAX_PORT; port++)
    {
      int sock = createConnection(ips[i], port);

      if (sock < 0)
      {
        continue;
      }

      // on verifie si la connexion attendu est correcte
      struct sockaddr_in addr;
      socklen_t addr_len = sizeof(addr);
      getpeername(sock, (struct sockaddr *) &addr, &addr_len);
      connectedPort = ntohs(addr.sin_port);

      if (connectedPort != port)
      {
        close(sock);
        continue;
      }

      // on ajoute le socket a la liste de connexions
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


  pid_t send_pid = sfork();

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
    swaitpid(send_pid, &sender_status, 0);

    if (WIFEXITED(sender_status))
    {
      printf("erreur dans recevoir_commandes\n");
    }
    else
    {
      printf("Erreur du processus\n");
    }

    //ferme les sockets
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