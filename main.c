#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Structures de données
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

// Structure pour le graphe bipartite unifié
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

// Structure pour PageRank
typedef struct {
    float *pagerank_vector;
    int nb_iterations;
    float convergence;
} ResultatPageRank;

// Fonction pour lire les transactions depuis Train.txt
Transaction* lire_fichier_train(const char* nom_fichier, int* nb_transactions) {
    FILE* fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", nom_fichier);
        *nb_transactions = 0;
        return NULL;
    }
    
    // Première passe : compter le nombre de lignes
    int count = 0;
    char ligne[256];
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (strlen(ligne) > 5) { // Ignorer les lignes vides
            count++;
        }
    }
    
    printf("Nombre de transactions détectées: %d\n", count);
    
    // Allouer la mémoire
    Transaction* transactions = malloc(count * sizeof(Transaction));
    if (!transactions) {
        printf("Erreur: Allocation mémoire échouée pour %d transactions\n", count);
        fclose(fichier);
        *nb_transactions = 0;
        return NULL;
    }
    
    // Deuxième passe : lire les données
    rewind(fichier);
    int index = 0;
    
    while (fgets(ligne, sizeof(ligne), fichier) && index < count) {
        // Format: user_id article_id category_id rating timestamp
        // Exemple: 249 1087 21 4.3 969215645
        int user_id, article_id, cat_id;
        float rating;
        double timestamp;
        
        int nb_parsed = sscanf(ligne, "%d %d %d %f %lf", 
                              &user_id, &article_id, &cat_id, &rating, &timestamp);
        
        if (nb_parsed == 5) {
            transactions[index].id_user = user_id;
            transactions[index].id_article = article_id;
            transactions[index].id_cat = cat_id;
            transactions[index].evaluation = rating;
            transactions[index].timestamp = timestamp;
            index++;
        } else {
            printf("Ligne mal formatée ignorée: %s", ligne);
        }
    }
    
    fclose(fichier);
    *nb_transactions = index;
    
    printf("Lecture terminée: %d transactions chargées avec succès\n", index);
    
    // Afficher quelques statistiques
    if (index > 0) {
        int min_user = transactions[0].id_user, max_user = transactions[0].id_user;
        int min_article = transactions[0].id_article, max_article = transactions[0].id_article;
        float min_rating = transactions[0].evaluation, max_rating = transactions[0].evaluation;
        
        for (int i = 1; i < index; i++) {
            if (transactions[i].id_user < min_user) min_user = transactions[i].id_user;
            if (transactions[i].id_user > max_user) max_user = transactions[i].id_user;
            if (transactions[i].id_article < min_article) min_article = transactions[i].id_article;
            if (transactions[i].id_article > max_article) max_article = transactions[i].id_article;
            if (transactions[i].evaluation < min_rating) min_rating = transactions[i].evaluation;
            if (transactions[i].evaluation > max_rating) max_rating = transactions[i].evaluation;
        }
        
        printf("Statistiques:\n");
        printf("- Users: %d à %d\n", min_user, max_user);
        printf("- Articles: %d à %d\n", min_article, max_article);
        printf("- Ratings: %.1f à %.1f\n", min_rating, max_rating);
    }
    
    return transactions;
}

// Fonction pour lire le fichier Test.txt
Transaction* lire_fichier_test(const char* nom_fichier, int* nb_transactions) {
    // Même logique que lire_fichier_train
    return lire_fichier_train(nom_fichier, nb_transactions);
}

