#include <stdio.h>
#include <stdlib.h>
#include "reco.h"

//------------ Users -----------------------------

// Lecture des users
int lire_User(FILE *f, User* u) {
    return fscanf(f, "%u;%d\n", &u->id_user, &u->nb_fois);
}

// Compter les users
int compter_user(char *name_file, User users[], int max_users) {
    FILE* f = fopen(name_file, "r");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", name_file);
        return -1;
    }

    int count = 0;
    while (count < max_users && lire_User(f, &users[count]) == 2) {
        count++;
    }
    fclose(f);
    return count;
}

// Écrire les users dans un fichier
void ecrire_users(const char* filename, User users[], int nb_users) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", filename);
        return;
    }
    for (int i = 0; i < nb_users; i++) {
        fprintf(f, "%u;%d\n", users[i].id_user, users[i].nb_fois);
    }
    fclose(f);
}


//#########################################################

//------------ Articles -----------------------------

// Lecture des articles
int lire_Article(FILE *f, Article* a) {
    return fscanf(f, "%u;%u;%d\n", &a->id_article, &a->id_cat, &a->nb_fois);
}

// Compter les articles
int compter_articles(char *name_file, Article articles[], int max_articles) {
    FILE* f = fopen(name_file, "r");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", name_file);
        return -1;
    }

    int count = 0;
    while (count < max_articles && lire_Article(f, &articles[count]) == 3) {
        count++;
    }
    fclose(f);
    return count;
}

// Écrire les articles dans un fichier

void ecrire_articles(const char* filename, Article articles[], int nb_articles) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", filename);
        return;
    }
    for (int i = 0; i < nb_articles; i++) {
        fprintf(f, "%u;%u;%d\n", articles[i].id_article, articles[i].id_cat, articles[i].nb_fois);
    }
    fclose(f);
}


//#########################################################

//------------ Categories -----------------------------

// Lecture des catégories
int lire_Categorie(FILE *f, Categorie* c) {
    return fscanf(f, "%u;%d\n", &c->id_cat, &c->nb_fois);
}

// Compter les catégories
int compter_categories(char *name_file, Categorie categories[], int max_categories) {
    FILE* f = fopen(name_file, "r");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", name_file);
        return -1;
    }

    int count = 0;
    while (count < max_categories && lire_Categorie(f, &categories[count]) == 2) {
        count++;
    }
    fclose(f);
    return count;
}

// Écrire les catégories dans un fichier

void ecrire_categories(const char* filename, Categorie categories[], int nb_categories) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", filename);
        return;
    }
    for (int i = 0; i < nb_categories; i++) {
        fprintf(f, "%u;%d\n", categories[i].id_cat, categories[i].nb_fois);
    }
    fclose(f);
}


//#########################################################

//------------ Transactions ----------------------------- 

//Lecture des data dans transaction

int Lire_Transaction(FILE *file, Transaction *t) {

    return fscanf(file, "%d %d %d %f %lf", &t->id_user, &t->id_article, &t->id_cat, &t->evaluation, &t->timestamp);
    
}

//Ecrire Transaction: aide pour fonctions

void Ecrire_Transaction(FILE *file, Transaction t) {

    fprintf(file, "%d %d %d %.2f %.0lf\n", t.id_user, t.id_article, t.id_cat, t.evaluation, t.timestamp);
    
}

// Écrire les transactions dans un fichier

void ecrire_transactions(const char* filename, Transaction transactions[], int nb_transactions) {

    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", filename);
        return;
    }
    for (int i = 0; i < nb_transactions; i++) {
        fprintf(f, "%d %d %d %.1f %.0f\n", transactions[i].id_user, transactions[i].id_article, transactions[i].id_cat, transactions[i].evaluation, transactions[i].timestamp);
    }
    fclose(f);
}

//------- 

// Ajouter une transaction (optionnelle pour des équilibrer avec les autres structures

