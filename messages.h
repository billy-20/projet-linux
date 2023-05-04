#ifndef _MESSAGES_H_
#define _MESSAGES_H_


#define MIN_PORT 40000
#define MAX_PORT 42069
#define TAILLE_COMMANDE 1024
#define TAILLE_BUFFER 1024



typedef enum {KILL, ID, EXEC} TypeCommande;
typedef struct {
	TypeCommande type;
	char commande[TAILLE_COMMANDE];
} Commande;

#endif
