#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "reco_KNN.h"

RecommandeurKNN* recommandeur_global = NULL;

//----------------Initialisation des données depuis de le fichier ------------

RecommandeurKNN* initialiser_recommandeur_depuis_fichier(const char* filename) {
    RecommandeurKNN* rec = creer_recommandeur(5);
    if (!rec) return NULL;
    
    // Charger les transactions depuis le fichier
    rec->nb_transactions = compter_transactions(filename, rec->transactions, MAX_USERS * MAX_ARTICLES);
    if (rec->nb_transactions <= 0) {
        printf("Erreur : Aucune transaction chargée depuis %s\n", filename);
        liberer_recommandeur(rec);
        return NULL;
    }
    
    printf("Transactions chargées : %d\n", rec->nb_transactions);
    
    // Extraire les utilisateurs uniques
    rec->nb_users = 0;
    for (int i = 0; i < rec->nb_transactions; i++) {
        unsigned int id_user = rec->transactions[i].id_user;
        int trouve = 0;
        
        // Chercher si l'utilisateur existe déjà
        for (int j = 0; j < rec->nb_users; j++) {
            if (rec->users[j].id_user == id_user) {
                rec->users[j].nb_fois++;
                trouve = 1;
                break;
            }
        }
        
        // Ajouter nouvel utilisateur
        if (!trouve) {
            if (rec->nb_users >= MAX_USERS) {
                printf("Attention : Limite MAX_USERS (%d) atteinte\n", MAX_USERS);
                break;
            }
            rec->users[rec->nb_users].id_user = id_user;
            rec->users[rec->nb_users].nb_fois = 1;
            rec->nb_users++;
        }
    }
    
    // Extraire les articles uniques
    rec->nb_articles = 0;
    for (int i = 0; i < rec->nb_transactions; i++) {
        unsigned int id_article = rec->transactions[i].id_article;
        unsigned int id_cat = rec->transactions[i].id_cat;
        int trouve = 0;
        
        // Chercher si l'article existe déjà
        for (int j = 0; j < rec->nb_articles; j++) {
            if (rec->articles[j].id_article == id_article) {
                rec->articles[j].nb_fois++;
                trouve = 1;
                break;
            }
        }
        
        // Ajouter nouvel article
        if (!trouve) {
            if (rec->nb_articles >= MAX_ARTICLES) {
                printf("Attention : Limite MAX_ARTICLES (%d) atteinte\n", MAX_ARTICLES);
                break;
            }
            rec->articles[rec->nb_articles].id_article = id_article;
            rec->articles[rec->nb_articles].id_cat = id_cat;
            rec->articles[rec->nb_articles].nb_fois = 1;
            rec->nb_articles++;
        }
    }
    
    printf("Utilisateurs uniques : %d\n", rec->nb_users);
    printf("Articles uniques : %d\n", rec->nb_articles);
    
    // Vérifications de cohérence
    if (rec->nb_users == 0 || rec->nb_articles == 0) {
        printf("Erreur : Données insuffisantes (users: %d, articles: %d)\n", 
               rec->nb_users, rec->nb_articles);
        liberer_recommandeur(rec);
        return NULL;
    }
    
    rec->donnees_chargees = 1;
    
    // Optionnel : Afficher quelques statistiques
    printf("Moyenne d'évaluations par utilisateur : %.2f\n", 
           (double)rec->nb_transactions / rec->nb_users);
    printf("Moyenne d'évaluations par article : %.2f\n", 
           (double)rec->nb_transactions / rec->nb_articles);
    
    return rec;
}

//#####################################################################################
//--------------------------- Bloc pour le calcul de Person --------------------------
//#####################################################################################

//-----------------------------------------------------------------
//--------------- Construction de la matrice evaluations ---------

int construire_matrice_evaluations(RecommandeurKNN* rec) {
    if (!rec || !rec->donnees_chargees) {
        printf("Erreur : Recommandeur non initialisé\n");
        return 0;
    }
    
    // Initialiser la matrice avec des valeurs par défaut (0.0 = pas d'évaluation)
    for (int i = 0; i < rec->nb_users; i++) {
        for (int j = 0; j < rec->nb_articles; j++) {
            rec->matrice_evaluations[i][j] = 0.0f; // Valeur par défaut
        }
    }
    
    // Remplir la matrice avec les évaluations réelles
    for (int t = 0; t < rec->nb_transactions; t++) {
        int user_idx = trouver_index_utilisateur(rec, rec->transactions[t].id_user);
        int article_idx = trouver_index_article(rec, rec->transactions[t].id_article);
        
        if (user_idx >= 0 && user_idx < rec->nb_users && 
            article_idx >= 0 && article_idx < rec->nb_articles) {
            rec->matrice_evaluations[user_idx][article_idx] = rec->transactions[t].evaluation;
        }
    }
    
    printf("Matrice d'évaluations construite (%dx%d)\n", rec->nb_users, rec->nb_articles);
    return 1;
}

//-----------------------------------------------------------------

//--------------- Calcul des moyennes utilisateurs ---------

