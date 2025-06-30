#ifndef FONCTIONS_CLIENT_H
#define FONCTIONS_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 8192
#define MAX_COMMAND_LENGTH 1024

// Structure pour gérer la connexion client
typedef struct {
    int socket;
    struct sockaddr_in server_addr;
    char server_ip[16];
    int server_port;
} client_connection_t;

// Prototypes des fonctions de connexion
int connecter_serveur(client_connection_t* client, const char* ip, int port);
void fermer_connexion(client_connection_t* client);
int tester_connexion(const char* ip, int port);

// Prototypes des fonctions de communication
int envoyer_commande(client_connection_t* client, const char* commande);
int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size);

// Prototypes des fonctions d'interface utilisateur
void afficher_menu();
void menu_numerique(client_connection_t* client);

#endif
