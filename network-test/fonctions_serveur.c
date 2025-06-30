#include "fonctions_serveur.h"
#include <pthread.h>  

// Variables globales

RecommandeurKNN* recommandeur_global = NULL;
double** matrice_similarite = NULL;
Prediction predictions[10000];
int nb_predictions = 0;

// Mutex global
pthread_mutex_t mutex_recommandeur = PTHREAD_MUTEX_INITIALIZER;

void envoyer_reponse(int client_sock, const char* message) {
    send(client_sock, message, strlen(message), 0);
}

void handle_pearson(int client_sock, char* fichier_train) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande PEARSON avec fichier: %s\n", fichier_train);

    if (matrice_similarite && recommandeur_global) {
        liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
        matrice_similarite = NULL;
    }

    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }

    matrice_similarite = Pearson(fichier_train);

    if (matrice_similarite && recommandeur_global) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer),
                "OK: Matrice de Pearson calculée avec succès\n"
                "Utilisateurs: %d\n"
                "Articles: %d\n"
                "Évaluations: %d\n",
                recommandeur_global->nb_users,
                recommandeur_global->nb_articles,
                recommandeur_global->nb_transactions);
        envoyer_reponse(client_sock, buffer);
        printf("Matrice Pearson calculée: %d utilisateurs\n", recommandeur_global->nb_users);
    } else {
        envoyer_reponse(client_sock, "ERREUR: Échec du calcul de la matrice de Pearson\n");
        printf("Erreur: Échec du calcul de la matrice de Pearson\n");
    }
    pthread_mutex_unlock(&mutex_recommandeur);
}

void handle_predict(int client_sock, unsigned int user, unsigned int item) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande PREDICT pour utilisateur %u et article %u\n", user, item);

    char buffer[512];
    if (!recommandeur_global || !matrice_similarite) {
        envoyer_reponse(client_sock, "ERREUR: Matrice non calculée. Utilisez PEARSON d'abord.\n");
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    int user_idx = trouver_index_utilisateur(recommandeur_global, user);
    int item_idx = trouver_index_article(recommandeur_global, item);

    if (user_idx == -1) {
        snprintf(buffer, sizeof(buffer), "ERREUR: Utilisateur %u non trouvé\n", user);
        envoyer_reponse(client_sock, buffer);
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    if (item_idx == -1) {
        snprintf(buffer, sizeof(buffer), "ERREUR: Article %u non trouvé\n", item);
        envoyer_reponse(client_sock, buffer);
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    float note_predite = Predict(recommandeur_global, user, item);
    float note_reelle = recommandeur_global->matrice_evaluations[user_idx][item_idx];

    snprintf(buffer, sizeof(buffer),
            "PREDICTION:\n"
            "User ID: %u\n"
            "Item ID: %u\n"
            "Prédiction: %.3f\n"
            "Note réelle: %.3f\n",
            user, item, note_predite, note_reelle > 0 ? note_reelle : 0.0);

    envoyer_reponse(client_sock, buffer);
    printf("Prédiction calculée: %.3f pour utilisateur %u et article %u\n", note_predite, user, item);
    pthread_mutex_unlock(&mutex_recommandeur);
}

void handle_predict_all(int client_sock, char* fichier_test) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande PREDICT_ALL avec fichier: %s\n", fichier_test);

    if (!recommandeur_global || !matrice_similarite) {
        envoyer_reponse(client_sock, "ERREUR: Calculez d'abord la matrice de Pearson\n");
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    nb_predictions = Predict_all(recommandeur_global, fichier_test, predictions, 10000);

    if (nb_predictions > 0) {
        char buffer[BUFFER_SIZE];
        int offset = snprintf(buffer, sizeof(buffer),
                "OK: Prédictions terminées avec succès\n"
                "Nombre de prédictions: %d\n"
                "Quelques exemples:\n", nb_predictions);

        int nb_exemples = (nb_predictions < 3) ? nb_predictions : 3;
        for (int i = 0; i < nb_exemples; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                    "  User %u, Item %u -> Prédiction: %.3f, Réel: %.3f\n",
                    predictions[i].id_user, predictions[i].id_article,
                    predictions[i].note_predite, predictions[i].note_reelle);
        }

        envoyer_reponse(client_sock, buffer);
        printf("Prédictions terminées: %d prédictions effectuées\n", nb_predictions);
    } else {
        envoyer_reponse(client_sock, "ERREUR: Aucune prédiction n'a pu être effectuée\n");
        printf("Erreur: Aucune prédiction effectuée\n");
    }

    pthread_mutex_unlock(&mutex_recommandeur);
}

void handle_evaluate(int client_sock) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande EVALUATE\n");

    char buffer[512];
    if (nb_predictions == 0) {
        envoyer_reponse(client_sock, "ERREUR: Pas de prédictions à évaluer. Utilisez PREDICT_ALL d'abord.\n");
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    double mae = calculer_mae(predictions, nb_predictions);
    double rmse = calculer_rmse(predictions, nb_predictions);

    snprintf(buffer, sizeof(buffer),
             "EVALUATION:\n"
             "Nombre de prédictions évaluées: %d\n"
             "MAE: %.3f\n"
             "RMSE: %.3f\n",
             nb_predictions, mae, rmse);

    envoyer_reponse(client_sock, buffer);
    printf("Évaluation terminée: MAE=%.3f, RMSE=%.3f\n", mae, rmse);
    pthread_mutex_unlock(&mutex_recommandeur);
}

void handle_save(int client_sock) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande SAVE\n");

    if (nb_predictions == 0) {
        envoyer_reponse(client_sock, "ERREUR: Pas de prédictions à sauvegarder\n");
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    const char* chemin_fichier = "data/KNN_TRAIN/resultats_predictions.txt";
    sauvegarder_predictions(chemin_fichier, predictions, nb_predictions);

    char buffer[BUFFER_SIZE];
    int offset = snprintf(buffer, sizeof(buffer),
                "✓ Prédictions sauvegardées dans : %s\n"
                "=== APERÇU DES PRÉDICTIONS ===\n"
                "USER;ITEM;PREDICTION;NOTE_REELLE\n", chemin_fichier);

    int nb_a_afficher = (nb_predictions < 20) ? nb_predictions : 20;
    for (int i = 0; i < nb_a_afficher && offset < BUFFER_SIZE - 100; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "%u;%u;%.2f;%.2f\n",
                           predictions[i].id_user,
                           predictions[i].id_article,
                           predictions[i].note_predite,
                           predictions[i].note_reelle);
    }

    if (nb_predictions > 20 && offset < BUFFER_SIZE - 100) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "... et %d autres prédictions\n", nb_predictions - 20);
    }

    envoyer_reponse(client_sock, buffer);
    printf("✓ Fichier sauvegardé et résumé envoyé (%d prédictions)\n", nb_predictions);
    pthread_mutex_unlock(&mutex_recommandeur);
}

