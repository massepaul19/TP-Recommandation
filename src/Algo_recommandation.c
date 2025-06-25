#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "reco.h"

#define MAX_UTILISATEURS 100
#define MAX_ARTICLES 100
#define MAX_K 20

// Structure pour représenter une notation

typedef struct {
    int id_utilisateur;
    int id_article;
    double note;
} Notation;

// Structure pour les voisins avec leur similarité
typedef struct {
    int id_utilisateur;
    double similarite;
} Voisin;

// Structure principale du système de recommandation
typedef struct {
    double matrice_utilisateur_article[MAX_UTILISATEURS][MAX_ARTICLES];
    double matrice_similarite[MAX_UTILISATEURS][MAX_UTILISATEURS];
    int nombre_utilisateurs;
    int nombre_articles;
    int k; // nombre de voisins
} RecommandeurKNN;


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


// Fonction pour initialiser la matrice avec des zéros

void initialiser_matrice(double matrice[MAX_UTILISATEURS][MAX_ARTICLES], int lignes, int colonnes) {
    for (int i = 0; i < lignes; i++) {
        for (int j = 0; j < colonnes; j++) {
            matrice[i][j] = 0.0;
        }
    }
}

// Fonction pour initialiser le recommandeur

void initialiser_recommandeur(RecommandeurKNN* rec, int k) {
    rec->k = k;
    rec->nombre_utilisateurs = 0;
    rec->nombre_articles = 0;
    initialiser_matrice(rec->matrice_utilisateur_article, MAX_UTILISATEURS, MAX_ARTICLES);
    
    // Initialiser la matrice de similarité
    
    for (int i = 0; i < MAX_UTILISATEURS; i++) {
        for (int j = 0; j < MAX_UTILISATEURS; j++) {
            rec->matrice_similarite[i][j] = 0.0;
        }
    }
}

int Lire_Transaction(FILE *file, Transaction *t) {

    return fscanf(file, "%d %d %d %f %lf", &t->id_user, &t->id_article, &t->id_cat, &t->evaluation, &t->timestamp);
    
}

// Calculer la matrice de similarité (utilise Pearson par défaut)

void calculer_matrice_similarite(RecommandeurKNN* rec, int utiliser_cosinus) {
    for (int i = 0; i < rec->nombre_utilisateurs; i++) {
        for (int j = 0; j < rec->nombre_utilisateurs; j++) {
            if (i == j) {
                rec->matrice_similarite[i][j] = 1.0;
            } else {
                if (utiliser_cosinus) {
                    rec->matrice_similarite[i][j] = similarite_cosinus(rec, i, j);
                } else {
                    rec->matrice_similarite[i][j] = correlation_pearson(rec, i, j);
                }
            }
        }
    }
}

// Calculer la corrélation de Pearson entre deux utilisateurs

