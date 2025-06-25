#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Constantes
#define MAX_USERS 1000
#define MAX_ARTICLES 1000
#define MAX_LINE_LENGTH 256

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

// Prototypes des fonctions
RecommandeurKNN* creer_recommandeur(int k);
void liberer_recommandeur(RecommandeurKNN* rec);
int charger_donnees_depuis_fichier(RecommandeurKNN* rec, const char* filename);
int construire_matrice_evaluations(RecommandeurKNN* rec);
void calculer_moyennes_utilisateurs(RecommandeurKNN* rec);
double correlation_pearson_entre_users(RecommandeurKNN* rec, int index_user1, int index_user2);
void calculer_matrice_similarite_complete(RecommandeurKNN* rec);
double** Pearson(const char* filename);
float Predict(RecommandeurKNN* rec, unsigned int id_user, unsigned int id_article);
Voisin* trouver_k_voisins_similaires(RecommandeurKNN* rec, unsigned int id_user);
double calculer_prediction_ponderee(RecommandeurKNN* rec, Voisin* voisins, unsigned int id_article, unsigned int id_user);
int trouver_index_utilisateur(RecommandeurKNN* rec, unsigned int id_user);
int trouver_index_article(RecommandeurKNN* rec, unsigned int id_article);
void trier_voisins_par_similarite(Voisin* voisins, int nb_voisins);
void afficher_matrice_similarite(RecommandeurKNN* rec, int limite);
void afficher_stats_recommandeur(RecommandeurKNN* rec);
void tester_predictions(RecommandeurKNN* rec);

// Implémentation des fonctions

RecommandeurKNN* creer_recommandeur(int k) {
    RecommandeurKNN* rec = malloc(sizeof(RecommandeurKNN));
    if (!rec) return NULL;
    
    memset(rec, 0, sizeof(RecommandeurKNN));
    rec->k = k;
    
    return rec;
}

void liberer_recommandeur(RecommandeurKNN* rec) {
    if (rec) {
        free(rec);
    }
}

int charger_donnees_depuis_fichier(RecommandeurKNN* rec, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    rec->nb_transactions = 0;
    rec->nb_users = 0;
    rec->nb_articles = 0;
    
    printf("Chargement des données depuis %s...\n", filename);
    
    // Lire ligne par ligne
    while (fgets(line, sizeof(line), file) && rec->nb_transactions < MAX_USERS * MAX_ARTICLES) {
        // Supprimer le retour à la ligne
        line[strcspn(line, "\n")] = 0;
        
        // Ignorer les lignes vides ou les commentaires
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Parser la ligne selon le format attendu
        // Format supposé : id_user,id_article,id_cat,evaluation,timestamp
        // ou : id_user id_article id_cat evaluation timestamp
        
        int id_user, id_article, id_cat;
        float evaluation;
        double timestamp = 0.0;
        
        // Essayer le format avec virgules
        int parsed = sscanf(line, "%d,%d,%d,%f,%lf", &id_user, &id_article, &id_cat, &evaluation, &timestamp);
        
        // Si échec, essayer le format avec espaces
        if (parsed < 4) {
            parsed = sscanf(line, "%d %d %d %f %lf", &id_user, &id_article, &id_cat, &evaluation, &timestamp);
        }
        
        // Si échec, essayer sans timestamp
        if (parsed < 4) {
            parsed = sscanf(line, "%d,%d,%d,%f", &id_user, &id_article, &id_cat, &evaluation);
            if (parsed < 4) {
                parsed = sscanf(line, "%d %d %d %f", &id_user, &id_article, &id_cat, &evaluation);
            }
        }
        
        if (parsed >= 4) {
            // Ajouter la transaction
            rec->transactions[rec->nb_transactions].id_user = id_user;
            rec->transactions[rec->nb_transactions].id_article = id_article;
            rec->transactions[rec->nb_transactions].id_cat = id_cat;
            rec->transactions[rec->nb_transactions].evaluation = evaluation;
            rec->transactions[rec->nb_transactions].timestamp = timestamp;
            rec->nb_transactions++;
            
            // Ajouter l'utilisateur s'il n'existe pas
            int user_existe = 0;
            for (int i = 0; i < rec->nb_users; i++) {
                if (rec->users[i].id_user == (unsigned int)id_user) {
                    rec->users[i].nb_fois++;
                    user_existe = 1;
                    break;
                }
            }
            if (!user_existe && rec->nb_users < MAX_USERS) {
                rec->users[rec->nb_users].id_user = id_user;
                rec->users[rec->nb_users].nb_fois = 1;
                rec->nb_users++;
            }
            
            // Ajouter l'article s'il n'existe pas
            int article_existe = 0;
            for (int i = 0; i < rec->nb_articles; i++) {
                if (rec->articles[i].id_article == (unsigned int)id_article) {
                    rec->articles[i].nb_fois++;
                    article_existe = 1;
                    break;
                }
            }
            if (!article_existe && rec->nb_articles < MAX_ARTICLES) {
                rec->articles[rec->nb_articles].id_article = id_article;
                rec->articles[rec->nb_articles].id_cat = id_cat;
                rec->articles[rec->nb_articles].nb_fois = 1;
                rec->nb_articles++;
            }
        } else {
            printf("Ligne ignorée (format incorrect) : %s\n", line);
        }
    }
    
    fclose(file);
    
    printf("Données chargées avec succès :\n");
    printf("- %d transactions\n", rec->nb_transactions);
    printf("- %d utilisateurs uniques\n", rec->nb_users);
    printf("- %d articles uniques\n", rec->nb_articles);
    
    rec->donnees_chargees = 1;
    return 1;
}

