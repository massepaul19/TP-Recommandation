#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "reco.h"
#include "reco_KNN.h"
//#include "menu.h"      // Pour les fonctions de menu si nécessaire

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096
#define MAX_PREDICTIONS 10000

// Variables globales partagées (protégées par mutex)
extern RecommandeurKNN* recommandeur_global;
static double** matrice_pearson_global = NULL;
static pthread_mutex_t mutex_recommandeur = PTHREAD_MUTEX_INITIALIZER;
static int serveur_pret = 0;
static Prediction predictions_globales[MAX_PREDICTIONS];
static int nb_predictions_globales = 0;

// Structure pour passer les données au thread client
typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    int client_id;
} client_data_t;

// Fonction pour envoyer une réponse au client
void envoyer_reponse(int socket, const char* reponse) {
    int len = strlen(reponse);
    int sent = 0;
    
    while (sent < len) {
        int bytes = send(socket, reponse + sent, len - sent, 0);
        if (bytes <= 0) {
            printf("[ERREUR] Impossible d'envoyer la réponse complète\n");
            break;
        }
        sent += bytes;
    }
}

// Fonction pour traiter la commande PEARSON
void traiter_commande_pearson(int client_socket, const char* filename) {
    char reponse[BUFFER_SIZE];
    
    printf("[SERVEUR] Calcul de la matrice de Pearson pour %s\n", filename);
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    // Libérer l'ancienne matrice si elle existe
    if (matrice_pearson_global && recommandeur_global) {
        liberer_matrice_similarite(matrice_pearson_global, recommandeur_global->nb_users);
        matrice_pearson_global = NULL;
    }
    
    // Libérer l'ancien recommandeur si il existe
    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }
    
    // Calculer la nouvelle matrice
    matrice_pearson_global = Pearson(filename);
    
    if (matrice_pearson_global != NULL && recommandeur_global != NULL) {
        serveur_pret = 1;
        nb_predictions_globales = 0; // Réinitialiser les prédictions
        
        snprintf(reponse, sizeof(reponse), 
                "OK: Matrice de Pearson calculée avec succès\n"
                "Utilisateurs: %d\n"
                "Articles: %d\n"
                "Transactions: %d\n"
                "Matrice: %dx%d\n", 
                recommandeur_global->nb_users,
                recommandeur_global->nb_articles,
                recommandeur_global->nb_transactions,
                recommandeur_global->nb_users, 
                recommandeur_global->nb_users);
    } else {
        serveur_pret = 0;
        snprintf(reponse, sizeof(reponse), 
                "ERREUR: Impossible de calculer la matrice de Pearson\n"
                "Vérifiez que le fichier %s existe et est accessible\n", filename);
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    
    envoyer_reponse(client_socket, reponse);
}

