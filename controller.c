#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "utils_v2.h"
#include "messages.h"

/**
 * PRE  : /
 * POST : Affiche et g�re le menu.
 * RES  : /
 */
void menu() {

	printf("\t\t====  menu  ====\n");
	printf("\t(1) chercher les zombies\n");
	printf("\t(2) envoyer un message a un zombie\n");
	printf("\t(3) envoyer un message a tous les zombies\n");
	printf("\t(4) tuer un zombie\n");
	printf("\t\t================\n");
	write(1, "\n> ", 3*sizeof(char));

	int choix;
	scanf("%d", &choix);

	printf("\n\tvotre choix : %d\n", choix);

}
/**
 * PRE  : zombies     : a table of all zombies to send the command to.
 *	  zombieCount : the count of zombies in the table.
 * POST : The user will be asked for a command, which will be sent to all the zombies given in the arguments.
 * RES  : nothing.
 */
void sendCommands(int * zombies, int zombieCount){
	char * command = malloc(BUFF_SZ * sizeof(char));
	write(1, "\n> ", 3 * sizeof(char));
	read(0, command, BUFF_SZ);
	for(int i = 0;i < zombieCount;i++){
		write(zombies[i], command, strlen(command));
	}
	free(command);
}

/**
 * PRE  : zombies     : a table of all zombies to send the command to.
 *	  zombieCount : the count of zombies in the table.
 * POST : this command will listen to all zombies given in the arguments, and display their answers to the screen.
 * RES  : nothing.
 */
void receiveAnswers(int * zombies, int zombieCount){

}


/**
 * PRE  : ip    :  l'adresse ip.
 *        port  :  le port désiré.
 * POST : une nouvelle connection est créée. Celle-ci communique avec l'adresse ip fournie, sur le port fourni.
 * RES  : un fd d'un socket sur l'adresse et le port passé en paramètre ou -1 en cas d'erreur.
 */
int createConnection(char * ip, int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
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
   		printf("\n\tUsage : controller [ip address] * (1..n)\n\n");
		return 1;
	}
	
	char ** ips = (char **) malloc((argc - 1) * sizeof(char *));

	for(int i = 1;i < argc;i++){
		ips[i - 1] = (char *) malloc(strlen(argv[i]) * sizeof(char));
		strcpy(ips[i - 1], argv[i]);
	}


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

	// printf("port du zombie = %d ",connectedPort);


	/*

	Il faut 2 processus un de lecture et un d'ecriture, separer le write et read en 
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

	for(int i = 0;i < argc - 1;i++){
		free(ips[i]);
	}
	free(ips);

	return 0;
}