// Optimisation: pré-scan pour dimensionner les mappings
void prescan_donnees(Transaction *transactions, int nb_trans, int* max_user, int* max_article, 
                    int* nb_users_uniques, int* nb_articles_uniques) {
    *max_user = 0;
    *max_article = 0;
    
    // Trouver les IDs max
    for (int i = 0; i < nb_trans; i++) {
        if (transactions[i].id_user > *max_user) *max_user = transactions[i].id_user;
        if (transactions[i].id_article > *max_article) *max_article = transactions[i].id_article;
    }
    
    // Compter les IDs uniques avec des tableaux temporaires
    char* users_vus = calloc(*max_user + 1, sizeof(char));
    char* articles_vus = calloc(*max_article + 1, sizeof(char));
    
    *nb_users_uniques = 0;
    *nb_articles_uniques = 0;
    
    for (int i = 0; i < nb_trans; i++) {
        if (!users_vus[transactions[i].id_user]) {
            users_vus[transactions[i].id_user] = 1;
            (*nb_users_uniques)++;
        }
        if (!articles_vus[transactions[i].id_article]) {
            articles_vus[transactions[i].id_article] = 1;
            (*nb_articles_uniques)++;
        }
    }
    
    free(users_vus);
    free(articles_vus);
    
    printf("Pré-scan: %d users uniques, %d articles uniques\n", 
           *nb_users_uniques, *nb_articles_uniques);
}

// Fonction optimisée pour créer les mappings avec gros datasets
void creer_mappings_optimise(Transaction *transactions, int nb_trans, GrapheBipartite *graphe) {
    int max_user, max_article, nb_users_uniques, nb_articles_uniques;
    
    // Pré-scan pour optimiser l'allocation
    prescan_donnees(transactions, nb_trans, &max_user, &max_article, 
                   &nb_users_uniques, &nb_articles_uniques);
    
    graphe->max_user_id = max_user;
    graphe->max_article_id = max_article;
    graphe->nb_users = nb_users_uniques;
    graphe->nb_articles = nb_articles_uniques;
    graphe->taille_totale = nb_users_uniques + nb_articles_uniques;
    
    // Allouer les tableaux de mapping
    graphe->map_users = malloc((max_user + 1) * sizeof(int));
    graphe->map_articles = malloc((max_article + 1) * sizeof(int));
    graphe->reverse_map_users = malloc(nb_users_uniques * sizeof(int));
    graphe->reverse_map_articles = malloc(nb_articles_uniques * sizeof(int));
    
    // Initialiser à -1
    for (int i = 0; i <= max_user; i++) graphe->map_users[i] = -1;
    for (int i = 0; i <= max_article; i++) graphe->map_articles[i] = -1;
    
    // Créer les mappings en une seule passe
    int user_count = 0, article_count = 0;
    
    for (int i = 0; i < nb_trans; i++) {
        // Mapper les users
        if (graphe->map_users[transactions[i].id_user] == -1) {
            graphe->map_users[transactions[i].id_user] = user_count;
            graphe->reverse_map_users[user_count] = transactions[i].id_user;
            user_count++;
        }
        
        // Mapper les articles
        if (graphe->map_articles[transactions[i].id_article] == -1) {
            graphe->map_articles[transactions[i].id_article] = article_count;
            graphe->reverse_map_articles[article_count] = transactions[i].id_article;
            article_count++;
        }
    }
    
    printf("Mappings créés: %d users [0-%d], %d articles [%d-%d]\n", 
           user_count, user_count-1, article_count, user_count, graphe->taille_totale-1);
}

