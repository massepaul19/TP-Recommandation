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

// Structure pour le graphe bipartite unifié (Users + Items dans une seule matrice)
typedef struct {
    int **matrice_adjacence;  // Matrice binaire (0 ou 1)
    int taille_totale;        // nb_users + nb_articles
    int nb_users;
    int nb_articles;
    int *map_users;           // mapping id_user -> index dans [0, nb_users-1]
    int *map_articles;        // mapping id_article -> index dans [nb_users, nb_users+nb_articles-1]
    int max_user_id;
    int max_article_id;
    int *reverse_map_users;   // index -> id_user original
    int *reverse_map_articles; // index -> id_article original
} GrapheBipartite;

// Structure pour PageRank
typedef struct {
    float *pagerank_vector;   // Vecteur PR unifié
    int nb_iterations;
    float convergence;
} ResultatPageRank;

// Fonction pour créer les mappings selon le modèle du cours
void creer_mappings(Transaction *transactions, int nb_trans, GrapheBipartite *graphe) {
    int users_count = 0, articles_count = 0;
    int max_user = 0, max_article = 0;
    
    // Trouver les IDs max
    for (int i = 0; i < nb_trans; i++) {
        if (transactions[i].id_user > max_user) max_user = transactions[i].id_user;
        if (transactions[i].id_article > max_article) max_article = transactions[i].id_article;
    }
    
    graphe->max_user_id = max_user;
    graphe->max_article_id = max_article;
    
    // Allouer les tableaux de mapping
    graphe->map_users = malloc((max_user + 1) * sizeof(int));
    graphe->map_articles = malloc((max_article + 1) * sizeof(int));
    
    // Initialiser à -1 (non mappé)
    for (int i = 0; i <= max_user; i++) graphe->map_users[i] = -1;
    for (int i = 0; i <= max_article; i++) graphe->map_articles[i] = -1;
    
    // Créer les mappings : Users dans [0, nb_users-1], Articles dans [nb_users, nb_users+nb_articles-1]
    for (int i = 0; i < nb_trans; i++) {
        if (graphe->map_users[transactions[i].id_user] == -1) {
            graphe->map_users[transactions[i].id_user] = users_count++;
        }
        if (graphe->map_articles[transactions[i].id_article] == -1) {
            graphe->map_articles[transactions[i].id_article] = articles_count;
            articles_count++;
        }
    }
    
    graphe->nb_users = users_count;
    graphe->nb_articles = articles_count;
    graphe->taille_totale = users_count + articles_count;
    
    // Créer les reverse mappings pour les recommandations
    graphe->reverse_map_users = malloc(users_count * sizeof(int));
    graphe->reverse_map_articles = malloc(articles_count * sizeof(int));
    
    for (int id = 0; id <= max_user; id++) {
        if (graphe->map_users[id] != -1) {
            graphe->reverse_map_users[graphe->map_users[id]] = id;
        }
    }
    
    for (int id = 0; id <= max_article; id++) {
        if (graphe->map_articles[id] != -1) {
            graphe->reverse_map_articles[graphe->map_articles[id]] = id;
        }
    }
    
    printf("Mappings créés: %d users [0-%d], %d articles [%d-%d]\n", 
           users_count, users_count-1, articles_count, users_count, graphe->taille_totale-1);
}