void calculer_moyennes_utilisateurs(RecommandeurKNN* rec) {
    if (!rec) return;
    
    for (int i = 0; i < rec->nb_users; i++) {
        double somme = 0.0;
        int nb_evaluations = 0;
        
        // Parcourir tous les articles pour cet utilisateur
        for (int j = 0; j < rec->nb_articles; j++) {
            if (rec->matrice_evaluations[i][j] > 0.0f) { // L'utilisateur a évalué cet article
                somme += rec->matrice_evaluations[i][j];
                nb_evaluations++;
            }
        }
        
        rec->moyenne_utilisateur[i] = (nb_evaluations > 0) ? (somme / nb_evaluations) : 0.0;
        rec->nb_eval_utilisateur[i] = nb_evaluations;
    }
    
    printf("Moyennes utilisateurs calculées\n");
}

//-----------------------------------------------------------------

//--------------- Calcul des correlation de pearson ---------

double correlation_pearson_entre_users(RecommandeurKNN* rec, int index_user1, int index_user2) {
    if (!rec || index_user1 < 0 || index_user2 < 0 || 
        index_user1 >= rec->nb_users || index_user2 >= rec->nb_users) {
        return 0.0;
    }
    
    // Cas particulier : même utilisateur
    if (index_user1 == index_user2) {
        return 1.0;
    }
    
    // Trouver les articles évalués par les deux utilisateurs
    double evaluations_user1[MAX_ARTICLES];
    double evaluations_user2[MAX_ARTICLES];
    int nb_communs = 0;
    
    for (int article = 0; article < rec->nb_articles; article++) {
        float eval1 = rec->matrice_evaluations[index_user1][article];
        float eval2 = rec->matrice_evaluations[index_user2][article];
        
        // Les deux utilisateurs ont évalué cet article
        if (eval1 > 0.0f && eval2 > 0.0f) {
            evaluations_user1[nb_communs] = eval1;
            evaluations_user2[nb_communs] = eval2;
            nb_communs++;
        }
    }
    
    // Pas assez de données communes
    if (nb_communs < 2) {
        return 0.0;
    }
    
    // Calculer les moyennes sur les articles communs uniquement
    double moyenne1 = 0.0, moyenne2 = 0.0;
    for (int i = 0; i < nb_communs; i++) {
        moyenne1 += evaluations_user1[i];
        moyenne2 += evaluations_user2[i];
    }
    moyenne1 /= nb_communs;
    moyenne2 /= nb_communs;
    
    // Calculer la corrélation de Pearson
    double numerateur = 0.0;
    double denominateur1 = 0.0;
    double denominateur2 = 0.0;
    
    for (int i = 0; i < nb_communs; i++) {
        double diff1 = evaluations_user1[i] - moyenne1;
        double diff2 = evaluations_user2[i] - moyenne2;
        
        numerateur += diff1 * diff2;
        denominateur1 += diff1 * diff1;
        denominateur2 += diff2 * diff2;
    }
    
    double denominateur = sqrt(denominateur1 * denominateur2);
    
    // Éviter la division par zéro
    if (denominateur == 0.0) {
        return 0.0;
    }
    
    return numerateur / denominateur;
}

// ========== TÂCHE 1: PEARSON(train-data) ==========

double** Pearson(const char* filename) {
    printf("=== DÉBUT CALCUL MATRICE PEARSON ===\n");
    
    // Initialiser le recommandeur depuis le fichier
    RecommandeurKNN* rec = initialiser_recommandeur_depuis_fichier(filename);
    if (!rec) {
        printf("✗ Erreur : Impossible d'initialiser le recommandeur\n");
        return NULL;
    }
    printf("DEBUG: Recommandeur initialisé, nb_users=%d, nb_articles=%d\n", rec->nb_users, rec->nb_articles);
    
    // Vérifier la validité des données
    if (!verifier_donnees_recommandeur(rec)) {
        printf("✗ Erreur : Données du recommandeur invalides\n");
        liberer_recommandeur(rec);
        return NULL;
    }
    printf("DEBUG: Données du recommandeur vérifiées\n");
    
    // Construire la matrice utilisateur-article
    if (!construire_matrice_evaluations(rec)) {
        printf("✗ Erreur : Impossible de construire la matrice d'évaluations\n");
        liberer_recommandeur(rec);
        return NULL;
    }
    printf("DEBUG: Matrice d'évaluations construite\n");
    
    // Calculer les moyennes des utilisateurs
    calculer_moyennes_utilisateurs(rec);
    printf("DEBUG: Moyennes utilisateurs calculées\n");
    
    // Calculer la matrice de similarité complète
    calculer_matrice_similarite_complete(rec);
    printf("DEBUG: Fonction similarité terminée\n");
    
    // Vérifier que le calcul s'est bien passé
    if (!rec->matrice_similarite) {
        printf("✗ Erreur : La matrice de similarité n'a pas été allouée\n");
        liberer_recommandeur(rec);
        return NULL;
    }
    printf("DEBUG: Matrice de similarité existe dans rec\n");
    
    // Allouer et copier la matrice de retour
    printf("DEBUG: Tentative d'allocation matrice retour %dx%d\n", rec->nb_users, rec->nb_users);
    double** matrice_retour = allouer_matrice_double(rec->nb_users, rec->nb_users);
    if (!matrice_retour) {
        printf("✗ Erreur : Allocation mémoire pour la matrice de retour\n");
        liberer_recommandeur(rec);
        return NULL;
    }
    printf("DEBUG: Matrice de retour allouée\n");
    
    // Copier les données
    for (int i = 0; i < rec->nb_users; i++) {
        for (int j = 0; j < rec->nb_users; j++) {
            matrice_retour[i][j] = rec->matrice_similarite[i][j];
        }
    }
    printf("DEBUG: Données copiées dans matrice retour\n");
    
    // Stocker globalement APRÈS avoir vérifié que tout est OK
    recommandeur_global = rec;
    printf("DEBUG: recommandeur_global assigné\n");
    
    printf("✓ Matrice de Pearson calculée avec succès (%dx%d)\n", rec->nb_users, rec->nb_users);
    printf("DEBUG: Retour de la fonction avec matrice non-NULL\n");
    
    return matrice_retour;
}