void ajouter_transaction(User users[], int* nb_users, Article articles[], int* nb_articles, Categorie categories[], int* nb_categories, Transaction transactions[], int* nb_transactions, Transaction nouvelle_transaction) {
    // Mettre à jour ou ajouter l'utilisateur
    User* user = trouver_utilisateur(users, *nb_users, nouvelle_transaction.id_user);
    if (user) {
        user->nb_fois++;
    } else {
        if (*nb_users < MAX_USERS) {
            users[*nb_users] = (User){nouvelle_transaction.id_user, 1};
            (*nb_users)++;
        }
    }

    // Mettre à jour ou ajouter l'article
    Article* article = trouver_article(articles, *nb_articles, nouvelle_transaction.id_article);
    if (article) {
        article->nb_fois++;
    } else {
        if (*nb_articles < MAX_ARTICLES) {
            articles[*nb_articles] = (Article){nouvelle_transaction.id_article, nouvelle_transaction.id_cat, 1};
            (*nb_articles)++;
        }
    }

    // Mettre à jour ou ajouter la catégorie
    Categorie* categorie = trouver_categorie(categories, *nb_categories, nouvelle_transaction.id_cat);
    if (categorie) {
        categorie->nb_fois++;
    } else {
        if (*nb_categories < MAX_CATEGORIES) {
            categories[*nb_categories] = (Categorie){nouvelle_transaction.id_cat, 1};
            (*nb_categories)++;
        }
    }

    // Ajouter la transaction
    if (*nb_transactions < MAX_ARTICLES) {
        transactions[*nb_transactions] = nouvelle_transaction;
        (*nb_transactions)++;
    }
}

// Compter les transactions

int compter_transactions(const char* filename, Transaction transactions[], int max_transactions) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Erreur d'ouverture du fichier %s.\n", filename);
        return -1;
    }

    int count = 0;
    Transaction t;
    while (count < max_transactions && fscanf(f, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        transactions[count] = t;
        count++;
    }
    fclose(f);
    return count;
}

// Fonction pour recharger toutes les données depuis donnees.txt