// Construction de la matrice d'adjacence unifiée selon le modèle du cours
void construire_matrice_adjacence(Transaction *transactions, int nb_trans, GrapheBipartite *graphe) {
    int taille = graphe->taille_totale;
    
    // Allouer la matrice carrée unifiée
    graphe->matrice_adjacence = malloc(taille * sizeof(int*));
    for (int i = 0; i < taille; i++) {
        graphe->matrice_adjacence[i] = calloc(taille, sizeof(int));
    }
    
    // Remplir la matrice avec gestion des doublons
    int **compteurs = malloc(graphe->nb_users * sizeof(int*));
    for (int i = 0; i < graphe->nb_users; i++) {
        compteurs[i] = calloc(graphe->nb_articles, sizeof(int));
    }
    
    // Traiter chaque transaction
    for (int i = 0; i < nb_trans; i++) {
        int user_idx = graphe->map_users[transactions[i].id_user];
        int article_local_idx = graphe->map_articles[transactions[i].id_article];
        int article_global_idx = graphe->nb_users + article_local_idx; // Décalage pour la matrice unifiée
        
        if (user_idx != -1 && article_local_idx != -1) {
            // Gestion des doublons : on marque simplement la relation (matrice binaire)
            if (!compteurs[user_idx][article_local_idx]) {
                // Relation bidirectionnelle dans la matrice unifiée
                graphe->matrice_adjacence[user_idx][article_global_idx] = 1;  // User -> Article
                graphe->matrice_adjacence[article_global_idx][user_idx] = 1;  // Article -> User
                compteurs[user_idx][article_local_idx] = 1;
            }
        }
    }
    
    // Libérer les compteurs
    for (int i = 0; i < graphe->nb_users; i++) {
        free(compteurs[i]);
    }
    free(compteurs);
    
    // Afficher un aperçu de la matrice (comme dans le tableau 2)
    printf("\nMatrice d'adjacence construite (%dx%d):\n", taille, taille);
    printf("Exemple des premières lignes:\n");
    printf("     ");
    for (int j = 0; j < taille && j < 10; j++) {
        if (j < graphe->nb_users) printf("U%-2d ", graphe->reverse_map_users[j]);
        else printf("I%-2d ", graphe->reverse_map_articles[j - graphe->nb_users]);
    }
    printf("\n");
    
    for (int i = 0; i < taille && i < 6; i++) {
        if (i < graphe->nb_users) printf("U%-2d: ", graphe->reverse_map_users[i]);
        else printf("I%-2d: ", graphe->reverse_map_articles[i - graphe->nb_users]);
        
        for (int j = 0; j < taille && j < 10; j++) {
            printf("%-3d ", graphe->matrice_adjacence[i][j]);
        }
        printf("\n");
    }
}

// PageRank selon la formule PR = α * M * PR + (1-α) * d
ResultatPageRank* pagerank_unifie(GrapheBipartite *graphe, float alpha, float tolerance, int max_iterations) {
    ResultatPageRank *resultat = malloc(sizeof(ResultatPageRank));
    int n = graphe->taille_totale;
    
    // Initialiser les vecteurs
    resultat->pagerank_vector = malloc(n * sizeof(float));
    float *new_pr = malloc(n * sizeof(float));
    float *d_vector = malloc(n * sizeof(float));  // Vecteur de personnalisation
    
    // Initialisation du vecteur PR : valeur uniforme 1/N
    float init_value = 1.0 / n;
    for (int i = 0; i < n; i++) {
        resultat->pagerank_vector[i] = init_value;
        d_vector[i] = init_value;  // Vecteur de personnalisation uniforme
    }
    
    // Calculer les degrés sortants pour normaliser M
    int *degre_sortant = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            degre_sortant[i] += graphe->matrice_adjacence[i][j];
        }
    }
    
    printf("\nDémarrage PageRank avec α = %.2f\n", alpha);
    
    // Algorithme PageRank itératif : PR = α * M * PR + (1-α) * d
    for (int iter = 0; iter < max_iterations; iter++) {
        // Initialiser avec le terme de téléportation (1-α) * d
        for (int i = 0; i < n; i++) {
            new_pr[i] = (1 - alpha) * d_vector[i];
        }
        
        // Ajouter le terme de marche aléatoire α * M * PR
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graphe->matrice_adjacence[j][i] > 0 && degre_sortant[j] > 0) {
                    // M[i][j] = adjacence[j][i] / degre_sortant[j]
                    new_pr[i] += alpha * resultat->pagerank_vector[j] * 
                                graphe->matrice_adjacence[j][i] / (float)degre_sortant[j];
                }
            }
        }
        
        // Vérifier la convergence
        float diff = 0.0;
        for (int i = 0; i < n; i++) {
            diff += fabs(new_pr[i] - resultat->pagerank_vector[i]);
        }
        
        // Copier les nouvelles valeurs
        memcpy(resultat->pagerank_vector, new_pr, n * sizeof(float));
        
        if (iter % 10 == 0 || diff < tolerance) {
            printf("Iteration %d, différence: %f\n", iter + 1, diff);
        }
        
        if (diff < tolerance) {
            resultat->nb_iterations = iter + 1;
            resultat->convergence = diff;
            break;
        }
    }
    
    free(new_pr);
    free(d_vector);
    free(degre_sortant);
    
    return resultat;
}

