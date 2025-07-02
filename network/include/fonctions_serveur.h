#ifndef FONCTIONS_SERVEUR_H
#define FONCTIONS_SERVEUR_H

#define BUFFER_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "reco_KNN.h"
#include "graphe.h"
#include "factorisation.h"

// Déclarations des variables globales (extern)
extern pthread_mutex_t mutex_recommandeur;

// Prototypes des fonctions
void envoyer_reponse(int client_sock, const char* message);
void process_client(int client_sock);
void handle_KNN(int client_sock, int id_user, int nb_reco);
//void handle_factorisation(int client_sock, int id_user, int nb_reco);
void handle_graphe(int client_sock, int id_user, int nb_reco);

#endif