double Pearson(char *file_name) {

file = fopen(file
    double somme1 = 0.0, somme2 = 0.0;
    int compteur = 0;
    
    // Calculer les moyennes pour les articles communs
    for (int i = 0; i < rec->nombre_articles; i++) {
        double note1 = rec->matrice_utilisateur_article[utilisateur1][i];
        double note2 = rec->matrice_utilisateur_article[utilisateur2][i];
        
        if (note1 > 0 && note2 > 0) {
            somme1 += note1;
            somme2 += note2;
            compteur++;
        }
    }
    
    if (compteur < 2) return 0.0;
    
    double moyenne1 = somme1 / compteur;
    double moyenne2 = somme2 / compteur;
    
    // Calculer la corrélation
    double numerateur = 0.0;
    double denominateur1 = 0.0;
    double denominateur2 = 0.0;
    
    for (int i = 0; i < rec->nombre_articles; i++) {
        double note1 = rec->matrice_utilisateur_article[utilisateur1][i];
        double note2 = rec->matrice_utilisateur_article[utilisateur2][i];
        
        if (note1 > 0 && note2 > 0) {
            double difference1 = note1 - moyenne1;
            double difference2 = note2 - moyenne2;
            
            numerateur += difference1 * difference2;
            denominateur1 += difference1 * difference1;
            denominateur2 += difference2 * difference2;
        }
    }
    
    if (denominateur1 == 0.0 || denominateur2 == 0.0) {
        return 0.0;
    }
    
    return numerateur / (sqrt(denominateur1) * sqrt(denominateur2));
}

// Construire la matrice utilisateur-article à partir des données
void construire_matrice_utilisateur_article(RecommandeurKNN* rec, Notation* notations, int nombre_notations) {
    // Trouver le nombre max d'utilisateurs et d'articles
    int max_utilisateur = 0, max_article = 0;
    
    for (int i = 0; i < nombre_notations; i++) {
        if (notations[i].id_utilisateur > max_utilisateur) 
            max_utilisateur = notations[i].id_utilisateur;
        if (notations[i].id_article > max_article) 
            max_article = notations[i].id_article;
    }
    
    rec->nombre_utilisateurs = max_utilisateur + 1;
    rec->nombre_articles = max_article + 1;
    
    // Remplir la matrice
    for (int i = 0; i < nombre_notations; i++) {
        int utilisateur = notations[i].id_utilisateur;
        int article = notations[i].id_article;
        double note = notations[i].note;
        
        rec->matrice_utilisateur_article[utilisateur][article] = note;
    }
}


// Fonction de comparaison pour trier les voisins

int comparer_voisins(const void* a, const void* b) {
    Voisin* v1 = (Voisin*)a;
    Voisin* v2 = (Voisin*)b;
    
    if (v1->similarite > v2->similarite) return -1;
    if (v1->similarite < v2->similarite) return 1;
    return 0;
}

// Trouver les k voisins les plus similaires
int trouver_k_voisins(RecommandeurKNN* rec, int id_utilisateur, Voisin* voisins) {
    int compteur = 0;
    
    for (int i = 0; i < rec->nombre_utilisateurs; i++) {
        if (i != id_utilisateur && rec->matrice_similarite[id_utilisateur][i] > 0) {
            voisins[compteur].id_utilisateur = i;
            voisins[compteur].similarite = rec->matrice_similarite[id_utilisateur][i];
            compteur++;
        }
    }
    
    // Trier par similarité décroissante
    qsort(voisins, compteur, sizeof(Voisin), comparer_voisins);
    
    // Retourner au maximum k voisins
    return (compteur > rec->k) ? rec->k : compteur;
}

// Prédire la note qu'un utilisateur donnerait à un article
double predire_note(RecommandeurKNN* rec, int id_utilisateur, int id_article) {
    if (id_utilisateur >= rec->nombre_utilisateurs || id_article >= rec->nombre_articles) {
        return 0.0;
    }
    
    // Si l'utilisateur a déjà noté cet article
    if (rec->matrice_utilisateur_article[id_utilisateur][id_article] > 0) {
        return rec->matrice_utilisateur_article[id_utilisateur][id_article];
    }
    
    Voisin voisins[MAX_UTILISATEURS];
    int nombre_voisins = trouver_k_voisins(rec, id_utilisateur, voisins);
    
    if (nombre_voisins == 0) {
        return 0.0;
    }
    
    double somme_ponderee = 0.0;
    double somme_similarites = 0.0;
    
    for (int i = 0; i < nombre_voisins; i++) {
        int id_voisin = voisins[i].id_utilisateur;
        double similarite = voisins[i].similarite;
        double note_voisin = rec->matrice_utilisateur_article[id_voisin][id_article];
        
        if (note_voisin > 0) {
            somme_ponderee += similarite * note_voisin;
            somme_similarites += fabs(similarite);
        }
    }
    
    if (somme_similarites == 0.0) {
        return 0.0;
    }
    
    return somme_ponderee / somme_similarites;
}

// Structure pour les recommandations
typedef struct {
    int id_article;
    double note_predite;
} Recommandation;

// Fonction de comparaison pour trier les recommandations
int comparer_recommandations(const void* a, const void* b) {
    Recommandation* r1 = (Recommandation*)a;
    Recommandation* r2 = (Recommandation*)b;
    
    if (r1->note_predite > r2->note_predite) return -1;
    if (r1->note_predite < r2->note_predite) return 1;
    return 0;
}

// Générer des recommandations pour un utilisateur
int recommander_articles(RecommandeurKNN* rec, int id_utilisateur, 
                        Recommandation* recommandations, int max_recommandations) {
    if (id_utilisateur >= rec->nombre_utilisateurs) {
        return 0;
    }
    
    int compteur = 0;
    
    // Pour chaque article non noté par l'utilisateur
    for (int id_article = 0; id_article < rec->nombre_articles; id_article++) {
        if (rec->matrice_utilisateur_article[id_utilisateur][id_article] == 0) {
            double note_predite = predire_note(rec, id_utilisateur, id_article);
            
            if (note_predite > 0) {
                recommandations[compteur].id_article = id_article;
                recommandations[compteur].note_predite = note_predite;
                compteur++;
            }
        }
    }
    
    // Trier par note prédite décroissante
    qsort(recommandations, compteur, sizeof(Recommandation), comparer_recommandations);
    
    return (compteur > max_recommandations) ? max_recommandations : compteur;
}

// Afficher la matrice utilisateur-article
void afficher_matrice_utilisateur_article(RecommandeurKNN* rec) {
    printf("\nMatrice Utilisateur-Article:\n");
    printf("     ");
    for (int j = 0; j < rec->nombre_articles; j++) {
        printf("Art%d ", j);
    }
    printf("\n");
    
    for (int i = 0; i < rec->nombre_utilisateurs; i++) {
        printf("U%d   ", i);
        for (int j = 0; j < rec->nombre_articles; j++) {
            if (rec->matrice_utilisateur_article[i][j] > 0) {
                printf("%.1f  ", rec->matrice_utilisateur_article[i][j]);
            } else {
                printf(" -   ");
            }
        }
        printf("\n");
    }
}

// Afficher la matrice de similarité
void afficher_matrice_similarite(RecommandeurKNN* rec) {
    printf("\nMatrice de Similarité:\n");
    printf("     ");
    for (int j = 0; j < rec->nombre_utilisateurs; j++) {
        printf("  U%d   ", j);
    }
    printf("\n");
    
    for (int i = 0; i < rec->nombre_utilisateurs; i++) {
        printf("U%d   ", i);
        for (int j = 0; j < rec->nombre_utilisateurs; j++) {
            printf("%.3f  ", rec->matrice_similarite[i][j]);
        }
        printf("\n");
    }
}

// Afficher les voisins les plus proches d'un utilisateur
void afficher_voisins_proches(RecommandeurKNN* rec, int id_utilisateur) {
    Voisin voisins[MAX_UTILISATEURS];
    int nombre_voisins = trouver_k_voisins(rec, id_utilisateur, voisins);
    
    printf("\nVoisins les plus proches de l'utilisateur %d:\n", id_utilisateur);
    for (int i = 0; i < nombre_voisins; i++) {
        printf("  Utilisateur %d: similarité = %.3f\n", 
               voisins[i].id_utilisateur, voisins[i].similarite);
    }
}

// Fonction principale de démonstration
int main() {
    printf("=== Système de Recommandation KNN en C ===\n");
    printf("=== Implémentation en Français ===\n\n");
    
    // Données d'exemple basées sur le tableau de l'exercice
    Notation notations[] = {
        {1, 1, 5.0}, {1, 2, 3.0}, {1, 3, 4.0}, {1, 4, 4.0},
        {2, 1, 3.0}, {2, 2, 1.0}, {2, 3, 2.0}, {2, 4, 3.0}, {2, 5, 3.0},
        {3, 1, 4.0}, {3, 2, 3.0}, {3, 3, 4.0}, {3, 4, 3.0}, {3, 5, 5.0},
        {4, 1, 3.0}, {4, 2, 3.0}, {4, 3, 1.0}, {4, 4, 5.0}, {4, 5, 4.0},
        {5, 1, 1.0}, {5, 2, 5.0}, {5, 3, 5.0}, {5, 4, 2.0}, {5, 5, 1.0}
    };
    
    int nombre_notations = sizeof(notations) / sizeof(Notation);
    
    // Initialiser le recommandeur avec k=3
    RecommandeurKNN rec;
    initialiser_recommandeur(&rec, 3);
    
    // Construire la matrice utilisateur-article
    construire_matrice_utilisateur_article(&rec, notations, nombre_notations);
    
    printf("Données chargées: %d utilisateurs, %d articles, %d notations\n", 
           rec.nombre_utilisateurs, rec.nombre_articles, nombre_notations);
    
    // Afficher la matrice utilisateur-article
    afficher_matrice_utilisateur_article(&rec);
    
    // Calculer la matrice de similarité avec Pearson
    printf("\n--- Calcul avec corrélation de Pearson ---\n");
    calculer_matrice_similarite(&rec, 0); // 0 = Pearson, 1 = Cosinus
    afficher_matrice_similarite(&rec);
    
    // Afficher les voisins proches
    int utilisateur_test = 1;
    afficher_voisins_proches(&rec, utilisateur_test);
    
    // Faire des prédictions
    int article_test = 5;
    printf("\n=== Prédictions ===\n");
    double note_predite = predire_note(&rec, utilisateur_test, article_test);
    printf("Prédiction pour Utilisateur %d, Article %d: %.3f\n", 
           utilisateur_test, article_test, note_predite);
    
    // Générer des recommandations
    printf("\n=== Recommandations pour l'Utilisateur %d ===\n", utilisateur_test);
    Recommandation recommandations[MAX_ARTICLES];
    int nombre_recs = recommander_articles(&rec, utilisateur_test, recommandations, 5);
    
    for (int i = 0; i < nombre_recs; i++) {
        printf("%d. Article %d: note prédite = %.3f\n", 
               i + 1, recommandations[i].id_article, recommandations[i].note_predite);
    }
    
    // Comparer avec la similarité cosinus
    printf("\n--- Calcul avec similarité cosinus ---\n");
    calculer_matrice_similarite(&rec, 1); // 1 = Cosinus
    afficher_matrice_similarite(&rec);
    
    note_predite = predire_note(&rec, utilisateur_test, article_test);
    printf("Prédiction (Cosinus) pour Utilisateur %d, Article %d: %.3f\n", 
           utilisateur_test, article_test, note_predite);
    
    // Nouvelles recommandations avec cosinus
    printf("\n=== Recommandations (Cosinus) pour l'Utilisateur %d ===\n", utilisateur_test);
    nombre_recs = recommander_articles(&rec, utilisateur_test, recommandations, 5);
    
    for (int i = 0; i < nombre_recs; i++) {
        printf("%d. Article %d: note prédite = %.3f\n", 
               i + 1, recommandations[i].id_article, recommandations[i].note_predite);
    }
    
    printf("\n=== Fin de la démonstration ===\n");
    return 0;
}