// Modification de la fonction calculer_matrice_similarite_complete
void calculer_matrice_similarite_complete(RecommandeurKNN* rec) {
    if (!rec) {
        printf("Erreur : Recommandeur NULL\n");
        return;
    }
    
    printf("Calcul de la matrice de similarité...\n");
    
    // CORRECTION : S'assurer que la matrice est allouée
    if (!rec->matrice_similarite) {
        printf("✗ Erreur : Matrice de similarité non allouée\n");
        return;
    }
    
    // Initialiser la matrice
    for (int i = 0; i < rec->nb_users; i++) {
        for (int j = 0; j < rec->nb_users; j++) {
            rec->matrice_similarite[i][j] = 0.0;
        }
    }
    
    // Calculer les similarités
    for (int i = 0; i < rec->nb_users; i++) {
        // Afficher le progrès
        if ((i + 1) % 10 == 0 || i == rec->nb_users - 1) {
            printf("Utilisateur %d/%d traité\n", i + 1, rec->nb_users);
        }
        
        for (int j = i; j < rec->nb_users; j++) {
            double correlation = correlation_pearson_entre_users(rec, i, j);
            
            // Vérifier si la corrélation est valide
            if (isnan(correlation) || isinf(correlation)) {
                correlation = 0.0;
            }
            
            rec->matrice_similarite[i][j] = correlation;
            rec->matrice_similarite[j][i] = correlation; // Matrice symétrique
        }
    }
    
    // CORRECTION : Marquer comme calculée
    rec->matrice_similarite_calculee = 1;
    printf("Matrice de similarité calculée avec succès\n");
}

//#####################################################################################
//----------------------- Foin Bloc pour le calcul de Person --------------------------
//#####################################################################################

// 8. Fonction de debug pour afficher quelques corrélations
void debug_afficher_correlations(RecommandeurKNN* rec) {
    printf("\n=== DEBUG : EXEMPLES DE CORRÉLATIONS ===\n");
    
    int nb_a_afficher = (rec->nb_users < 5) ? rec->nb_users : 5;
    
    for (int i = 0; i < nb_a_afficher; i++) {
        for (int j = i + 1; j < nb_a_afficher; j++) {
            printf("User[%d] - User[%d] : %.3f\n", 
                   rec->users[i].id_user, rec->users[j].id_user, 
                   rec->matrice_similarite[i][j]);
        }
    }
}

//========================================================================

// ========== TÂCHE 2: PREDICT(ui) ==========

float Predict(RecommandeurKNN* rec, unsigned int id_user, unsigned int id_article) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Erreur : Matrice de similarité non calculée\n");
        return -1.0;
    }
    
    int index_user = trouver_index_utilisateur(rec, id_user);
    int index_article = trouver_index_article(rec, id_article);
    
    if (index_user < 0 || index_article < 0) {
        printf("Erreur : Utilisateur %u ou article %u non trouvé\n", id_user, id_article);
        return -1.0;
    }
    
    // Vérifier si l'utilisateur a déjà noté cet article
    if (rec->matrice_evaluations[index_user][index_article] > 0) {
        return rec->matrice_evaluations[index_user][index_article]; // Déjà noté
    }
    
    // Trouver les voisins similaires
    Voisin* voisins = trouver_k_voisins_similaires(rec, id_user);
    if (!voisins) {
        return -1.0;
    }
    
    // Calculer la prédiction pondérée
    double prediction = calculer_prediction_ponderee(rec, voisins, id_article, id_user);
    
    free(voisins);
    return (float)prediction;
}