int construire_matrice_evaluations(RecommandeurKNN* rec) {
    if (!rec || !rec->donnees_chargees) return 0;
    
    // Initialiser la matrice à 0
    for (int i = 0; i < MAX_USERS; i++) {
        for (int j = 0; j < MAX_ARTICLES; j++) {
            rec->matrice_evaluations[i][j] = 0.0;
        }
    }
    
    // Remplir la matrice avec les évaluations
    for (int t = 0; t < rec->nb_transactions; t++) {
        int index_user = trouver_index_utilisateur(rec, rec->transactions[t].id_user);
        int index_article = trouver_index_article(rec, rec->transactions[t].id_article);
        
        if (index_user >= 0 && index_article >= 0) {
            rec->matrice_evaluations[index_user][index_article] = rec->transactions[t].evaluation;
        }
    }
    
    printf("Matrice utilisateur-article construite\n");
    return 1;
}

void calculer_moyennes_utilisateurs(RecommandeurKNN* rec) {
    for (int i = 0; i < rec->nb_users; i++) {
        double somme = 0.0;
        int compteur = 0;
        
        for (int j = 0; j < rec->nb_articles; j++) {
            if (rec->matrice_evaluations[i][j] > 0) {
                somme += rec->matrice_evaluations[i][j];
                compteur++;
            }
        }
        
        rec->moyenne_utilisateur[i] = (compteur > 0) ? somme / compteur : 0.0;
        rec->nb_eval_utilisateur[i] = compteur;
    }
    
    printf("Moyennes des utilisateurs calculées\n");
}

double correlation_pearson_entre_users(RecommandeurKNN* rec, int index_user1, int index_user2) {
    if (index_user1 == index_user2) return 1.0;
    
    double somme1 = 0.0, somme2 = 0.0;
    int compteur = 0;
    
    // Trouver les articles notés par les deux utilisateurs
    for (int i = 0; i < rec->nb_articles; i++) {
        float note1 = rec->matrice_evaluations[index_user1][i];
        float note2 = rec->matrice_evaluations[index_user2][i];
        
        if (note1 > 0 && note2 > 0) {
            somme1 += note1;
            somme2 += note2;
            compteur++;
        }
    }
    
    if (compteur < 2) return 0.0; // Pas assez d'éléments communs
    
    double moyenne1 = somme1 / compteur;
    double moyenne2 = somme2 / compteur;
    
    // Calculer la corrélation de Pearson
    double numerateur = 0.0;
    double denominateur1 = 0.0;
    double denominateur2 = 0.0;
    
    for (int i = 0; i < rec->nb_articles; i++) {
        float note1 = rec->matrice_evaluations[index_user1][i];
        float note2 = rec->matrice_evaluations[index_user2][i];
        
        if (note1 > 0 && note2 > 0) {
            double diff1 = note1 - moyenne1;
            double diff2 = note2 - moyenne2;
            
            numerateur += diff1 * diff2;
            denominateur1 += diff1 * diff1;
            denominateur2 += diff2 * diff2;
        }
    }
    
    if (denominateur1 == 0.0 || denominateur2 == 0.0) {
        return 0.0;
    }
    
    return numerateur / (sqrt(denominateur1) * sqrt(denominateur2));
}

