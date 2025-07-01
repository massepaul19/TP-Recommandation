#ifndef RECO_KNN_H
#define RECO_KNN_H

#include "reco.h"
#include <math.h>
#include <stdlib.h>

// ========== STRUCTURES DE DONNÉES ==========

typedef struct {
    User users[MAX_USERS];
    Article articles[MAX_ARTICLES];
    Transaction transactions[MAX_USERS * MAX_ARTICLES];
    
    int nb_users;
    int nb_articles;
    int nb_transactions;
    
    float matrice_evaluations[MAX_USERS][MAX_ARTICLES];
    double matrice_similarite[MAX_USERS][MAX_USERS];
    
    double moyenne_utilisateur[MAX_USERS];
    int nb_eval_utilisateur[MAX_USERS];
    
    int k;
    int donnees_chargees;
    int matrice_similarite_calculee;
} RecommandeurKNN;

typedef struct {
    unsigned int id_user;
    unsigned int id_article;
    float note_predite;
    float note_reelle;
    double confiance;
} Prediction;

typedef struct {
    int index_utilisateur;
    unsigned int id_utilisateur;
    double similarite;
} Voisin;

extern RecommandeurKNN* recommandeur_global;

// ========== FONCTIONS PRINCIPALES (TÂCHES DU DEVOIR) ==========

double** Pearson(const char* filename);

float Predict(RecommandeurKNN* rec, unsigned int id_user, unsigned int id_article);

int Predict_all(RecommandeurKNN* rec, const char* test_filename, Prediction predictions[], int max_predictions);

// ========== GESTION DU RECOMMANDEUR ==========

/**
 * Création et initialisation du recommandeur
 */
RecommandeurKNN* creer_recommandeur(int k);
RecommandeurKNN* initialiser_recommandeur_depuis_fichier(const char* filename);
void liberer_recommandeur(RecommandeurKNN* rec);

/**
 * Chargement et construction des données
 */
int charger_donnees_depuis_fichier(RecommandeurKNN* rec, const char* filename);
int construire_matrice_evaluations(RecommandeurKNN* rec);

// ========== CALCULS DE SIMILARITÉ ==========

/**
 * Calcul des moyennes et corrélations
 */
void calculer_moyennes_utilisateurs(RecommandeurKNN* rec);
double correlation_pearson_entre_users(RecommandeurKNN* rec, int index_user1, int index_user2);
void calculer_matrice_similarite_complete(RecommandeurKNN* rec);

int verifier_donnees_recommandeur(RecommandeurKNN* rec);

// ========== ALGORITHME KNN ==========

/**
 * Recherche des voisins et calcul des prédictions
 */
Voisin* trouver_k_voisins_similaires(RecommandeurKNN* rec, unsigned int id_user);
double calculer_prediction_ponderee(RecommandeurKNN* rec, Voisin* voisins, unsigned int id_article, unsigned int id_user);
int utilisateur_a_note_article(RecommandeurKNN* rec, int index_user, int index_article);

// ========== FONCTIONS UTILITAIRES ==========

/**
 * Gestion des indices et accès aux données
 */
int trouver_index_utilisateur(RecommandeurKNN* rec, unsigned int id_user);
int trouver_index_article(RecommandeurKNN* rec, unsigned int id_article);
float obtenir_evaluation(RecommandeurKNN* rec, int index_user, int index_article);

/**
 * Gestion de la mémoire
 */
double** allouer_matrice_double(int lignes, int colonnes);
void liberer_matrice_similarite(double** matrice, int nb_users);

/**
 * Tri et comparaison des voisins
 */
int comparer_voisins_par_similarite(const void* a, const void* b);
void trier_voisins_par_similarite(Voisin* voisins, int nb_voisins);

// ========== ÉVALUATION DES PERFORMANCES ==========

/**
 * Métriques d'évaluation
 */
double calculer_rmse(Prediction predictions[], int nb_predictions);
double calculer_mae(Prediction predictions[], int nb_predictions);
double calculer_precision_seuil(Prediction predictions[], int nb_predictions, float seuil);
void afficher_metriques_evaluation(Prediction predictions[], int nb_predictions);

// ========== FONCTIONS D'AFFICHAGE ET DEBUG ==========

/**
 * Affichage des résultats et debug
 */
void afficher_matrice_similarite(double** matrice, RecommandeurKNN* rec);

void afficher_matrice_similarite_limitee(RecommandeurKNN* rec, int limite);
void afficher_voisins(Voisin* voisins, int k);
void afficher_predictions(Prediction predictions[], int nb_predictions);
void afficher_stats_recommandeur(RecommandeurKNN* rec);

/**
 * Sauvegarde des résultats
 */
void sauvegarder_predictions(const char* filename, Prediction predictions[], int nb_predictions);

/**
 * Fonctions de test
 */
void tester_predictions(RecommandeurKNN* rec);

// Fonction pour traiter les recommandations KNN via buffer
char* traiter_recommandation_knn(int id_user, int nb_reco);

int comparer_predictions(const void* a, const void* b);
// Fonction buffer pour KNN
void recommander_articles_knn_buffer(int id_user, int nb_reco, RecommandeurKNN* rec, char* buffer);

int utilisateur_a_note_article(RecommandeurKNN* rec, int index_user, int index_article);

#endif 