Voisin* trouver_k_voisins_similaires(RecommandeurKNN* rec, unsigned int id_user) {
    int index_user = trouver_index_utilisateur(rec, id_user);
    if (index_user < 0) return NULL;
    
    Voisin* tous_voisins = malloc(rec->nb_users * sizeof(Voisin));
    if (!tous_voisins) return NULL;
    
    int nb_voisins = 0;
    
    // Collecter tous les autres utilisateurs avec leur similarité
    for (int i = 0; i < rec->nb_users; i++) {
        if (i != index_user) {
            tous_voisins[nb_voisins].index_utilisateur = i;
            tous_voisins[nb_voisins].id_utilisateur = rec->users[i].id_user;
            tous_voisins[nb_voisins].similarite = rec->matrice_similarite[index_user][i];
            nb_voisins++;
        }
    }
    
    // Trier par similarité décroissante
    trier_voisins_par_similarite(tous_voisins, nb_voisins);
    
    // Retourner les k meilleurs voisins
    int k_effectif = (rec->k < nb_voisins) ? rec->k : nb_voisins;
    Voisin* k_voisins = malloc((k_effectif + 1) * sizeof(Voisin)); // +1 pour marquer la fin
    
    if (k_voisins) {
        for (int i = 0; i < k_effectif; i++) {
            k_voisins[i] = tous_voisins[i];
        }
        k_voisins[k_effectif].index_utilisateur = -1; // Marqueur de fin temporaire
        
        // FILTRAGE PAR SEUIL DE SIMILARITÉ
        int voisins_valides = 0;
        for (int i = 0; i < k_effectif; i++) {
            if (fabs(k_voisins[i].similarite) >= 0.1) { // Seuil minimal de similarité
                k_voisins[voisins_valides++] = k_voisins[i];
            }
        }
        k_voisins[voisins_valides].index_utilisateur = -1; // Nouveau marqueur de fin
        
        // Optionnel : réallouer pour économiser la mémoire si beaucoup de voisins filtrés
        if (voisins_valides < k_effectif / 2) {
            Voisin* k_voisins_reduit = realloc(k_voisins, (voisins_valides + 1) * sizeof(Voisin));
            if (k_voisins_reduit) {
                k_voisins = k_voisins_reduit;
            }
        }
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
    int voisins_valides = 0;

    for (int i = 0; voisins[i].index_utilisateur >= 0; i++) {
        int index_voisin = voisins[i].index_utilisateur;
        double similarite = voisins[i].similarite;
        
        if (rec->matrice_evaluations[index_voisin][index_article] > 0) {
            double note_voisin = rec->matrice_evaluations[index_voisin][index_article];
            double moyenne_voisin = rec->moyenne_utilisateur[index_voisin];
            
            // Utiliser la similarité brute (pas de fabs)
            numerateur += similarite * (note_voisin - moyenne_voisin);
            denominateur += similarite; 
            voisins_valides++;
        }
    }

    if (voisins_valides == 0 || fabs(denominateur) < 0.0001) {
        return moyenne_user; // Retourner la moyenne si pas de voisins valides
    }

    double prediction = moyenne_user + (numerateur / denominateur);
    
    // Borner la prédiction entre 1 et 5
    return fmax(1.0, fmin(5.0, prediction));
}

//========================================================================

// ========== TÂCHE 3: PREDICT-ALL(test-data) ==========

int Predict_all(RecommandeurKNN* rec, const char* test_filename, Prediction predictions[], int max_predictions) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Erreur : Recommandeur non initialisé ou matrice non calculée\n");
        return -1;
    }
    
    printf("=== DÉBUT PRÉDICTION SUR JEU DE TEST ===\n");
    
    // Charger les transactions de test
    Transaction transactions_test[MAX_USERS * MAX_ARTICLES];
    int nb_transactions_test = compter_transactions(test_filename, transactions_test, MAX_USERS * MAX_ARTICLES);
    
    if (nb_transactions_test <= 0) {
        printf("Erreur : Impossible de charger les données de test depuis %s\n", test_filename);
        return -1;
    }
    
    printf("Transactions de test chargées : %d\n", nb_transactions_test);
    
    int nb_predictions = 0;
    int nb_echecs = 0;
    
    // Faire des prédictions pour chaque transaction de test
    for (int i = 0; i < nb_transactions_test && nb_predictions < max_predictions; i++) {
        unsigned int id_user = transactions_test[i].id_user;
        unsigned int id_article = transactions_test[i].id_article;
        float note_reelle = transactions_test[i].evaluation;
        
        float note_predite = Predict(rec, id_user, id_article);
        
        if (note_predite >= 0) {
            predictions[nb_predictions].id_user = id_user;
            predictions[nb_predictions].id_article = id_article;
            predictions[nb_predictions].note_predite = note_predite;
            predictions[nb_predictions].note_reelle = note_reelle;
            predictions[nb_predictions].confiance = 0.5; // À améliorer
            
            nb_predictions++;
        } else {
            nb_echecs++;
        }
        
        // Affichage du progrès
        if ((i + 1) % 100 == 0) {
            printf("Prédictions effectuées : %d/%d (échecs: %d)\n", 
                   nb_predictions, i + 1, nb_echecs);
        }
    }
    
    printf("=== FIN PRÉDICTION ===\n");
    printf("Total prédictions réussies : %d\n", nb_predictions);
    printf("Total échecs : %d\n", nb_echecs);
    
    return nb_predictions;
}

// ========== FONCTIONS D'ÉVALUATION ==========

double calculer_rmse(Prediction predictions[], int nb_predictions) {
    if (nb_predictions <= 0) return -1.0;
    
    double somme_carres = 0.0;
    
    for (int i = 0; i < nb_predictions; i++) {
        double erreur = predictions[i].note_predite - predictions[i].note_reelle;
        somme_carres += erreur * erreur;
    }
    
    return sqrt(somme_carres / nb_predictions);
}

double calculer_mae(Prediction predictions[], int nb_predictions) {
    if (nb_predictions <= 0) return -1.0;
    
    double somme_absolue = 0.0;
    
    for (int i = 0; i < nb_predictions; i++) {
        double erreur = fabs(predictions[i].note_predite - predictions[i].note_reelle);
        somme_absolue += erreur;
    }
    
    return somme_absolue / nb_predictions;
}

double calculer_precision_seuil(Prediction predictions[], int nb_predictions, float seuil) {
    if (nb_predictions <= 0) return -1.0;
    
    int predictions_correctes = 0;
    
    for (int i = 0; i < nb_predictions; i++) {
        double erreur = fabs(predictions[i].note_predite - predictions[i].note_reelle);
        if (erreur <= seuil) {
            predictions_correctes++;
        }
    }
    
    return (double)predictions_correctes / nb_predictions;
}