void calculer_matrice_similarite_complete(RecommandeurKNN* rec) {
    printf("Calcul de la matrice de similarité...\n");
    
    for (int i = 0; i < rec->nb_users; i++) {
        for (int j = 0; j < rec->nb_users; j++) {
            rec->matrice_similarite[i][j] = correlation_pearson_entre_users(rec, i, j);
        }
        
        if ((i + 1) % 10 == 0 || i == rec->nb_users - 1) {
            printf("Utilisateur %d/%d traité\n", i + 1, rec->nb_users);
        }
    }
    
    rec->matrice_similarite_calculee = 1;
    printf("Matrice de similarité calculée avec succès\n");
}

double** Pearson(const char* filename) {
    printf("=== CALCUL MATRICE PEARSON ===\n");
    
    RecommandeurKNN* rec = creer_recommandeur(5);
    if (!rec) {
        printf("Erreur : Impossible de créer le recommandeur\n");
        return NULL;
    }
    
    if (!charger_donnees_depuis_fichier(rec, filename)) {
        liberer_recommandeur(rec);
        return NULL;
    }
    
    if (!construire_matrice_evaluations(rec)) {
        liberer_recommandeur(rec);
        return NULL;
    }
    
    calculer_moyennes_utilisateurs(rec);
    calculer_matrice_similarite_complete(rec);
    
    // Allouer et copier la matrice de retour
    double** matrice_retour = malloc(rec->nb_users * sizeof(double*));
    if (!matrice_retour) {
        liberer_recommandeur(rec);
        return NULL;
    }
    
    for (int i = 0; i < rec->nb_users; i++) {
        matrice_retour[i] = malloc(rec->nb_users * sizeof(double));
        if (!matrice_retour[i]) {
            for (int j = 0; j < i; j++) {
                free(matrice_retour[j]);
            }
            free(matrice_retour);
            liberer_recommandeur(rec);
            return NULL;
        }
        
        for (int j = 0; j < rec->nb_users; j++) {
            matrice_retour[i][j] = rec->matrice_similarite[i][j];
        }
    }
    
    printf("Matrice Pearson retournée (%dx%d)\n", rec->nb_users, rec->nb_users);
    // Note : rec reste alloué pour utilisation ultérieure
    
    return matrice_retour;
}

int trouver_index_utilisateur(RecommandeurKNN* rec, unsigned int id_user) {
    for (int i = 0; i < rec->nb_users; i++) {
        if (rec->users[i].id_user == id_user) {
            return i;
        }
    }
    return -1;
}

int trouver_index_article(RecommandeurKNN* rec, unsigned int id_article) {
    for (int i = 0; i < rec->nb_articles; i++) {
        if (rec->articles[i].id_article == id_article) {
            return i;
        }
    }
    return -1;
}

void trier_voisins_par_similarite(Voisin* voisins, int nb_voisins) {
    for (int i = 0; i < nb_voisins - 1; i++) {
        for (int j = 0; j < nb_voisins - i - 1; j++) {
            if (voisins[j].similarite < voisins[j + 1].similarite) {
                Voisin temp = voisins[j];
                voisins[j] = voisins[j + 1];
                voisins[j + 1] = temp;
            }
        }
    }
}