// Fonction PageRank pour Train-data et recommandations top-N
void pagerank_train_data(GrapheBipartite *graphe, ResultatPageRank *pagerank, 
                        int user_id, int top_n) {
    
    int user_idx = -1;
    if (user_id <= graphe->max_user_id && graphe->map_users[user_id] != -1) {
        user_idx = graphe->map_users[user_id];
    }
    
    if (user_idx == -1) {
        printf("Utilisateur %d non trouvé dans les données d'entraînement\n", user_id);
        return;
    }
    
    // Créer une liste des articles avec leurs scores PageRank
    typedef struct {
        int article_id;
        float score_pagerank;
        int deja_vu;
    } ArticleScore;
    
    ArticleScore *articles = malloc(graphe->nb_articles * sizeof(ArticleScore));
    
    // Remplir la liste des articles
    for (int i = 0; i < graphe->nb_articles; i++) {
        int article_global_idx = graphe->nb_users + i;
        articles[i].article_id = graphe->reverse_map_articles[i];
        articles[i].score_pagerank = pagerank->pagerank_vector[article_global_idx];
        
        // Vérifier si l'utilisateur a déjà interagi avec cet article
        articles[i].deja_vu = graphe->matrice_adjacence[user_idx][article_global_idx];
    }
    
    // Trier les articles par score PageRank décroissant (tri à bulles)
    for (int i = 0; i < graphe->nb_articles - 1; i++) {
        for (int j = 0; j < graphe->nb_articles - i - 1; j++) {
            if (articles[j].score_pagerank < articles[j + 1].score_pagerank) {
                ArticleScore temp = articles[j];
                articles[j] = articles[j + 1];
                articles[j + 1] = temp;
            }
        }
    }
    
    // Afficher les recommandations top-N (articles non vus)
    printf("\n=== TOP-%d Recommandations pour l'utilisateur %d ===\n", top_n, user_id);
    int count = 0;
    for (int i = 0; i < graphe->nb_articles && count < top_n; i++) {
        if (!articles[i].deja_vu) {  // Article non encore vu
            count++;
            printf("%d. Article %d (Score PageRank: %.6f)\n", 
                   count, articles[i].article_id, articles[i].score_pagerank);
        }
    }
    
    // Afficher aussi les articles déjà vus avec les meilleurs scores (pour validation)
    printf("\n=== Articles déjà vus avec meilleurs scores PageRank ===\n");
    count = 0;
    for (int i = 0; i < graphe->nb_articles && count < 3; i++) {
        if (articles[i].deja_vu) {
            count++;
            printf("%d. Article %d (Score PageRank: %.6f) [DÉJÀ VU]\n", 
                   count, articles[i].article_id, articles[i].score_pagerank);
        }
    }
    
    free(articles);
}