void afficher_metriques_evaluation(Prediction predictions[], int nb_predictions) {
    if (nb_predictions <= 0) {
        printf("Aucune prédiction à évaluer\n");
        return;
    }
    
    double rmse = calculer_rmse(predictions, nb_predictions);
    double mae = calculer_mae(predictions, nb_predictions);
    double precision_05 = calculer_precision_seuil(predictions, nb_predictions, 0.5);
    double precision_10 = calculer_precision_seuil(predictions, nb_predictions, 1.0);
    
    printf("\n=== MÉTRIQUES D'ÉVALUATION ===\n");
    printf("Nombre de prédictions : %d\n", nb_predictions);
    printf("RMSE (Root Mean Square Error) : %.4f\n", rmse);
    printf("MAE (Mean Absolute Error) : %.4f\n", mae);
    printf("Précision (seuil 0.5) : %.2f%%\n", precision_05 * 100);
    printf("Précision (seuil 1.0) : %.2f%%\n", precision_10 * 100);
    
    // Analyser la distribution des erreurs
    int erreurs[6] = {0}; // 0-0.5, 0.5-1, 1-1.5, 1.5-2, 2-2.5, >2.5
    
    for (int i = 0; i < nb_predictions; i++) {
        double erreur = fabs(predictions[i].note_predite - predictions[i].note_reelle);
        int indice = (int)(erreur / 0.5);
        if (indice > 5) indice = 5;
        erreurs[indice]++;
    }
    
    printf("\nDistribution des erreurs absolues :\n");
    printf("0.0-0.5 : %d (%.1f%%)\n", erreurs[0], 100.0 * erreurs[0] / nb_predictions);
    printf("0.5-1.0 : %d (%.1f%%)\n", erreurs[1], 100.0 * erreurs[1] / nb_predictions);
    printf("1.0-1.5 : %d (%.1f%%)\n", erreurs[2], 100.0 * erreurs[2] / nb_predictions);
    printf("1.5-2.0 : %d (%.1f%%)\n", erreurs[3], 100.0 * erreurs[3] / nb_predictions);
    printf("2.0-2.5 : %d (%.1f%%)\n", erreurs[4], 100.0 * erreurs[4] / nb_predictions);
    printf(">2.5    : %d (%.1f%%)\n", erreurs[5], 100.0 * erreurs[5] / nb_predictions);
}

// ========== FONCTIONS UTILITAIRES ==========

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

void liberer_matrice_similarite(double** matrice, int nb_users) {
    if (matrice) {
        for (int i = 0; i < nb_users; i++) {
            free(matrice[i]);
        }
        free(matrice);
    }
}

double** allouer_matrice_double(int lignes, int colonnes) {
    double** matrice = malloc(lignes * sizeof(double*));
    if (!matrice) return NULL;
    
    for (int i = 0; i < lignes; i++) {
        matrice[i] = malloc(colonnes * sizeof(double));
        if (!matrice[i]) {
            for (int j = 0; j < i; j++) {
                free(matrice[j]);
            }
            free(matrice);
            return NULL;
        }
    }
    
    return matrice;
}

//----------------------------------------------------------------------
int trouver_index_utilisateur(RecommandeurKNN* rec, unsigned int id_user) {
    if (!rec) return -1;
    
    for (int i = 0; i < rec->nb_users; i++) {
        if (rec->users[i].id_user == id_user) {
            return i;
        }
    }
    return -1;
}

//----------------------------------------------------------------------
int trouver_index_article(RecommandeurKNN* rec, unsigned int id_article) {
    if (!rec) return -1;
    
    for (int i = 0; i < rec->nb_articles; i++) {
        if (rec->articles[i].id_article == id_article) {
            return i;
        }
    }
    return -1;
}


//----------------------------------------------------------------------
// 6. Fonction pour vérifier la validité des données

int verifier_donnees_recommandeur(RecommandeurKNN* rec) {
    if (!rec) {
        printf("Erreur : Recommandeur NULL\n");
        return 0;
    }
    
    if (rec->nb_users <= 0 || rec->nb_articles <= 0 || rec->nb_transactions <= 0) {
        printf("Erreur : Données insuffisantes (users:%d, articles:%d, trans:%d)\n",
               rec->nb_users, rec->nb_articles, rec->nb_transactions);
        return 0;
    }
    
    // Vérifier que chaque transaction correspond à des utilisateurs/articles valides
    int transactions_valides = 0;
    for (int i = 0; i < rec->nb_transactions; i++) {
        int user_idx = trouver_index_utilisateur(rec, rec->transactions[i].id_user);
        int article_idx = trouver_index_article(rec, rec->transactions[i].id_article);
        
        if (user_idx >= 0 && article_idx >= 0) {
            transactions_valides++;
        }
    }
    
    printf("Transactions valides : %d/%d\n", transactions_valides, rec->nb_transactions);
    
    if (transactions_valides < rec->nb_transactions * 0.9) { // Au moins 90% valides
        printf("Attention : Trop de transactions invalides\n");
        return 0;
    }
    
    return 1;
}

//----------------------------------------------------------------------

