#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "messages.h"

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

int main(int argc, char ** argv){
	menu();
	return 0;

}