// Fonction Test-PageRank pour évaluer le modèle
void test_pagerank(GrapheBipartite *graphe_train, Transaction *test_data, int nb_test, 
                  ResultatPageRank *pagerank, int top_n) {
    printf("\n=== TEST-PAGERANK : Évaluation sur %d transactions de test ===\n", nb_test);
    
    int hits = 0;  // Nombre de recommandations correctes
    int total_recommendations = 0;
    
    // Pour chaque transaction de test
    for (int t = 0; t < nb_test; t++) {
        int user_id = test_data[t].id_user;
        int article_test = test_data[t].id_article;
        
        // Vérifier si l'utilisateur existe dans les données d'entraînement
        if (user_id > graphe_train->max_user_id || 
            graphe_train->map_users[user_id] == -1) {
            continue;  // Skip si utilisateur pas dans train
        }
        
        int user_idx = graphe_train->map_users[user_id];
        
        // Créer liste des recommandations pour cet utilisateur
        typedef struct {
            int article_id;
            float score;
        } RecommendationScore;
        
        RecommendationScore *recommandations = malloc(graphe_train->nb_articles * sizeof(RecommendationScore));
        int nb_recommandations = 0;
        
        // Collecter les articles non vus avec leurs scores
        for (int i = 0; i < graphe_train->nb_articles; i++) {
            int article_global_idx = graphe_train->nb_users + i;
            int article_id = graphe_train->reverse_map_articles[i];
            
            // Si pas encore vu dans les données d'entraînement
            if (!graphe_train->matrice_adjacence[user_idx][article_global_idx]) {
                recommandations[nb_recommandations].article_id = article_id;
                recommandations[nb_recommandations].score = pagerank->pagerank_vector[article_global_idx];
                nb_recommandations++;
            }
        }
        
        // Trier par score décroissant
        for (int i = 0; i < nb_recommandations - 1; i++) {
            for (int j = 0; j < nb_recommandations - i - 1; j++) {
                if (recommandations[j].score < recommandations[j + 1].score) {
                    RecommendationScore temp = recommandations[j];
                    recommandations[j] = recommandations[j + 1];
                    recommandations[j + 1] = temp;
                }
            }
        }
        
        // Vérifier si l'article de test est dans le top-N
        int found_in_topn = 0;
        int limite = (top_n < nb_recommandations) ? top_n : nb_recommandations;
        for (int i = 0; i < limite; i++) {
            if (recommandations[i].article_id == article_test) {
                found_in_topn = 1;
                break;
            }
        }
        
        if (found_in_topn) hits++;
        total_recommendations++;
        
        free(recommandations);
    }
    
    // Calculer et afficher les métriques
    float precision = (total_recommendations > 0) ? (float)hits / total_recommendations : 0.0;
    printf("Résultats Test-PageRank:\n");
    printf("- Transactions de test évaluées: %d\n", total_recommendations);
    printf("- Hits dans Top-%d: %d\n", top_n, hits);
    printf("- Précision@%d: %.4f (%.2f%%)\n", top_n, precision, precision * 100);
}

// Fonction principale
int main() {
    // Données d'exemple (Train.txt)
    Transaction train_data[] = {
        {249, 1087, 21, 4.3, 969215645},
        {83, 1134, 34, 3.5, 1000121189},
        {7, 1231, 43, 2.4, 998855800},
        {64, 1042, 32, 3.0, 952138789},
        {249, 1087, 21, 4.5, 970215645},  // Doublon géré
        {83, 1134, 34, 3.7, 1001121189},
        {7, 1231, 43, 2.6, 999855800},
        {249, 1231, 43, 4.0, 970215645},
        {83, 1042, 32, 3.2, 1001121189},
        {64, 1231, 43, 3.8, 952140000},
        {249, 1042, 32, 4.1, 970220000}
    };
    
    // Données de test (Test.txt)
    Transaction test_data[] = {
        {249, 1134, 34, 4.0, 980000000},  // Nouvel article pour user 249
        {83, 1231, 43, 3.5, 980100000},   // Article déjà vu par d'autres
        {7, 1042, 32, 2.8, 980200000}     // Nouvel article pour user 7
    };
    
    int nb_train = sizeof(train_data) / sizeof(Transaction);
    int nb_test = sizeof(test_data) / sizeof(Transaction);
    
    printf("=== CONSTRUCTION DU GRAPHE BIPARTITE (Train-data) ===\n");
    
    // Étape 1: Construire le graphe à partir des données d'entraînement
    GrapheBipartite graphe = {0};
    creer_mappings(train_data, nb_train, &graphe);
    construire_matrice_adjacence(train_data, nb_train, &graphe);
    
    printf("\n=== APPLICATION PAGERANK ===\n");
    
    // Étape 2: Appliquer PageRank
    ResultatPageRank *pagerank = pagerank_unifie(&graphe, 0.85, 1e-6, 100);
    
    printf("\nConvergence PageRank: %d itérations (diff: %.8f)\n", 
           pagerank->nb_iterations, pagerank->convergence);
    
    // Étape 3: PageRank(Train-data, User) - Recommandations
    printf("\n=== PAGERANK(TRAIN-DATA, USER) - RECOMMANDATIONS ===\n");
    pagerank_train_data(&graphe, pagerank, 249, 5);  // Top-5 pour user 249
    pagerank_train_data(&graphe, pagerank, 83, 3);   // Top-3 pour user 83
    
    // Étape 4: Test-PageRank(Train-data, Test-data)
    test_pagerank(&graphe, test_data, nb_test, pagerank, 3);
    
    // Libération mémoire
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
    
    return 0;
}