float obtenir_evaluation(RecommandeurKNN* rec, int index_user, int index_article) {
    if (index_user >= 0 && index_user < rec->nb_users && 
        index_article >= 0 && index_article < rec->nb_articles) {
        return rec->matrice_evaluations[index_user][index_article];
    }
    return 0.0;
}

int utilisateur_a_note_article(RecommandeurKNN* rec, int index_user, int index_article) {
    if (index_user >= 0 && index_user < rec->nb_users && 
        index_article >= 0 && index_article < rec->nb_articles) {
        return rec->matrice_evaluations[index_user][index_article] > 0;
    }
    return 0;
}

void trier_voisins_par_similarite(Voisin* voisins, int nb_voisins) {
    // Tri à bulles par similarité décroissante
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

int comparer_voisins_par_similarite(const void* a, const void* b) {
    const Voisin* voisin_a = (const Voisin*)a;
    const Voisin* voisin_b = (const Voisin*)b;
    
    if (voisin_a->similarite > voisin_b->similarite) return -1;
    if (voisin_a->similarite < voisin_b->similarite) return 1;
    return 0;
}

// ========== FONCTIONS D'AFFICHAGE ==========

void afficher_matrice_similarite(double** matrice, RecommandeurKNN* rec) {
    if (rec == NULL || matrice == NULL) {
        printf("Erreur : pointeur NULL fourni.\n");
        return;
    }

    int nb_users = rec->nb_users;
    int limite = (nb_users > 10) ? 10 : nb_users; // Limiter l'affichage à 10x10

    printf("=== Matrice de similarité (%d utilisateurs, affichage %dx%d) ===\n\n", 
           nb_users, limite, limite);

    // En-têtes de colonnes
    printf("       ");
    for (int j = 0; j < limite; j++) {
        printf("   U%-2d  ", j);
    }
    printf("\n");

    // Ligne supérieure
    printf("      +");
    for (int j = 0; j < limite; j++) {
        printf("--------+");
    }
    printf("\n");

    // Lignes de données
    for (int i = 0; i < limite; i++) {
        printf("  U%-2d |", i);  // En-tête de ligne
        for (int j = 0; j < limite; j++) {
            printf(" %6.3f |", matrice[i][j]);
        }
        printf("\n");

        // Ligne de séparation
        printf("      +");
        for (int j = 0; j < limite; j++) {
            printf("--------+");
        }
        printf("\n");
    }
    
    if (limite < nb_users) {
        printf("... (matrice tronquée pour l'affichage)\n");
    }
}

void afficher_voisins(Voisin* voisins, int k) {
    printf("\n=== TOP %d VOISINS ===\n", k);
    printf("Index\tID\tSimilarité\n");
    printf("-------------------------\n");
    
    for (int i = 0; i < k && voisins[i].index_utilisateur >= 0; i++) {
        printf("%d\t%u\t%.4f\n", 
               voisins[i].index_utilisateur,
               voisins[i].id_utilisateur,
               voisins[i].similarite);
    }
}

void afficher_predictions(Prediction predictions[], int nb_predictions) {
    int limite = (nb_predictions > 20) ? 20 : nb_predictions;
    
    printf("\n=== ÉCHANTILLON DE PRÉDICTIONS (%d/%d) ===\n", limite, nb_predictions);
    printf("User\tArticle\tPrédit\tRéel\tErreur\tConfiance\n");
    printf("--------------------------------------------------\n");
    
    for (int i = 0; i < limite; i++) {
        float erreur = predictions[i].note_predite - predictions[i].note_reelle;
        printf("%u\t%u\t%.2f\t%.2f\t%.2f\t%.2f\n", 
               predictions[i].id_user, 
               predictions[i].id_article,
               predictions[i].note_predite, 
               predictions[i].note_reelle, 
               erreur,
               predictions[i].confiance);
    }
}

// ========== FONCTIONS MANQUANTES À AJOUTER AU FICHIER reco_KNN.c ==========


void afficher_stats_recommandeur(RecommandeurKNN* rec) {
    if (rec == NULL) {
        printf("Erreur : le pointeur vers le recommandeur est NULL.\n");
        return;
    }

    printf("=== STATISTIQUES DU RECOMMANDEUR KNN ===\n");
    
    // Données générales
    printf("Nombre d'utilisateurs       : %d\n", rec->nb_users);
    printf("Nombre d'articles           : %d\n", rec->nb_articles);
    printf("Nombre de transactions      : %d\n", rec->nb_transactions);
    
    // Paramètre KNN
    printf("Valeur de k (voisins)       : %d\n", rec->k);

    // État des données
    printf("Données chargées            : %s\n", rec->donnees_chargees ? "Oui" : "Non");
    printf("Similarité calculée         : %s\n", rec->matrice_similarite_calculee ? "Oui" : "Non");
    
    // Statistiques sur les évaluations
    if (rec->donnees_chargees) {
        int total_evaluations = 0;
        double somme_moyennes = 0.0;
        
        for (int i = 0; i < rec->nb_users; i++) {
            total_evaluations += rec->nb_eval_utilisateur[i];
            somme_moyennes += rec->moyenne_utilisateur[i];
        }
        
        printf("Total évaluations           : %d\n", total_evaluations);
        printf("Moyenne générale            : %.2f\n", 
               rec->nb_users > 0 ? somme_moyennes / rec->nb_users : 0.0);
        
        // Densité de la matrice
        int elements_possibles = rec->nb_users * rec->nb_articles;
        double densite = elements_possibles > 0 ? 
                        (double)total_evaluations / elements_possibles * 100.0 : 0.0;
        printf("Densité de la matrice       : %.2f%%\n", densite);
    }
    
    printf("==========================================\n");
}

// Fonction de sauvegarde des prédictions
void sauvegarder_predictions(const char* filename, Prediction predictions[], int nb_predictions) {
    FILE* fichier = fopen(filename, "w");
    if (!fichier) {
        printf("Erreur : Impossible de créer le fichier %s\n", filename);
        return;
    }
    
    // En-tête CSV
    fprintf(fichier, "id_user,id_article,note_predite,note_reelle,erreur_absolue,confiance\n");
    
    // Données
    for (int i = 0; i < nb_predictions; i++) {
        float erreur_absolue = fabs(predictions[i].note_predite - predictions[i].note_reelle);
        fprintf(fichier, "%u,%u,%.3f,%.3f,%.3f,%.3f\n",
                predictions[i].id_user,
                predictions[i].id_article,
                predictions[i].note_predite,
                predictions[i].note_reelle,
                erreur_absolue,
                predictions[i].confiance);
    }
    
    fclose(fichier);
    printf("Prédictions sauvegardées dans %s\n", filename);
}

// Fonction de test des prédictions

void tester_predictions(RecommandeurKNN* rec) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Erreur : Recommandeur non initialisé\n");
        return;
    }
    
    printf("\n=== TEST DES PRÉDICTIONS ===\n");
    
    // Test sur quelques utilisateurs/articles connus
    unsigned int users_test[] = {1, 2, 3, 4, 5};
    unsigned int articles_test[] = {10, 20, 30, 40, 50};
    size_t nb_users_test = sizeof(users_test) / sizeof(users_test[0]);
    size_t nb_articles_test = sizeof(articles_test) / sizeof(articles_test[0]);
    int nb_tests = 5;
    
    for (int i = 0; i < nb_tests && i < rec->nb_users; i++) {
        for (int j = 0; j < nb_tests && j < rec->nb_articles; j++) {
            unsigned int id_user = ((size_t)i < nb_users_test) ? 
                                  users_test[i] : rec->users[i].id_user;
            unsigned int id_article = ((size_t)j < nb_articles_test) ? 
                                     articles_test[j] : rec->articles[j].id_article;
            
            float prediction = Predict(rec, id_user, id_article);
            
            printf("User %u, Article %u -> Prédiction: %.2f\n", 
                   id_user, id_article, prediction);
        }
    }
}