void charger_donnees(User users[], int* nb_users, Article articles[], int* nb_articles, 
                     Categorie categories[], int* nb_categories, Transaction transactions[], int* nb_transactions) {
    
    
    *nb_users = 0;
    *nb_articles = 0;
    *nb_categories = 0;
    *nb_transactions = 0;
    
    // Réinitialiser les tableaux
    memset(users, 0, sizeof(User) * MAX_USERS);
    memset(articles, 0, sizeof(Article) * MAX_ARTICLES);
    memset(categories, 0, sizeof(Categorie) * MAX_CATEGORIES);
    
    FILE* file = fopen("data/donnees.txt", "r");
    if (file != NULL) {
        Transaction t;
        while (fscanf(file, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
            ajouter_transaction(users, nb_users, articles, nb_articles,
                                categories, nb_categories, transactions, nb_transactions, t);
        }
        fclose(file);
        printf("Données chargées/rechargées depuis donnees.txt\n");
        printf("Users: %d, Articles: %d, Categories: %d, Transactions: %d\n", 
               *nb_users, *nb_articles, *nb_categories, *nb_transactions);
    } else {
        printf("Erreur : impossible d'ouvrir le fichier donnees.txt\n");
    }
}

//#########################################################

//------------ Fonctions d'aide -----------------------------

// Trouver un utilisateur par ID
User* trouver_utilisateur(User users[], int nb_users, unsigned int id_user) {
    for (int i = 0; i < nb_users; i++) {
        if (users[i].id_user == id_user) {
            return &users[i];
        }
    }
    return NULL;
}

// Trouver un article par ID
Article* trouver_article(Article articles[], int nb_articles, unsigned int id_article) {
    for (int i = 0; i < nb_articles; i++) {
        if (articles[i].id_article == id_article) {
            return &articles[i];
        }
    }
    return NULL;
}

// Trouver une catégorie par ID
Categorie* trouver_categorie(Categorie categories[], int nb_categories, unsigned int id_cat) {
    for (int i = 0; i < nb_categories; i++) {
        if (categories[i].id_cat == id_cat) {
            return &categories[i];
        }
    }
    return NULL;
}

//#########################################################

//------------ Fonctions de devoir -----------------------------

// Afficher les statistiques

void afficher_stats(const char* fichier) {

    User users[MAX_USERS];
    Article articles[MAX_ARTICLES];
    Categorie categories[MAX_CATEGORIES];

    int nb_users = compter_user("data/Users.txt", users, MAX_USERS);
    int nb_articles = compter_articles("data/Articles.txt", articles, MAX_ARTICLES);
    int nb_categories = compter_categories("data/Categories.txt", categories, MAX_CATEGORIES);

    FILE* file = fopen(fichier, "r");
    if (file == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier);
        return;
    }

    int categorie_count[MAX_CATEGORIES] = {0};
    double min_timestamp = DBL_MAX;
    double max_timestamp = 0;
    int nb_transactions = 0;

    Transaction t;
    
    while (fscanf(file, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        // Mettre à jour le nombre de fois pour la catégorie
        if (t.id_cat >= 0 && t.id_cat < MAX_CATEGORIES) {
            categorie_count[t.id_cat]++;
        }

        // Mettre à jour les timestamps min et max
        if (t.timestamp < min_timestamp) min_timestamp = t.timestamp;
        if (t.timestamp > max_timestamp) max_timestamp = t.timestamp;

        nb_transactions++;
    }
    fclose(file);

    // Conversion des timestamps en dates lisibles
    time_t min_time = (time_t)min_timestamp;
    time_t max_time = (time_t)max_timestamp;
    char min_date[100], max_date[100];
    strftime(min_date, sizeof(min_date), "%Y-%m-%d %H:%M:%S", localtime(&min_time));
    strftime(max_date, sizeof(max_date), "%Y-%m-%d %H:%M:%S", localtime(&max_time));

    printf("\n--------- Affichage -----------\n\n");
    printf("Nombre d'utilisateurs : %d\n", nb_users);
    printf("Nombre d'articles : %d\n", nb_articles);
    printf("Nombre de catégories : %d\n", nb_categories);
    printf("Plage temporelle : %s - %s\n\n", min_date, max_date);
    printf("Nombre d'éléments par catégorie :\n");

    int max_count = 0;
    int min_count = INT_MAX;
    int max_cat_id = -1;
    int min_cat_id = -1;

    for (int i = 0; i < MAX_CATEGORIES; i++) {
        int count = categorie_count[i];
        if (count > 0) {
            printf("Catégorie %d : %d items\n", i, count);
        }
        if (count > max_count) {
            max_count = count;
            max_cat_id = i;
        }
        if (count > 0 && count < min_count) {
            min_count = count;
            min_cat_id = i;
        }
    }

    printf("\nCatégorie la plus fréquente : %d (%d articles)\n", max_cat_id, max_count);
    printf("Catégorie la moins fréquente : %d (%d articles)\n", min_cat_id, min_count);
}

//#############
//----------------------------------------------

//Extraction des données par période

void extraire_transactions_par_periode(const char* fichier_entree, const char* fichier_sortie, time_t t1, time_t t2) {

    FILE* file_entree = fopen(fichier_entree, "r");
    if (file_entree == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_entree);
        return;
    }

    FILE* file_sortie = fopen(fichier_sortie, "w");
    if (file_sortie == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_sortie);
        fclose(file_entree);
        return;
    }

    Transaction t;
    
    while (fscanf(file_entree, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
    
        time_t transaction_time = (time_t)t.timestamp;
        
        if (transaction_time >= t1 && transaction_time <= t2) {
            fprintf(file_sortie, "%d %d %d %.1f %.0f\n", t.id_user, t.id_article, t.id_cat, t.evaluation, t.timestamp);
        }
    }

    fclose(file_entree);
    fclose(file_sortie);
}


//#############

//filtrage par frequence min d'apparition

//ici il suffit juste de copier ceux qui sont sup ou egal au min

//Filtrage par frequence min d'apparition

//Filtrer par utilisateurs uniquement

