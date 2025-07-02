#include "fonctions_serveur.h"

// Définitions des variables globales
pthread_mutex_t mutex_recommandeur;

void envoyer_reponse(int client_sock, const char* message) {
    send(client_sock, message, strlen(message), 0);
}

void handle_KNN(int client_sock, int id_user, int nb_reco) {
    printf("Traitement commande KNN pour utilisateur %d, %d recommandations\n", id_user, nb_reco);
    
    // Appel de votre fonction de traitement KNN
    char* resultat = traiter_recommandation_knn(id_user, nb_reco);
    
    if (resultat && strlen(resultat) > 0) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), 
                "=== RECOMMANDATIONS KNN ===\n"
                "Utilisateur: %d\n"
                "Nombre demandé: %d\n"
                "%s", 
                id_user, nb_reco, resultat);
        envoyer_reponse(client_sock, buffer);
        printf("Recommandations KNN envoyées pour utilisateur %d\n", id_user);
    } else {
        char erreur[256];
        snprintf(erreur, sizeof(erreur), 
                "ERREUR: Impossible de générer des recommandations KNN pour l'utilisateur %d\n", 
                id_user);
        envoyer_reponse(client_sock, erreur);
        printf("Erreur recommandations KNN pour utilisateur %d\n", id_user);
    }
}


void handle_factorisation(int client_sock, int id_user, int nb_reco) {
    printf("Traitement commande FACTORISATION pour utilisateur %d, %d recommandations\n", id_user, nb_reco);
    
    // Appel de votre fonction de traitement factorisation
    char* resultat = traiter_recommandation_factorisation(id_user, nb_reco);
    if (resultat && strlen(resultat) > 0) {
        // Vérifier si c'est une erreur
        if (strncmp(resultat, "ERREUR", 6) == 0) {
            envoyer_reponse(client_sock, resultat);
            printf("Erreur factorisation: %s\n", resultat);
        } else {
            char buffer[BUFFER_SIZE];
            snprintf(buffer, sizeof(buffer), 
                    "=== RECOMMANDATIONS FACTORISATION MATRICIELLE ===\n"
                    "Utilisateur: %d\n"
                    "Nombre demandé: %d\n"
                    "%s", 
                    id_user, nb_reco, resultat);
            envoyer_reponse(client_sock, buffer);
            printf("Recommandations factorisation envoyées pour utilisateur %d\n", id_user);
        }
    } else {
        char erreur[256];
        snprintf(erreur, sizeof(erreur), 
                "ERREUR: Impossible de générer des recommandations par factorisation pour l'utilisateur %d\n", 
                id_user);
        envoyer_reponse(client_sock, erreur);
        printf("Erreur recommandations factorisation pour utilisateur %d\n", id_user);
    }
}

void handle_graphe(int client_sock, int id_user, int nb_reco) {
    printf("Traitement commande GRAPHE pour utilisateur %d, %d recommandations\n", id_user, nb_reco);
    
    char* resultat = traiter_recommandation_graphe(id_user, nb_reco);
    
    if (resultat == NULL) {
        printf("DEBUG: traiter_recommandation_graphe a renvoyé NULL\n");
    } else {
        printf("DEBUG: traiter_recommandation_graphe a renvoyé :\n%s\n", resultat);
    }

    if (resultat && strlen(resultat) > 0) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), 
                "=== RECOMMANDATIONS GRAPHE BIPARTITE ===\n"
                "Utilisateur: %d\n"
                "Nombre demandé: %d\n"
                "%s", 
                id_user, nb_reco, resultat);
        envoyer_reponse(client_sock, buffer);
        printf("Recommandations graphe envoyées pour utilisateur %d\n", id_user);
    } else {
        char erreur[256];
        snprintf(erreur, sizeof(erreur), 
                "ERREUR: Impossible de générer des recommandations graphe pour l'utilisateur %d\n", 
                id_user);
        envoyer_reponse(client_sock, erreur);
        printf("Erreur recommandations graphe pour utilisateur %d\n", id_user);
    }
}


void process_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    printf("Nouveau client connecté\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            printf("Client déconnecté ou erreur recv (code %d)\n", received);
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = 0; // Nettoyer saut de ligne
        printf("Commande reçue: %s\n", buffer);

        char cmd[64] = {0}, param1[256] = {0}, param2[256] = {0};
        int n = sscanf(buffer, "%63s %255s %255s", cmd, param1, param2);

        if (strcasecmp(cmd, "KNN") == 0 && n >= 3) {
            pthread_mutex_lock(&mutex_recommandeur);
            handle_KNN(client_sock, atoi(param1), atoi(param2));
            pthread_mutex_unlock(&mutex_recommandeur);
        } 
        
        else if (strcasecmp(cmd, "FACTORISATION") == 0 && n >= 3) {
            pthread_mutex_lock(&mutex_recommandeur);
            handle_factorisation(client_sock, atoi(param1), atoi(param2));
            pthread_mutex_unlock(&mutex_recommandeur);
        } 
        else if (strcasecmp(cmd, "GRAPHE") == 0 && n >= 3) {
            pthread_mutex_lock(&mutex_recommandeur);
            handle_graphe(client_sock, atoi(param1), atoi(param2));
            pthread_mutex_unlock(&mutex_recommandeur);
        } 
        else if (strcasecmp(cmd, "QUIT") == 0) {
            envoyer_reponse(client_sock, "Au revoir !\n");
            printf("Client a quitté\n");
            break;
        } 
        else {
            envoyer_reponse(client_sock,
                "ERREUR: Commande non reconnue\n"
                "Commandes disponibles:\n"
                "  KNN <user_id> <nb_reco>\n"
                "  FACTORISATION <user_id> <nb_reco>\n"
                "  GRAPHE <user_id> <nb_reco>\n"
                "  QUIT\n");
            printf("Commande non reconnue: %s\n", cmd);
        }
    }

    close(client_sock);
    printf("Connexion client fermée\n");
}