// Fonction d'affichage alternative pour la matrice de similarité avec limite

void afficher_matrice_similarite_limitee(RecommandeurKNN* rec, int limite) {
    if (!rec || !rec->matrice_similarite_calculee) {
        printf("Erreur : Matrice de similarité non disponible\n");
        return;
    }
    
    int nb_users = rec->nb_users;
    if (limite > nb_users) limite = nb_users;
    
    printf("=== Matrice de similarité (%d utilisateurs, affichage %dx%d) ===\n\n", 
           nb_users, limite, limite);

    // En-têtes de colonnes
    printf("       ");
    for (int j = 0; j < limite; j++) {
        printf("   U%-2d  ", j);
    }
    printf("\n");

    // Ligne supérieure
    printf("      +");
    for (int j = 0; j < limite; j++) {
        printf("--------+");
    }
    printf("\n");

    // Lignes de données
    for (int i = 0; i < limite; i++) {
        printf("  U%-2d |", i);  // En-tête de ligne
        for (int j = 0; j < limite; j++) {
            printf(" %6.3f |", rec->matrice_similarite[i][j]);
        }
        printf("\n");

        // Ligne de séparation
        printf("      +");
        for (int j = 0; j < limite; j++) {
            printf("--------+");
        }
        printf("\n");
    }
    
    if (limite < nb_users) {
        printf("... (matrice tronquée pour l'affichage)\n");
    }
}


//###############################################################################
//----------------------------------------------------------------------------
// Cette fonction me permet d'automatiser toutes les étapes afin de juste donner 
// la recommandation aux clients KNN

char* traiter_recommandation_knn(int id_user, int nb_reco) {

    static char buffer[2048];
    buffer[0] = '\0';
    
    static RecommandeurKNN* rec = NULL;
    static int knn_initialise = 0;
    
    // Initialisation du recommandeur KNN (une seule fois)
    if (!knn_initialise) {
        rec = initialiser_recommandeur_depuis_fichier("data/KNN_TRAIN/Train.txt");
        if (!rec) {
            snprintf(buffer, 2048, "Erreur : Impossible d'initialiser le recommandeur KNN.\n");
            return buffer;
        }
        
        // Vérifier si les données sont bien chargées
        printf("Données chargées - Users: %d, Articles: %d, Transactions: %d\n", 
               rec->nb_users, rec->nb_articles, rec->nb_transactions);
        
        // Construction de la matrice d'évaluations
        int result_matrice = construire_matrice_evaluations(rec);
        printf("Résultat construction matrice: %d\n", result_matrice);
        
        // Vérifier si la construction a échoué (ta fonction retourne 1 en cas de succès)
        if (result_matrice != 1) {
            snprintf(buffer, 2048, "Erreur : Impossible de construire la matrice d'évaluations.\n");
            return buffer;
        }
        
        // Vérification supplémentaire des données
        if (rec->nb_users == 0 || rec->nb_articles == 0) {
            snprintf(buffer, 2048, "Erreur : Données insuffisantes pour KNN.\n");
            return buffer;
        }
        
        // Calcul des moyennes des utilisateurs
        printf("Calcul des moyennes utilisateurs...\n");
        calculer_moyennes_utilisateurs(rec);
        
        // Calcul de la matrice de similarité complète
        printf("Calcul de la matrice de similarité...\n");
        calculer_matrice_similarite_complete(rec);
        
        printf("Initialisation KNN terminée avec succès!\n");
        knn_initialise = 1;
    }
    
    // Appel de la fonction buffer pour les recommandations
    recommander_articles_knn_buffer(id_user, nb_reco, rec, buffer);
    
    return buffer;
}


