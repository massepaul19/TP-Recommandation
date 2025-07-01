#ifndef FONCTIONS_CLIENT_H
#define FONCTIONS_CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096

typedef struct {
    int socket_fd;
    struct sockaddr_in server_addr;
} client_connection_t;

// Fonctions de connexion
int tester_connexion(const char* ip, int port);
int connecter_serveur(client_connection_t* client, const char* ip, int port);
int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size);
int envoyer_requete(client_connection_t* client, const char* message);
void fermer_connexion(client_connection_t* client);

// Interface utilisateur
void afficher_menu(void);
void menu_numerique(client_connection_t* client);

#endif // FONCTIONS_CLIENT_H
