#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_ZOMBIES 2

#include "messages.h"
#include "utils_v2.h"

void arreter_zombie(pid_t * zombies, int numZombies) {
   	int i;
	for (i = 0; i < numZombies; i++) {
		kill(zombies[i], SIGTERM);
	}

	for (i = 0; i < numZombies; i++) {
		waitpid(zombies[i], NULL, 0);
	}
}

int main() {
	pid_t zombies[NUM_ZOMBIES];


	int i;
	for (i = 0; i < NUM_ZOMBIES; i++) {
		pid_t pid = fork();
		if (pid == -1) {
			printf("error fils");
			arreter_zombie(zombies, i);
			printf("Erreur lors de l'exécution du zombie");
			return 1;
		} else if (pid == 0) {
			execl("./zombie", PRGM_NAME, NULL);
			exit(1);
		} else {
			zombies[i] = pid;
		}
	}

	return 0;
}
