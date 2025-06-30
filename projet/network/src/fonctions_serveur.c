#include "fonctions_serveur.h"

// Définitions des variables globales

RecommandeurKNN* recommandeur_global = NULL;

double** matrice_similarite = NULL;
Prediction predictions[10000];
int nb_predictions = 0;

void envoyer_reponse(int client_sock, const char* message) {
    send(client_sock, message, strlen(message), 0);
}

void handle_pearson(int client_sock, char* fichier_train) {
    printf("Traitement commande PEARSON avec fichier: %s\n", fichier_train);
    
    // Libérer les ressources précédentes
    if (matrice_similarite && recommandeur_global) {
        liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
        matrice_similarite = NULL;
    }
    
    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }

    // Calculer la nouvelle matrice de Pearson
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
}

void handle_predict(int client_sock, unsigned int user, unsigned int item) {
    printf("Traitement commande PREDICT pour utilisateur %u et article %u\n", user, item);
    
    char buffer[512];
    if (!recommandeur_global || !matrice_similarite) {
        envoyer_reponse(client_sock, "ERREUR: Matrice non calculée. Utilisez PEARSON d'abord.\n");
        return;
    }
    
    // Vérifier que les IDs existent
    int user_idx = trouver_index_utilisateur(recommandeur_global, user);
    int item_idx = trouver_index_article(recommandeur_global, item);
    
    if (user_idx == -1) {
        snprintf(buffer, sizeof(buffer), "ERREUR: Utilisateur %u non trouvé\n", user);
        envoyer_reponse(client_sock, buffer);
        return;
    }
    
    if (item_idx == -1) {
        snprintf(buffer, sizeof(buffer), "ERREUR: Article %u non trouvé\n", item);
        envoyer_reponse(client_sock, buffer);
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
}

void handle_predict_all(int client_sock, char* fichier_test) {
    printf("Traitement commande PREDICT_ALL avec fichier: %s\n", fichier_test);
    
    if (!recommandeur_global || !matrice_similarite) {
        envoyer_reponse(client_sock, "ERREUR: Calculez d'abord la matrice de Pearson\n");
        return;
    }
    
    nb_predictions = Predict_all(recommandeur_global, fichier_test, predictions, 10000);
    
    if (nb_predictions > 0) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), 
                "OK: Prédictions terminées avec succès\n"
                "Nombre de prédictions: %d\n"
                "Quelques exemples:\n", nb_predictions);
        
        // Ajouter quelques exemples
        int nb_exemples = (nb_predictions < 3) ? nb_predictions : 3;
        for (int i = 0; i < nb_exemples; i++) {
            char exemple[128];
            snprintf(exemple, sizeof(exemple), 
                    "  User %u, Item %u -> Prédiction: %.3f, Réel: %.3f\n",
                    predictions[i].id_user, predictions[i].id_article,
                    predictions[i].note_predite, predictions[i].note_reelle);
            strcat(buffer, exemple);
        }
        
        envoyer_reponse(client_sock, buffer);
        printf("Prédictions terminées: %d prédictions effectuées\n", nb_predictions);
    } else {
        envoyer_reponse(client_sock, "ERREUR: Aucune prédiction n'a pu être effectuée\n");
        printf("Erreur: Aucune prédiction effectuée\n");
    }
}

void handle_evaluate(int client_sock) {
    printf("Traitement commande EVALUATE\n");
    
    char buffer[512];
    if (nb_predictions == 0) {
        envoyer_reponse(client_sock, "ERREUR: Pas de prédictions à évaluer. Utilisez PREDICT_ALL d'abord.\n");
        return;
    }
    
    double mae = calculer_mae(predictions, nb_predictions);
    double rmse = calculer_rmse(predictions, nb_predictions);
    
    snprintf(buffer, sizeof(buffer),
             "EVALUATION:\n"
             "Nombre de prédictions évaluées: %d\n"
             "MAE (Mean Absolute Error): %.3f\n"
             "RMSE (Root Mean Square Error): %.3f\n", 
             nb_predictions, mae, rmse);
    
    envoyer_reponse(client_sock, buffer);
    printf("Évaluation terminée: MAE=%.3f, RMSE=%.3f\n", mae, rmse);
}

void handle_save(int client_sock) {
    printf("Traitement commande SAVE\n");

    if (nb_predictions == 0) {
        envoyer_reponse(client_sock, "ERREUR: Pas de prédictions à sauvegarder\n");
        return;
    }

    // 1. Sauvegarde sur le serveur
    const char* chemin_fichier = "data/KNN_TRAIN/resultats_predictions.txt";
    sauvegarder_predictions(chemin_fichier, predictions, nb_predictions);

    // 2. Préparer un résumé à envoyer au client
    char buffer[BUFFER_SIZE];
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "✓ Prédictions sauvegardées dans : %s\n", chemin_fichier);
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "=== APERÇU DES PRÉDICTIONS ===\n");
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "USER;ITEM;PREDICTION;NOTE_REELLE\n");

    int nb_a_afficher = (nb_predictions < 20) ? nb_predictions : 20;

    for (int i = 0; i < nb_a_afficher; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "%u;%u;%.2f;%.2f\n",
                           predictions[i].id_user,
                           predictions[i].id_article,
                           predictions[i].note_predite,
                           predictions[i].note_reelle);

        if (offset >= (int)(sizeof(buffer) - 100)) break; // marge de sécurité
    }

    if (nb_predictions > 20 && offset < (int)(sizeof(buffer) - 100)) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "... et %d autres prédictions\n", nb_predictions - 20);
    }

    // 3. Envoyer le résumé au client
    envoyer_reponse(client_sock, buffer);

    printf("✓ Fichier sauvegardé et résumé envoyé (%d prédictions)\n", nb_predictions);
}

void handle_stats(int client_sock) {
    printf("Traitement commande STATS\n");
    
    char buffer[512];
    if (!recommandeur_global) {
        envoyer_reponse(client_sock, "ERREUR: Aucun recommandeur chargé. Utilisez PEARSON d'abord.\n");
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

        buffer[strcspn(buffer, "\r\n")] = 0; // Nettoyer saut de ligne
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
            envoyer_reponse(client_sock, "ERREUR: Commande non reconnue\nCommandes disponibles: PEARSON, PREDICT, PREDICT_ALL, EVALUATE, SAVE, STATS, QUIT\n");
            printf("Commande non reconnue: %s\n", cmd);
        }
    }
    close(client_sock);
}
