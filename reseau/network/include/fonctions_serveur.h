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

#include "reco_KNN.h"

// Déclarations des variables globales (extern)

extern double** matrice_similarite;
extern Prediction predictions[10000];
extern int nb_predictions;

// Prototypes des fonctions

void envoyer_reponse(int client_sock, const char* message);
void handle_pearson(int client_sock, char* fichier_train);
void handle_predict(int client_sock, unsigned int user, unsigned int item);
void handle_predict_all(int client_sock, char* fichier_test);
void handle_evaluate(int client_sock);
void handle_save(int client_sock);
void handle_stats(int client_sock);
void process_client(int client_sock);

#endif