// Construction optimisée de la matrice d'adjacence pour gros datasets
void construire_matrice_adjacence_optimise(Transaction *transactions, int nb_trans, GrapheBipartite *graphe) {
    int taille = graphe->taille_totale;
    
    printf("Allocation matrice %dx%d (%d éléments)...\n", taille, taille, taille*taille);
    
    // Allouer la matrice unifiée
    graphe->matrice_adjacence = malloc(taille * sizeof(int*));
    if (!graphe->matrice_adjacence) {
        printf("Erreur: Allocation mémoire échouée pour la matrice\n");
        return;
    }
    
    for (int i = 0; i < taille; i++) {
        graphe->matrice_adjacence[i] = calloc(taille, sizeof(int));
        if (!graphe->matrice_adjacence[i]) {
            printf("Erreur: Allocation mémoire échouée ligne %d\n", i);
            return;
        }
    }
    
    // Optimisation: utiliser un tableau 2D temporaire pour éviter les doublons
    // Plus efficace qu'une hashmap pour ce cas
    char **relations_vues = malloc(graphe->nb_users * sizeof(char*));
    for (int i = 0; i < graphe->nb_users; i++) {
        relations_vues[i] = calloc(graphe->nb_articles, sizeof(char));
    }
    
    int doublons_elimines = 0;
    int relations_uniques = 0;
    
    // Traiter chaque transaction
    for (int i = 0; i < nb_trans; i++) {
        int user_idx = graphe->map_users[transactions[i].id_user];
        int article_local_idx = graphe->map_articles[transactions[i].id_article];
        int article_global_idx = graphe->nb_users + article_local_idx;
        
        if (user_idx != -1 && article_local_idx != -1) {
            // Vérifier si cette relation a déjà été vue
            if (!relations_vues[user_idx][article_local_idx]) {
                // Nouvelle relation : l'ajouter à la matrice
                graphe->matrice_adjacence[user_idx][article_global_idx] = 1;
                graphe->matrice_adjacence[article_global_idx][user_idx] = 1;
                relations_vues[user_idx][article_local_idx] = 1;
                relations_uniques++;
            } else {
                doublons_elimines++;
            }
        }
        
        // Afficher progression pour gros datasets
        if ((i + 1) % 500 == 0) {
            printf("Progression: %d/%d transactions traitées\r", i + 1, nb_trans);
            fflush(stdout);
        }
    }
    
    printf("\nMatrice construite: %d relations uniques, %d doublons éliminés\n", 
           relations_uniques, doublons_elimines);
    
    // Libérer la matrice temporaire
    for (int i = 0; i < graphe->nb_users; i++) {
        free(relations_vues[i]);
    }
    free(relations_vues);
    
    // Calculer la densité de la matrice
    float densite = (float)(relations_uniques * 2) / (taille * taille) * 100;
    printf("Densité de la matrice: %.4f%%\n", densite);
}

// PageRank optimisé pour gros datasets
ResultatPageRank* pagerank_optimise(GrapheBipartite *graphe, float alpha, float tolerance, int max_iterations) {
    ResultatPageRank *resultat = malloc(sizeof(ResultatPageRank));
    int n = graphe->taille_totale;
    
    printf("Démarrage PageRank sur %d noeuds...\n", n);
    
    // Allouer les vecteurs
    resultat->pagerank_vector = malloc(n * sizeof(float));
    float *new_pr = malloc(n * sizeof(float));
    
    // Initialisation uniforme
    float init_value = 1.0 / n;
    for (int i = 0; i < n; i++) {
        resultat->pagerank_vector[i] = init_value;
    }
    
    // Pré-calculer les degrés sortants
    int *degre_sortant = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            degre_sortant[i] += graphe->matrice_adjacence[i][j];
        }
    }
    
    // Algorithme PageRank avec affichage de progression
    for (int iter = 0; iter < max_iterations; iter++) {
        // Terme de téléportation
        float teleportation = (1 - alpha) / n;
        for (int i = 0; i < n; i++) {
            new_pr[i] = teleportation;
        }
        
        // Terme de marche aléatoire
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graphe->matrice_adjacence[j][i] > 0 && degre_sortant[j] > 0) {
                    new_pr[i] += alpha * resultat->pagerank_vector[j] / degre_sortant[j];
                }
            }
        }
        
        // Vérifier convergence
        float diff = 0.0;
        for (int i = 0; i < n; i++) {
            diff += fabs(new_pr[i] - resultat->pagerank_vector[i]);
        }
        
        // Copier les nouveaux valeurs
        memcpy(resultat->pagerank_vector, new_pr, n * sizeof(float));
        
        // Affichage progression
        if (iter % 5 == 0 || diff < tolerance) {
            printf("Iteration %d/%d, convergence: %.8f\r", iter + 1, max_iterations, diff);
            fflush(stdout);
        }
        
        if (diff < tolerance) {
            printf("\nConvergence atteinte à l'itération %d\n", iter + 1);
            resultat->nb_iterations = iter + 1;
            resultat->convergence = diff;
            break;
        }
    }
    
    free(new_pr);
    free(degre_sortant);
    
    return resultat;
}