int filtrer_transactions_par_min_U(const char* fichier_entree, const char* fichier_sortie, int minU){

    FILE *f_users = fopen("data/Users.txt", "r");
    FILE *f_in = fopen(fichier_entree, "r");
    FILE *f_out = fopen(fichier_sortie, "w");

    if (!f_users || !f_in || !f_out) {
        printf("Erreur d'ouverture de fichier.\n");
        if (f_users) fclose(f_users);
        if (f_in) fclose(f_in);
        if (f_out) fclose(f_out);
        return -1; 
    }

    // Lecture des utilisateurs dans une liste
    User *users = malloc(sizeof(User) * 100);
    int nb_users = 0;
    int capacity_users = 100;
    
    while (lire_User(f_users, &users[nb_users]) == 2) {
        nb_users++;
        if (nb_users >= capacity_users) {
            capacity_users += 100;
            users = realloc(users, sizeof(User) * capacity_users);
        }
    }
    fclose(f_users);

    // Filtrage des transactions
    Transaction t;
    int nb_transactions_filtrees = 0;

    while (fscanf(f_in, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        int user_freq_ok = 0;
        
        // Vérifier la fréquence de l'utilisateur
        for (int i = 0; i < nb_users; i++) {
            if (users[i].id_user == (unsigned int)t.id_user && users[i].nb_fois >= minU) {
                user_freq_ok = 1;
                break;
            }
        }

        if (user_freq_ok) {
            fprintf(f_out, "%d %d %d %.2f %.0lf\n", t.id_user, t.id_article, t.id_cat, t.evaluation, t.timestamp);
            nb_transactions_filtrees++;
        }
    }

    free(users);
    fclose(f_in);
    fclose(f_out);

    return nb_transactions_filtrees; 
}

//--------------------------------------------------
//Filtrer par articles uniquement

int filtrer_transactions_par_min_I(const char* fichier_entree, const char* fichier_sortie, int minI){
   
    FILE *f_articles = fopen("data/Articles.txt", "r");
    FILE *f_in = fopen(fichier_entree, "r");
    FILE *f_out = fopen(fichier_sortie, "w");

    if (!f_articles || !f_in || !f_out) {
        printf("Erreur d'ouverture de fichier.\n");
        if (f_articles) fclose(f_articles);
        if (f_in) fclose(f_in);
        if (f_out) fclose(f_out);
        return -1;
    }

    // Lecture des articles dans une liste
    Article *articles = malloc(sizeof(Article) * 100);
    int nb_articles = 0;
    int capacity_articles = 100;
    
    while (lire_Article(f_articles, &articles[nb_articles]) == 3) {
        nb_articles++;
        if (nb_articles >= capacity_articles) {
            capacity_articles += 100;
            articles = realloc(articles, sizeof(Article) * capacity_articles);
        }
    }
    fclose(f_articles);

    // Filtrage des transactions
    Transaction t;
    int nb_transactions_filtrees = 0;

    while (fscanf(f_in, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        int article_freq_ok = 0;
        
        // Vérifier la fréquence de l'article
        for (int i = 0; i < nb_articles; i++) {
            if (articles[i].id_article == (unsigned int)t.id_article && articles[i].nb_fois >= minI) {
                article_freq_ok = 1;
                break;
            }
        }

        if (article_freq_ok) {
            fprintf(f_out, "%d %d %d %.2f %.0lf\n", t.id_user, t.id_article, t.id_cat, t.evaluation, t.timestamp);
            nb_transactions_filtrees++;
        }
    }

    free(articles);
    fclose(f_in);
    fclose(f_out);

    return nb_transactions_filtrees; 
}


//--------------------------------------------------

//Filtrer par utilisateurs ET articles

int filtrer_transactions_par_min_U_et_I(const char* fichier_entree, const char* fichier_sortie, int minU, int minI){
   
    FILE *f_users = fopen("data/Users.txt", "r");
    FILE *f_articles = fopen("data/Articles.txt", "r");
    FILE *f_in = fopen(fichier_entree, "r");
    FILE *f_out = fopen(fichier_sortie, "w");

    if (!f_users || !f_articles || !f_in || !f_out) {
        printf("Erreur d'ouverture de fichier.\n");
        if (f_users) fclose(f_users);
        if (f_articles) fclose(f_articles);
        if (f_in) fclose(f_in);
        if (f_out) fclose(f_out);
        return -1;
    }

    // Lecture des utilisateurs dans une liste
    User *users = malloc(sizeof(User) * 100);
    int nb_users = 0;
    int capacity_users = 100;
    
    while (lire_User(f_users, &users[nb_users]) == 2) {
        nb_users++;
        if (nb_users >= capacity_users) {
            capacity_users += 100;
            users = realloc(users, sizeof(User) * capacity_users);
        }
    }
    fclose(f_users);

    // Lecture des articles dans une liste
    Article *articles = malloc(sizeof(Article) * 100);
    int nb_articles = 0;
    int capacity_articles = 100;
    
    while (lire_Article(f_articles, &articles[nb_articles]) == 3) {
        nb_articles++;
        if (nb_articles >= capacity_articles) {
            capacity_articles += 100;
            articles = realloc(articles, sizeof(Article) * capacity_articles);
        }
    }
    fclose(f_articles);

    // Filtrage des transactions
    Transaction t;
    int nb_transactions_filtrees = 0;

    while (fscanf(f_in, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        int user_freq_ok = 0, article_freq_ok = 0;
        
        // Vérifier la fréquence de l'utilisateur
        for (int i = 0; i < nb_users; i++) {
            if (users[i].id_user == (unsigned int)t.id_user && users[i].nb_fois >= minU) {
                user_freq_ok = 1;
                break;
            }
        }

        // Vérifier la fréquence de l'article
        for (int i = 0; i < nb_articles; i++) {
            if (articles[i].id_article == (unsigned int)t.id_article && articles[i].nb_fois >= minI) {
                article_freq_ok = 1;
                break;
            }
        }

        if (user_freq_ok && article_freq_ok) {
            fprintf(f_out, "%d %d %d %.2f %.0lf\n", t.id_user, t.id_article, t.id_cat, t.evaluation, t.timestamp);
            nb_transactions_filtrees++;
        }
    }

    free(users);
    free(articles);
    fclose(f_in);
    fclose(f_out);

    return nb_transactions_filtrees; 
}

//--------------------------------------------------
//#############
//-------------------------------------------------

//Nettoyer les données
//Ici l'idée est de creer train contenant une partie de datas et test.txt contenant une partie de Train et de fausses datas crées. Le resultat est obtenu en faisant l'intersection entre Train et Test 

int nettoyer_fichier_test(const char* fichier_train, const char* fichier_test, const char* fichier_clean) {

    FILE *f_train = fopen(fichier_train, "r");
    FILE *f_test = fopen(fichier_test, "r");
    FILE *f_clean = fopen(fichier_clean, "w");
    

    if (!f_train || !f_test || !f_clean) {
        printf("Erreur d'ouverture des fichiers\n");
        return -1;
    }

    // Allocation dynamique ligne par ligne pour les transactions du train
    
    int nb_train = 0;
    Transaction *train_data = NULL;
    Transaction temp;

    while (Lire_Transaction(f_train, &temp) == 5) {
        // Allocation mémoire pour ajouter une nouvelle transaction
        Transaction *new_data = malloc((nb_train + 1) * sizeof(Transaction));
        if (!new_data) {
            printf("Erreur d'allocation mémoire\n");
            return -1;
        }

        // Copier les anciennes transactions
        for (int i = 0; i < nb_train; i++) {
            new_data[i] = train_data[i];
        }

        // Ajouter la nouvelle transaction à la fin
        new_data[nb_train] = temp;
        nb_train++;

        // Libérer l'ancienne mémoire
        free(train_data);
        train_data = new_data;
    }

    // Nettoyer les transactions dans le fichier test
    int nb_clean = 0;
    
    while (Lire_Transaction(f_test, &temp) == 5) {
        int user_ok = 0, article_ok = 0;

        // Comparer les transactions test avec celles du train
        for (int i = 0; i < nb_train; i++) {
            if (train_data[i].id_user == temp.id_user) user_ok = 1;
            if (train_data[i].id_article == temp.id_article) article_ok = 1;
            if (user_ok && article_ok) break;
        }

        // Si la transaction dans le test est valide, on l'écrit dans le fichier clean
        if (user_ok && article_ok) {
            Ecrire_Transaction(f_clean , temp);
            nb_clean++;
        }
    }

    // Libération de la mémoire allouée pour les données du train
    free(train_data);

    // Fermeture des fichiers
    fclose(f_train);
    fclose(f_test);
    fclose(f_clean);

    return nb_clean;
}

//--------------------------------------------------
//#############
//------------- Autres Utilitaires -----------------

void extraire_vers_fichier(const char *fichier_entree, const char *fichier_sortie, int ligne_debut, int ligne_fin) {
   
    FILE *fin = fopen(fichier_entree, "r");
    FILE *fout = fopen(fichier_sortie, "w");

    if (!fin || !fout) {
        perror("Erreur lors de l'ouverture des fichiers");
        exit(EXIT_FAILURE);
    }

    Transaction t;
    int ligne = 0;

    while (fscanf(fin, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        ligne++;
        if (ligne >= ligne_debut && ligne <= ligne_fin) {
            fprintf(fout, "%d %d %.1f\n", t.id_user, t.id_article, t.evaluation);
        }
        if (ligne > ligne_fin) break;
    }

    fclose(fin);
    fclose(fout);
    printf("Extraction terminée : lignes %d à %d enregistrées dans Train.txt\n", ligne_debut, ligne_fin);
}

//Paul-Basthylle MASSE MASSE
//################################################
