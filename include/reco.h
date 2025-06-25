#ifndef RECO_H
#define RECO_H

#include <string.h>
#include <time.h>
#include <limits.h>
#include <float.h>

#define MAX_USERS 50
#define MAX_ARTICLES 50
#define MAX_CATEGORIES 50

typedef struct {
    unsigned int id_user;
    int nb_fois;
} User;

typedef struct {
    unsigned int id_article;
    unsigned int id_cat;
    int nb_fois;
} Article;

typedef struct {
    unsigned int id_cat;
    int nb_fois;
} Categorie;

typedef struct {
    int id_user;
    int id_article;
    int id_cat;
    float evaluation;
    double timestamp;
} Transaction;

//------ Users ----------------

int lire_User(FILE *f, User* u);
int compter_user(char *name_file, User users[], int max_users);
void ecrire_users(const char* filename, User users[], int nb_users);

//------ Articles ----------------

int lire_Article(FILE *f, Article* a);
int compter_articles(char *name_file, Article articles[], int max_articles);
void ecrire_articles(const char* filename, Article articles[], int nb_articles);

//------ Categories ----------------

int lire_Categorie(FILE *f, Categorie* c);
int compter_categories(char *name_file, Categorie categories[], int max_categories);
void ecrire_categories(const char* filename, Categorie categories[], int nb_categories);

//------ Transactions ----------------

void ecrire_transactions(const char* filename, Transaction transactions[], int nb_transactions);

int Lire_Transaction(FILE *file, Transaction *t);

void ajouter_transaction(User users[], int* nb_users, Article articles[], int* nb_articles, Categorie categories[], int* nb_categories, Transaction transac[], int* nb_transactions, Transaction nouvelle_transaction);

int compter_transactions(const char* filename, Transaction transactions[], int max_transactions);

void charger_donnees(User users[], int* nb_users, Article articles[], int* nb_articles, Categorie categories[], int* nb_categories, Transaction transactions[], int* nb_transactions);


//------ Fonctions d'aide ----------------

User* trouver_utilisateur(User users[], int nb_users, unsigned int id_user);
Article* trouver_article(Article articles[], int nb_articles, unsigned int id_article);
Categorie* trouver_categorie(Categorie categories[], int nb_categories, unsigned int id_cat);

//------ Autres Utilitaires ----------------
//Extraction

void extraire_vers_fichier(const char *fichier_entree, const char *fichier_sortie, int ligne_debut, int ligne_fin);

//######################################################

//------ Fonctions de devoir ----------------


// Fonction pour afficher les statistiques

void afficher_stats(const char* fichier);

//--------------------------------------------------

// Fonction pour extraire les transactions dans une période spécifique

void extraire_transactions_par_periode(const char* fichier_entree, const char* fichier_sortie, time_t t1, time_t t2);

//--------------------------------------------------

//filtrage par frequence min d'apparition


//Filtrer par utilisateurs uniquement

int filtrer_transactions_par_min_U(const char* fichier_entree, const char* fichier_sortie, int minU);

//Filtrer par articles uniquement

int filtrer_transactions_par_min_I(const char* fichier_entree, const char* fichier_sortie, int minI);

//Filtrer par utilisateurs ET articles

int filtrer_transactions_par_min_U_et_I(const char* fichier_entree, const char* fichier_sortie, int minU, int minI);

//--------------------------------------------------
// Nettoyage des données

int nettoyer_fichier_test(const char* fichier_train, const char* fichier_test, const char* fichier_clean);

#endif