// Fonction pour sauvegarder les résultats PageRank
void sauvegarder_pagerank(GrapheBipartite *graphe, ResultatPageRank *pagerank, const char* nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "w");
    if (!fichier) {
        printf("Erreur: Impossible de créer le fichier %s\n", nom_fichier);
        return;
    }
    
    fprintf(fichier, "# Résultats PageRank - %d itérations, convergence: %f\n", 
            pagerank->nb_iterations, pagerank->convergence);
    fprintf(fichier, "# Type ID Score_PageRank\n");
    
    // Sauvegarder les scores des users
    for (int i = 0; i < graphe->nb_users; i++) {
        fprintf(fichier, "USER %d %.8f\n", 
                graphe->reverse_map_users[i], pagerank->pagerank_vector[i]);
    }
    
    // Sauvegarder les scores des articles
    for (int i = 0; i < graphe->nb_articles; i++) {
        int global_idx = graphe->nb_users + i;
        fprintf(fichier, "ARTICLE %d %.8f\n", 
                graphe->reverse_map_articles[i], pagerank->pagerank_vector[global_idx]);
    }
    
    fclose(fichier);
    printf("Résultats PageRank sauvegardés dans %s\n", nom_fichier);
}

// Fonction principale pour gros datasets
int main() {
    printf("=== SYSTÈME DE RECOMMANDATION - GRAPHE BIPARTITE + PAGERANK ===\n");
    printf("Optimisé pour gros datasets (2000+ transactions)\n\n");
    
    // Étape 1: Lire les données depuis Train.txt
    printf("=== LECTURE DES DONNÉES ===\n");
    int nb_train = 0;
    Transaction* train_data = lire_fichier_train("Train.txt", &nb_train);
    
    if (!train_data || nb_train == 0) {
        printf("Aucune donnée d'entraînement trouvée. Arrêt du programme.\n");
        return 1;
    }
    
    // Étape 2: Construire le graphe bipartite
    printf("\n=== CONSTRUCTION DU GRAPHE BIPARTITE ===\n");
    GrapheBipartite graphe = {0};
    
    creer_mappings_optimise(train_data, nb_train, &graphe);
    construire_matrice_adjacence_optimise(train_data, nb_train, &graphe);
    
    // Étape 3: Appliquer PageRank
    printf("\n=== APPLICATION PAGERANK ===\n");
    ResultatPageRank *pagerank = pagerank_optimise(&graphe, 0.85, 1e-6, 50);
    
    // Étape 4: Sauvegarder les résultats
    printf("\n=== SAUVEGARDE DES RÉSULTATS ===\n");
    sauvegarder_pagerank(&graphe, pagerank, "pagerank_results.txt");
    
    // Étape 5: Test avec fichier Test.txt s'il existe
    printf("\n=== TEST DU MODÈLE ===\n");
    int nb_test = 0;
    Transaction* test_data = lire_fichier_test("Test.txt", &nb_test);
    
    if (test_data && nb_test > 0) {
        printf("Évaluation sur %d transactions de test...\n", nb_test);
        // Ici vous pouvez ajouter la fonction test_pagerank du code précédent
    } else {
        printf("Aucun fichier Test.txt trouvé ou vide.\n");
    }
    
    // Étape 6: Exemple de recommandations
    printf("\n=== EXEMPLES DE RECOMMANDATIONS ===\n");
    if (graphe.nb_users > 0) {
        int exemple_user = graphe.reverse_map_users[0];  // Premier user
        printf("Recommandations pour l'utilisateur %d:\n", exemple_user);
        // Ici vous pouvez ajouter pagerank_train_data du code précédent
    }
    
    // Libération mémoire
    printf("\n=== NETTOYAGE MÉMOIRE ===\n");
    for (int i = 0; i < graphe.taille_totale; i++) {
        free(graphe.matrice_adjacence[i]);
    }
    free(graphe.matrice_adjacence);
    free(graphe.map_users);
    free(graphe.map_articles);
    free(graphe.reverse_map_users);
    free(graphe.reverse_map_articles);
    free(pagerank->pagerank_vector);
    free(pagerank);
    free(train_data);
    if (test_data) free(test_data);
    
    printf("Programme terminé avec succès !\n");
    return 0;
}