Voisin* trouver_k_voisins_similaires(RecommandeurKNN* rec, unsigned int id_user) {
    int index_user = trouver_index_utilisateur(rec, id_user);
    if (index_user < 0) return NULL;
    
    Voisin* tous_voisins = malloc(rec->nb_users * sizeof(Voisin));
    if (!tous_voisins) return NULL;
    
    int nb_voisins = 0;
    
    for (int i = 0; i < rec->nb_users; i++) {
        if (i != index_user) {
            tous_voisins[nb_voisins].index_utilisateur = i;
            tous_voisins[nb_voisins].id_utilisateur = rec->users[i].id_user;
            tous_voisins[nb_voisins].similarite = rec->matrice_similarite[index_user][i];
            nb_voisins++;
        }
    }
    
    trier_voisins_par_similarite(tous_voisins, nb_voisins);
    
    int k_effectif = (rec->k < nb_voisins) ? rec->k : nb_voisins;
    Voisin* k_voisins = malloc((k_effectif + 1) * sizeof(Voisin));
    
    if (k_voisins) {
        for (int i = 0; i < k_effectif; i++) {
            k_voisins[i] = tous_voisins[i];
        }
        k_voisins[k_effectif].index_utilisateur = -1; // Marqueur de fin
    }
    
    free(tous_voisins);
    return k_voisins;
}

double calculer_prediction_ponderee(RecommandeurKNN* rec, Voisin* voisins, unsigned int id_article, unsigned int id_user) {
    int index_user = trouver_index_utilisateur(rec, id_user);
    int index_article = trouver_index_article(rec, id_article);
    
    if (index_user < 0 || index_article < 0) return -1.0;
    
    double numerateur = 0.0;
    double denominateur = 0.0;
    double moyenne_user = rec->moyenne_utilisateur[index_user];
    
    for (int i = 0; voisins[i].index_utilisateur >= 0; i++) {
        int index_voisin = voisins[i].index_utilisateur;
        double similarite = voisins[i].similarite;
        
        if (rec->matrice_evaluations[index_voisin][index_article] > 0) {
            double note_voisin = rec->matrice_evaluations[index_voisin][index_article];
            double moyenne_voisin = rec->moyenne_utilisateur[index_voisin];
            
            numerateur += similarite * (note_voisin - moyenne_voisin);
            denominateur += fabs(similarite);
        }
    }
    
    if (denominateur == 0.0) {
        return moyenne_user;
    }
    
    double prediction = moyenne_user + (numerateur / denominateur);
    
    if (prediction < 1.0) prediction = 1.0;
    if (prediction > 5.0) prediction = 5.0;
    
    return prediction;
}

float Predict(RecommandeurKNN* rec, unsigned int id_user, unsigned int id_article) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Erreur : Matrice de similarité non calculée\n");
        return -1.0;
    }
    
    int index_user = trouver_index_utilisateur(rec, id_user);
    int index_article = trouver_index_article(rec, id_article);
    
    if (index_user < 0 || index_article < 0) {
        return -1.0;
    }
    
    // Vérifier si déjà noté
    if (rec->matrice_evaluations[index_user][index_article] > 0) {
        return rec->matrice_evaluations[index_user][index_article];
    }
    
    Voisin* voisins = trouver_k_voisins_similaires(rec, id_user);
    if (!voisins) {
        return -1.0;
    }
    
    double prediction = calculer_prediction_ponderee(rec, voisins, id_article, id_user);
    
    free(voisins);
    return (float)prediction;
}

void afficher_matrice_similarite(RecommandeurKNN* rec, int limite) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Matrice de similarité non calculée\n");
        return;
    }
    
    int max_affichage = (limite > 0 && limite < rec->nb_users) ? limite : rec->nb_users;
    
    printf("\n=== MATRICE DE SIMILARITÉ (Pearson) ===\n");
    printf("Utilisateurs: 0 à %d\n\n", max_affichage - 1);
    
    // En-tête
    printf("     ");
    for (int j = 0; j < max_affichage; j++) {
        printf("%6d ", j);
    }
    printf("\n");
    
    // Matrice
    for (int i = 0; i < max_affichage; i++) {
        printf("%3d: ", i);
        for (int j = 0; j < max_affichage; j++) {
            printf("%6.3f ", rec->matrice_similarite[i][j]);
        }
        printf("\n");
    }
    
    if (limite > 0 && limite < rec->nb_users) {
        printf("... (affichage limité à %d/%d utilisateurs)\n", limite, rec->nb_users);
    }
}