// Fonction pour traiter la commande PREDICT
void traiter_commande_predict(int client_socket, unsigned int id_user, unsigned int id_article) {
    char reponse[BUFFER_SIZE];
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    if (!serveur_pret || recommandeur_global == NULL || matrice_pearson_global == NULL) {
        snprintf(reponse, sizeof(reponse), 
                "ERREUR: Serveur non initialisé\n"
                "Utilisez d'abord la commande PEARSON <fichier_train>\n");
    } else {
        // Vérifier que les IDs existent
        int user_idx = trouver_index_utilisateur(recommandeur_global, id_user);
        int item_idx = trouver_index_article(recommandeur_global, id_article);
        
        if (user_idx == -1) {
            snprintf(reponse, sizeof(reponse), 
                    "ERREUR: Utilisateur %u non trouvé dans les données\n", id_user);
        } else if (item_idx == -1) {
            snprintf(reponse, sizeof(reponse), 
                    "ERREUR: Article %u non trouvé dans les données\n", id_article);
        } else {
            // Appeler votre fonction existante
            float prediction = Predict(recommandeur_global, id_user, id_article);
            float note_reelle = recommandeur_global->matrice_evaluations[user_idx][item_idx];
            
            if (prediction >= 0) {
                snprintf(reponse, sizeof(reponse), 
                        "OK: Prédiction réussie\n"
                        "User ID: %u (index: %d)\n"
                        "Item ID: %u (index: %d)\n"
                        "Prédiction: %.3f\n"
                        "Note réelle: %s\n"
                        "%s",
                        id_user, user_idx,
                        id_article, item_idx,
                        prediction,
                        (note_reelle > 0) ? "Oui" : "Non évaluée",
                        (note_reelle > 0) ? 
                            (snprintf(reponse + strlen(reponse), sizeof(reponse) - strlen(reponse), 
                                     "Valeur réelle: %.3f\nErreur absolue: %.3f\n", 
                                     note_reelle, fabs(prediction - note_reelle)), "") : "");
            } else {
                snprintf(reponse, sizeof(reponse), 
                        "ERREUR: Impossible de calculer la prédiction\n"
                        "Utilisateur %u, Article %u\n", id_user, id_article);
            }
        }
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    
    envoyer_reponse(client_socket, reponse);
}

// Fonction pour traiter la commande PREDICT_ALL
void traiter_commande_predict_all(int client_socket, const char* test_filename) {
    char reponse[BUFFER_SIZE];
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    if (!serveur_pret || recommandeur_global == NULL || matrice_pearson_global == NULL) {
        snprintf(reponse, sizeof(reponse), 
                "ERREUR: Serveur non initialisé\n"
                "Utilisez d'abord la commande PEARSON <fichier_train>\n");
    } else {
        printf("[SERVEUR] Calcul de toutes les prédictions pour %s\n", test_filename);
        
        // Appeler votre fonction existante
        nb_predictions_globales = Predict_all(recommandeur_global, test_filename, 
                                            predictions_globales, MAX_PREDICTIONS);
        
        if (nb_predictions_globales > 0) {
            // Calculer les métriques
            double rmse = calculer_rmse(predictions_globales, nb_predictions_globales);
            double mae = calculer_mae(predictions_globales, nb_predictions_globales);
            
            snprintf(reponse, sizeof(reponse), 
                    "OK: Prédictions effectuées avec succès\n"
                    "Nombre de prédictions: %d\n"
                    "RMSE: %.4f\n"
                    "MAE: %.4f\n"
                    "Fichier test: %s\n"
                    "\nPremiers exemples:\n", 
                    nb_predictions_globales, rmse, mae, test_filename);
            
            // Ajouter quelques exemples
            int nb_exemples = (nb_predictions_globales < 3) ? nb_predictions_globales : 3;
            for (int i = 0; i < nb_exemples; i++) {
                char exemple[256];
                snprintf(exemple, sizeof(exemple), 
                        "User %u, Item %u -> Prédiction: %.3f, Réel: %.3f\n",
                        predictions_globales[i].id_user, 
                        predictions_globales[i].id_article,
                        predictions_globales[i].note_predite, 
                        predictions_globales[i].note_reelle);
                strncat(reponse, exemple, sizeof(reponse) - strlen(reponse) - 1);
            }
            
        } else {
            snprintf(reponse, sizeof(reponse), 
                    "ERREUR: Aucune prédiction effectuée\n"
                    "Vérifiez que le fichier %s existe et contient des données valides\n", 
                    test_filename);
        }
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    
    envoyer_reponse(client_socket, reponse);
}

// Fonction pour traiter la commande EVALUATE
void traiter_commande_evaluate(int client_socket) {
    char reponse[BUFFER_SIZE * 2]; // Plus grand buffer pour les métriques détaillées
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    if (nb_predictions_globales == 0) {
        snprintf(reponse, sizeof(reponse), 
                "ERREUR: Aucune prédiction disponible\n"
                "Utilisez d'abord la commande PREDICT_ALL <fichier_test>\n");
    } else {
        // Calculer les métriques détaillées
        double mae = calculer_mae(predictions_globales, nb_predictions_globales);
        double rmse = calculer_rmse(predictions_globales, nb_predictions_globales);
        
        snprintf(reponse, sizeof(reponse), 
                "=== ÉVALUATION DES PERFORMANCES ===\n"
                "Nombre de prédictions: %d\n"
                "MAE (Mean Absolute Error): %.4f\n"
                "RMSE (Root Mean Square Error): %.4f\n"
                "=====================================\n",
                nb_predictions_globales, mae, rmse);
        
        // Ajouter des statistiques supplémentaires si vous avez une fonction pour cela
        // Par exemple, si vous avez une fonction afficher_metriques_evaluation:
        // (Cette partie dépend de votre implémentation existante)
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    
    envoyer_reponse(client_socket, reponse);
}

// Fonction pour traiter la commande SAVE
void traiter_commande_save(int client_socket, const char* filename) {
    char reponse[BUFFER_SIZE];
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    if (nb_predictions_globales == 0) {
        snprintf(reponse, sizeof(reponse), 
                "ERREUR: Aucune prédiction à sauvegarder\n"
                "Utilisez d'abord la commande PREDICT_ALL <fichier_test>\n");
    } else {
        // Utiliser le nom de fichier fourni ou un nom par défaut
        const char* nom_fichier = (filename && strlen(filename) > 0) ? 
                                 filename : "data/predictions_serveur.txt";
        
        // Appeler votre fonction de sauvegarde existante
        sauvegarder_predictions(nom_fichier, predictions_globales, nb_predictions_globales);
        
        snprintf(reponse, sizeof(reponse), 
                "OK: Prédictions sauvegardées avec succès\n"
                "Fichier: %s\n"
                "Nombre de prédictions: %d\n",
                nom_fichier, nb_predictions_globales);
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    
    envoyer_reponse(client_socket, reponse);
}

// Fonction principale du thread client
void* handle_client(void* arg) {
    client_data_t* client = (client_data_t*)arg;
    char buffer[BUFFER_SIZE];
    char commande[256], param1[256], param2[256];
    
    printf("[SERVEUR] Client %d connecté depuis %s:%d\n", 
           client->client_id, 
           inet_ntoa(client->client_addr.sin_addr), 
           ntohs(client->client_addr.sin_port));
    
    // Message de bienvenue
    const char* bienvenue = 
        "========================================\n"
        "=== SERVEUR KNN RECOMMANDATION ===\n"
        "========================================\n"
        "Commandes disponibles:\n"
        "\n"
        "PEARSON <fichier_train>\n"
        "  -> Calculer la matrice de similarité\n"
        "\n"
        "PREDICT <id_user> <id_article>\n"
        "  -> Prédire une note pour un utilisateur/article\n"
        "\n" 
        "PREDICT_ALL <fichier_test>\n"
        "  -> Effectuer toutes les prédictions sur le jeu de test\n"
        "\n"
        "EVALUATE\n"
        "  -> Évaluer les performances (MAE, RMSE)\n"
        "\n"
        "SAVE [nom_fichier]\n"
        "  -> Sauvegarder les prédictions\n"
        "\n"
        "STATS\n"
        "  -> Afficher les statistiques du serveur\n"
        "\n"
        "HELP\n"
        "  -> Afficher cette aide\n"
        "\n"
        "QUIT\n"
        "  -> Déconnecter\n"
        "========================================\n"
        "Tapez votre commande:\n";
    
    envoyer_reponse(client->client_socket, bienvenue);
    
    // Boucle de traitement des commandes
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_recus = recv(client->client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_recus <= 0) {
            if (bytes_recus == 0) {
                printf("[CLIENT %d] Connexion fermée proprement\n", client->client_id);
            } else {
                printf("[CLIENT %d] Erreur de réception: %s\n", client->client_id, strerror(errno));
            }
            break;
        }
        
        // Nettoyer la commande reçue
        buffer[bytes_recus] = '\0';
        
        // Enlever les caractères de fin de ligne
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        newline = strchr(buffer, '\r');
        if (newline) *newline = '\0';
        
        // Ignorer les commandes vides
        if (strlen(buffer) == 0) continue;
        
        printf("[CLIENT %d] Commande reçue: '%s'\n", client->client_id, buffer);
        
        // Parser la commande
        memset(commande, 0, sizeof(commande));
        memset(param1, 0, sizeof(param1));
        memset(param2, 0, sizeof(param2));
        
        int nb_params = sscanf(buffer, "%255s %255s %255s", commande, param1, param2);
        
        // Traiter les commandes
        if (strcasecmp(commande, "PEARSON") == 0 && nb_params >= 2) {
            traiter_commande_pearson(client->client_socket, param1);
            
        } else if (strcasecmp(commande, "PREDICT") == 0 && nb_params >= 3) {
            unsigned int id_user = (unsigned int)atoi(param1);
            unsigned int id_article = (unsigned int)atoi(param2);
            
            if (id_user == 0 || id_article == 0) {
                envoyer_reponse(client->client_socket, 
                              "ERREUR: IDs utilisateur et article doivent être des nombres > 0\n");
            } else {
                traiter_commande_predict(client->client_socket, id_user, id_article);
            }
            
        } else if (strcasecmp(commande, "PREDICT_ALL") == 0 && nb_params >= 2) {
            traiter_commande_predict_all(client->client_socket, param1);
            
        } else if (strcasecmp(commande, "EVALUATE") == 0) {
            traiter_commande_evaluate(client->client_socket);
            
        } else if (strcasecmp(commande, "SAVE") == 0) {
            const char* filename = (nb_params >= 2) ? param1 : NULL;
            traiter_commande_save(client->client_socket, filename);
            
        } else if (strcasecmp(commande, "STATS") == 0) {
            pthread_mutex_lock(&mutex_recommandeur);
            char stats[BUFFER_SIZE];
            
            if (recommandeur_global) {
                snprintf(stats, sizeof(stats), 
                        "=== STATISTIQUES SERVEUR ===\n"
                        "État: %s\n"
                        "Utilisateurs: %d\n"
                        "Articles: %d\n" 
                        "Transactions: %d\n"
                        "Matrice calculée: %s\n"
                        "Prédictions en mémoire: %d\n"
                        "=============================\n",
                        serveur_pret ? "Prêt" : "Non initialisé",
                        recommandeur_global->nb_users,
                        recommandeur_global->nb_articles,
                        recommandeur_global->nb_transactions,
                        matrice_pearson_global ? "Oui" : "Non",
                        nb_predictions_globales);
            } else {
                snprintf(stats, sizeof(stats), 
                        "=== STATISTIQUES SERVEUR ===\n"
                        "État: Non initialisé\n"
                        "Aucune donnée chargée\n"
                        "Utilisez PEARSON <fichier> pour initialiser\n"
                        "=============================\n");
            }
            
            pthread_mutex_unlock(&mutex_recommandeur);
            envoyer_reponse(client->client_socket, stats);
            
        } else if (strcasecmp(commande, "HELP") == 0) {
            envoyer_reponse(client->client_socket, bienvenue);
            
        } else if (strcasecmp(commande, "QUIT") == 0) {
            envoyer_reponse(client->client_socket, "Au revoir! Déconnexion...\n");
            break;
            
        } else {
            char erreur[BUFFER_SIZE];
            snprintf(erreur, sizeof(erreur),
                    "ERREUR: Commande inconnue ou paramètres manquants\n"
                    "Commande reçue: %s\n"
                    "Tapez HELP pour voir les commandes disponibles\n",
                    commande);
            envoyer_reponse(client->client_socket, erreur);
        }
        
        // Petite pause pour éviter la surcharge
        usleep(1000);
    }
    
    close(client->client_socket);
    free(client);
    printf("[CLIENT %d] Thread terminé\n", client->client_id);
    pthread_exit(NULL);
}

// Fonction pour gérer l'arrêt propre du serveur
void cleanup_serveur() {
    printf("\n[SERVEUR] Nettoyage des ressources...\n");
    
    pthread_mutex_lock(&mutex_recommandeur);
    
    if (matrice_pearson_global && recommandeur_global) {
        liberer_matrice_similarite(matrice_pearson_global, recommandeur_global->nb_users);
        matrice_pearson_global = NULL;
    }
    
    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }
    
    pthread_mutex_unlock(&mutex_recommandeur);
    pthread_mutex_destroy(&mutex_recommandeur);
    
    printf("[SERVEUR] Nettoyage terminé\n");
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;
    static int client_counter = 0;
    
    printf("========================================\n");
    printf("=== SERVEUR KNN RECOMMANDATION ===\n");
    printf("========================================\n");
    printf("Port: %d\n", PORT);
    printf("Clients max: %d\n", MAX_CLIENTS);
    printf("========================================\n");
    
    // Gestionnaire de signal pour un arrêt propre
    signal(SIGINT, (void(*)(int))cleanup_serveur);
    signal(SIGTERM, (void(*)(int))cleanup_serveur);
    
    // Créer le socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur création socket");
        exit(1);
    }
    
    // Permettre la réutilisation de l'adresse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erreur setsockopt");
        close(server_socket);
        exit(1);
    }
    
    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Lier le socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    // Écouter les connexions
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        close(server_socket);
        exit(1);
    }
    
    printf("Serveur en écoute sur le port %d...\n", PORT);
    printf("En attente de connexions clients...\n");
    printf("Appuyez sur Ctrl+C pour arrêter le serveur\n\n");
    
    // Boucle principale du serveur
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            if (errno == EINTR) {
                printf("[SERVEUR] Interruption détectée, arrêt du serveur...\n");
                break;
            }
            perror("Erreur accept");
            continue;
        }
        
        // Créer la structure pour le thread client
        client_data_t* client_data = malloc(sizeof(client_data_t));
        if (!client_data) {
            printf("[ERREUR] Allocation mémoire pour client\n");
            close(client_socket);
            continue;
        }
        
        client_data->client_socket = client_socket;
        client_data->client_addr = client_addr;
        client_data->client_id = ++client_counter;
        
        // Créer un thread pour ce client
        if (pthread_create(&thread_id, NULL, handle_client, client_data) != 0) {
            perror("Erreur création thread");
            close(client_socket);
            free(client_data);
            continue;
        }
        
        // Détacher le thread
        pthread_detach(thread_id);
    }
    
    close(server_socket);
    cleanup_serveur();
    
    printf("Serveur arrêté.\n");
    return 0;
}
