#ifndef GRAPHE_H
#define GRAPHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "reco.h"

typedef struct {
    int **matrice_adjacence;
    int taille_totale;
    int nb_users;
    int nb_articles;
    int *map_users;
    int *map_articles;
    int *reverse_map_users;
    int *reverse_map_articles;
    int max_user_id;
    int max_article_id;
} GrapheBipartite;

typedef struct {
    float *pagerank_vector;
    int nb_iterations;
    float convergence;
} ResultatPageRank;

// Fonctions de lecture
Transaction* lire_fichier_train(const char* nom_fichier, int* nb_transactions);
Transaction* lire_fichier_test(const char* nom_fichier, int* nb_transactions);

// Analyse et mapping
void prescan_donnees(Transaction *transactions, int nb_trans, int* max_user, int* max_article, int* nb_users_uniques, int* nb_articles_uniques);

void creer_mappings_optimise(Transaction *transactions, int nb_trans, GrapheBipartite *graphe);

// Graphe et matrice
void construire_matrice_adjacence_optimise(Transaction *transactions, int nb_trans, GrapheBipartite *graphe);

// PageRank
ResultatPageRank* pagerank_optimise(GrapheBipartite *graphe, float alpha, float tolerance, int max_iterations);

// Sauvegarde des résultats
void sauvegarder_pagerank(GrapheBipartite *graphe, ResultatPageRank *pagerank, const char* nom_fichier);

//test recommandation

void recommander_articles(int id_user, GrapheBipartite *graphe, ResultatPageRank *pagerank, Transaction *transactions, int nb_transactions);

#endif 