void handle_stats(int client_sock) {
    pthread_mutex_lock(&mutex_recommandeur);
    printf("Traitement commande STATS\n");

    char buffer[512];
    if (!recommandeur_global) {
        envoyer_reponse(client_sock, "ERREUR: Aucun recommandeur chargé. Utilisez PEARSON d'abord.\n");
        pthread_mutex_unlock(&mutex_recommandeur);
        return;
    }

    snprintf(buffer, sizeof(buffer),
             "STATISTIQUES:\n"
             "Utilisateurs: %d\n"
             "Articles: %d\n"
             "Évaluations: %d\n"
             "Matrice de similarité: %s\n"
             "Prédictions disponibles: %d\n",
             recommandeur_global->nb_users,
             recommandeur_global->nb_articles,
             recommandeur_global->nb_transactions,
             matrice_similarite ? "Calculée" : "Non calculée",
             nb_predictions);

    envoyer_reponse(client_sock, buffer);
    printf("Statistiques envoyées\n");
    pthread_mutex_unlock(&mutex_recommandeur);
}

void process_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    printf("Nouveau client connecté\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            printf("Client déconnecté\n");
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = 0;
        printf("Commande reçue: %s\n", buffer);

        char cmd[64], param1[256], param2[256];
        int n = sscanf(buffer, "%63s %255s %255s", cmd, param1, param2);

        if (strcasecmp(cmd, "PEARSON") == 0 && n >= 2) {
            handle_pearson(client_sock, param1);
        } else if (strcasecmp(cmd, "PREDICT") == 0 && n >= 3) {
            handle_predict(client_sock, atoi(param1), atoi(param2));
        } else if (strcasecmp(cmd, "PREDICT_ALL") == 0 && n >= 2) {
            handle_predict_all(client_sock, param1);
        } else if (strcasecmp(cmd, "EVALUATE") == 0) {
            handle_evaluate(client_sock);
        } else if (strcasecmp(cmd, "SAVE") == 0) {
            handle_save(client_sock);
        } else if (strcasecmp(cmd, "STATS") == 0) {
            handle_stats(client_sock);
        } else if (strcasecmp(cmd, "QUIT") == 0) {
            envoyer_reponse(client_sock, "Au revoir !\n");
            printf("Client a quitté\n");
            break;
        } else {
            envoyer_reponse(client_sock, "ERREUR: Commande non reconnue\n");
        }
    }

    close(client_sock);
}

