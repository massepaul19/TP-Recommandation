#ifndef FACTORISATION_H
#define FACTORISATION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "reco.h"

// Structure de factorisation
typedef struct {
    double **U;         // Matrice des utilisateurs (M x K)
    double **V;         // Matrice des items (N x K)
    double *bias_u;     // Biais utilisateurs
    double *bias_v;     // Biais items
    double bias_global; // Biais global
    int M;              // Nombre d'utilisateurs
    int N;              // Nombre d'articles
    int K;              // Nombre de facteurs latents
} MatriceFactorisation;

// Matrice complète
typedef struct {
    double **matrice;  // Matrice des notes M x N
    int M;             // Nombre d'utilisateurs
    int N;             // Nombre d'articles
} MatriceComplete;

//extern MatriceComplete* matrice_complete;

// Fonctions principales
MatriceFactorisation* init_matrice_factorisation(int M, int N, int K);
void liberer_matrice_factorisation(MatriceFactorisation* mf);

MatriceComplete* MF(Transaction* train_data, int nb_train, int M, int N, int K,
                    double alpha, double lambda, int nb_iterations);

double* Predict_all_MF(MatriceComplete* full_matrix, Transaction* test_data, int nb_test);

double predire_note(MatriceFactorisation* mf, int user, int item);
void entrainer_modele(MatriceFactorisation* mf, Transaction* data, int nb_data,
                      double alpha, double lambda, int nb_iterations);

double calculer_erreur_rmse(MatriceFactorisation* mf, Transaction* data, int nb_data);

// Matrice complète
MatriceComplete* creer_matrice_complete(int M, int N);
void liberer_matrice_complete(MatriceComplete* mc);
void afficher_matrice_complete(MatriceComplete* mc);

// Données
Transaction* lire_fichier_train(const char* filename, int* nb_train);
Transaction* creer_donnees_test(int* nb_test);
void liberer_transactions(Transaction* data);

// Utils
double aleatoire_normal(double mean, double std);
void melanger_transactions(Transaction* data, int nb_data);

// Fonctions pour le menu
char* traiter_recommandation_matricielle(int id_user, int nb_reco);
void diviser_donnees(Transaction* transactions, int nb_total, 
                    Transaction** train_data, int* nb_train,
                    Transaction** test_data, int* nb_test,
                    double ratio_train);
void evaluer_predictions(double* predictions, Transaction* test_data, int nb_test);

void demander_id_et_recommandations(MatriceComplete* , int* id_user, int* nb_reco);

char* traiter_recommandation_factorisation(int id_user, int nb_reco);
// Nettoyage
void nettoyer_memoire_matricielle(void);

//char* traiter_recommandation_factorisation(int id_user, int nb_reco);

#endif