void afficher_stats_recommandeur(RecommandeurKNN* rec) {
    if (!rec) return;
    
    printf("\n=== STATISTIQUES DU RECOMMANDEUR ===\n");
    printf("Nombre d'utilisateurs: %d\n", rec->nb_users);
    printf("Nombre d'articles: %d\n", rec->nb_articles);
    printf("Nombre de transactions: %d\n", rec->nb_transactions);
    printf("Valeur de k: %d\n", rec->k);
    printf("Données chargées: %s\n", rec->donnees_chargees ? "Oui" : "Non");
    printf("Similarité calculée: %s\n", rec->matrice_similarite_calculee ? "Oui" : "Non");
    
    if (rec->donnees_chargees) {
        printf("\nTop 5 utilisateurs les plus actifs:\n");
        for (int i = 0; i < 5 && i < rec->nb_users; i++) {
            printf("- Utilisateur %u: %d évaluations\n", 
                   rec->users[i].id_user, rec->users[i].nb_fois);
        }
        
        printf("\nTop 5 articles les plus évalués:\n");
        for (int i = 0; i < 5 && i < rec->nb_articles; i++) {
            printf("- Article %u: %d évaluations\n", 
                   rec->articles[i].id_article, rec->articles[i].nb_fois);
        }
    }
}

void tester_predictions(RecommandeurKNN* rec) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Système non prêt pour les prédictions\n");
        return;
    }
    
    printf("\n=== TEST DE PRÉDICTIONS ===\n");
    
    // Tester quelques prédictions
    for (int i = 0; i < 5 && i < rec->nb_users; i++) {
        unsigned int id_user = rec->users[i].id_user;
        
        for (int j = 0; j < 3 && j < rec->nb_articles; j++) {
            unsigned int id_article = rec->articles[j].id_article;
            
            float prediction = Predict(rec, id_user, id_article);
            
            printf("Utilisateur %u, Article %u: %.2f\n", 
                   id_user, id_article, prediction);
        }
    }
}

// Fonction principale de démonstration
int main() {
    printf("=== SYSTÈME DE RECOMMANDATION KNN ===\n\n");
    
    // Créer le recommandeur avec k=5
    RecommandeurKNN* rec = creer_recommandeur(5);
    if (!rec) {
        printf("Erreur: Impossible de créer le recommandeur\n");
        return -1;
    }
    
    // Charger les données
    if (!charger_donnees_depuis_fichier(rec, "données.txt")) {
        printf("Erreur: Impossible de charger les données\n");
        liberer_recommandeur(rec);
        return -1;
    }
    
    // Construire la matrice utilisateur-article
    if (!construire_matrice_evaluations(rec)) {
        printf("Erreur: Impossible de construire la matrice\n");
        liberer_recommandeur(rec);
        return -1;
    }
    
    // Calculer les moyennes
    calculer_moyennes_utilisateurs(rec);
    
    // Calculer la matrice de similarité
    calculer_matrice_similarite_complete(rec);
    
    // Afficher les statistiques
    afficher_stats_recommandeur(rec);
    
    // Afficher la matrice de similarité (limitée à 10x10)
    afficher_matrice_similarite(rec, 10);
    
    // Tester quelques prédictions
    tester_predictions(rec);
    
    // Test avec la fonction Pearson
    printf("\n=== TEST FONCTION PEARSON ===\n");
    double** matrice_pearson = Pearson("données.txt");
    if (matrice_pearson) {
        printf("Matrice Pearson calculée avec succès\n");
        // Libérer la matrice
        for (int i = 0; i < rec->nb_users; i++) {
            free(matrice_pearson[i]);
        }
        free(matrice_pearson);
    }
    
    // Nettoyer la mémoire
    liberer_recommandeur(rec);
    
    printf("\n=== FIN DU PROGRAMME ===\n");
    return 0;
}
