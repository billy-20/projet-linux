#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



/*
1. int getPort(int minPort, int maxPort) : renvoie un port ouvert dans la plage définie par minPort et maxPort, ou –1 en cas d’erreur. 

2. int createConnection(int port) : créé un socket qui écoute sur le port donné en paramètre. 

3. int toArgTable(char * commande, char *** table) : découpe une commande selon les espaces, les morceaux sont stockés dans le tableau “table” alloué dynamiquement pendant la fonction. La fonction renvoie le nombre de morceaux stockés dans le tableau. 

4. int main(int argc, char* argv[]) : Initialise le serveur et attend les connexions entrantes. 
*/

#include "utils_v2.h"
#include "messages.h"



/**
     * PRE  : la plage de port acceptés.
     * POST : cherche un port ouvert dans la plage fournie. 
     * RES  : le numéro d'un port ouvert ou -1 si aucun port n'est disponible ou en cas d'erreur.
*/
int getPort(int minPort , int maxPort){
   
    return 0;
}

/**
     * PRE  : port, le port désiré.
     * POST : une nouvelle connection est créée. Celle-ci accepte toutes les adresses et écoute sur le port fourni en paramètre.
     * RES  : un fd d'un socket qui écoute sur le port passé en paramètre ou -1 en cas d'erreur.
*/
int createConnection(int port){
  
  int socketfd;
	struct sockaddr_in addr;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if(socketfd < 0){
		perror("Erreurs avec la creation du socket");
		return -1;
	}

	memset(&addr,0,sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_port = htons(port);
  	addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	

  	if(bind(socketfd, (struct sockaddr *) &addr, sizeof(addr))){
  		perror("Erreurs association du socket");
  		return -1;
  	}

  	if(listen(socketfd, BACKLOG) < 0){
  		perror("Erreurs d'ecoute");
  		return -1;
  	}

	return socketfd;  
}

int toArgTable(char * commande, char *** table){
  return 0;
} 



int main(int argc, char* argv[]) {
  return 0;
}