//###############################################################################

void recommander_articles_knn_buffer(int id_user, int nb_reco, RecommandeurKNN* rec, char* buffer) {
    // Initialisation du buffer
    snprintf(buffer, 2048, "Recommandations KNN pour l'utilisateur %d:\n", id_user);
    int offset = strlen(buffer);
    
    // Vérifications de sécurité
    if (!rec) {
        snprintf(buffer + offset, 2048 - offset, "Erreur : Recommandeur non initialisé.\n");
        return;
    }
    
    if (rec->nb_users == 0 || rec->nb_articles == 0) {
        snprintf(buffer + offset, 2048 - offset, "Erreur : Aucune donnée disponible.\n");
        return;
    }
    
    // Vérification de l'existence de l'utilisateur
    int index_user = trouver_index_utilisateur(rec, id_user);
    if (index_user == -1) {
        snprintf(buffer + offset, 2048 - offset, "Erreur : Utilisateur %d inconnu.\n", id_user);
        
        // Debug : afficher les utilisateurs disponibles (premiers 10)
        offset += snprintf(buffer + offset, 2048 - offset, "Utilisateurs disponibles : ");
        int nb_a_afficher = (rec->nb_users > 10) ? 10 : rec->nb_users;
        for (int i = 0; i < nb_a_afficher; i++) {
            offset += snprintf(buffer + offset, 2048 - offset, "%u ", rec->users[i].id_user);
        }
        if (rec->nb_users > 10) {
            offset += snprintf(buffer + offset, 2048 - offset, "...");
        }
        offset += snprintf(buffer + offset, 2048 - offset, "\n");
        return;
    }
    
    // Créer un tableau pour stocker les prédictions
    Prediction* predictions = malloc(rec->nb_articles * sizeof(Prediction));
    if (!predictions) {
        snprintf(buffer + offset, 2048 - offset, "Erreur : Allocation mémoire impossible.\n");
        return;
    }
    
    int nb_predictions = 0;
    int articles_deja_notes = 0;
    
    // Générer les prédictions pour tous les articles non notés
    for (int i = 0; i < rec->nb_articles; i++) {
        unsigned int id_article = rec->articles[i].id_article;
        
        // Vérifier si l'utilisateur a déjà noté cet article
        if (utilisateur_a_note_article(rec, index_user, i)) {
            articles_deja_notes++;
            continue;
        }
        
        float prediction = Predict(rec, id_user, id_article);
        
        if (prediction > 0) { // Si la prédiction est valide
            predictions[nb_predictions].id_user = id_user;
            predictions[nb_predictions].id_article = id_article;
            predictions[nb_predictions].note_predite = prediction;
            predictions[nb_predictions].note_reelle = 0; // Non applicable ici
            predictions[nb_predictions].confiance = 1.0; // Simplifié
            nb_predictions++;
        }
    }
    
    offset += snprintf(buffer + offset, 2048 - offset, 
                      "Articles déjà notés: %d, Prédictions calculées: %d\n", 
                      articles_deja_notes, nb_predictions);
    
    if (nb_predictions == 0) {
        snprintf(buffer + offset, 2048 - offset, "Aucune prédiction KNN disponible pour cet utilisateur.\n");
        free(predictions);
        return;
    }
    
    // Trier les prédictions par note décroissante
    qsort(predictions, nb_predictions, sizeof(Prediction), comparer_predictions);
    
    // Afficher les meilleures recommandations
    int count = 0;
    for (int i = 0; i < nb_predictions && count < nb_reco; i++) {
        offset += snprintf(buffer + offset, 2048 - offset,
                          "%d. Article %u (Note prédite: %.2f)\n",
                          count + 1, predictions[i].id_article, predictions[i].note_predite);
        count++;
    }
    
    if (count == 0) {
        snprintf(buffer + offset, 2048 - offset, "Aucune recommandation KNN de qualité trouvée.\n");
    }
    
    free(predictions);
}

//###############################################################################

// Fonction de comparaison pour trier les prédictions par note décroissante
int comparer_predictions(const void* a, const void* b) {
    const Prediction* pred_a = (const Prediction*)a;
    const Prediction* pred_b = (const Prediction*)b;
    
    if (pred_a->note_predite > pred_b->note_predite) return -1;
    if (pred_a->note_predite < pred_b->note_predite) return 1;
    return 0;
}

//###############################################################################
// Fonction pour nettoyer les ressources KNN (optionnelle)

void nettoyer_knn() {
    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }
}

