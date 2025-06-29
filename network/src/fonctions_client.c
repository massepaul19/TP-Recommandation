#include "fonctions_client.h"

int connecter_serveur(client_connection_t* client, const char* ip, int port) {
    client->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket < 0) {
        perror("Erreur création socket client");
        return -1;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {
        printf("Erreur: Adresse IP invalide\n");
        close(client->socket);
        return -1;
    }

    if (connect(client->socket, (struct sockaddr*)&client->server_addr,
                sizeof(client->server_addr)) < 0) {
        perror("Erreur connexion au serveur");
        close(client->socket);
        return -1;
    }

    strcpy(client->server_ip, ip);
    client->server_port = port;
    return 0;
}

int envoyer_commande(client_connection_t* client, const char* commande) {
    int len = strlen(commande);
    int sent = 0;

    while (sent < len) {
        int bytes = send(client->socket, commande + sent, len - sent, 0);
        if (bytes <= 0) {
            printf("Erreur: Impossible d'envoyer la commande\n");
            return -1;
        }
        sent += bytes;
    }
    
    printf("Commande envoyée: %s", commande);
    return 0;
}

#include <time.h>  // pour clock_gettime

int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    int total_received = 0;
    int bytes_received;

    const int timeout_ms = 5000;  // Timeout total 5000 ms = 5 sec
    const int wait_per_try_ms = 50; // pause 50ms entre essais

    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (total_received < buffer_size - 1) {
        bytes_received = recv(client->socket, buffer + total_received,
                              buffer_size - total_received - 1, MSG_DONTWAIT);
        if (bytes_received > 0) {
            total_received += bytes_received;
            buffer[total_received] = '\0';

            // Reset timer à chaque nouvelle donnée reçue (optionnel)
            clock_gettime(CLOCK_MONOTONIC, &start_time);

            // Vérifier si la réponse est complète selon tes mots clés
            if (strstr(buffer, "OK:") || strstr(buffer, "ERREUR:") ||
                strstr(buffer, "PREDICTION:") || strstr(buffer, "EVALUATION:") ||
                strstr(buffer, "SAUVEGARDE:") || strstr(buffer, "STATISTIQUES:") ||
                strstr(buffer, "Au revoir")) {
                break; // réponse complète reçue
            }

        } else if (bytes_received == 0) {
            printf("Connexion fermée par le serveur\n");
            return -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Pas de données disponibles => pause puis vérifier timeout
                usleep(wait_per_try_ms * 1000);

                clock_gettime(CLOCK_MONOTONIC, &current_time);
                long elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000
                                  + (current_time.tv_nsec - start_time.tv_nsec) / 1000000;

                if (elapsed_ms >= timeout_ms) {
                    printf("Timeout: Aucune réponse reçue du serveur après %d ms\n", timeout_ms);
                    return -1;
                }

                continue;
            } else {
                perror("Erreur réception");
                return -1;
            }
        }
    }

    return total_received;
}


/*
int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size) {

    memset(buffer, 0, buffer_size);
    int total_received = 0;
    int bytes_received;
    int tentatives = 0;

    while (total_received < buffer_size - 1 && tentatives < 50) {
        bytes_received = recv(client->socket, buffer + total_received,
                              buffer_size - total_received - 1, MSG_DONTWAIT);
        
        if (bytes_received > 0) {
            total_received += bytes_received;
            buffer[total_received] = '\0';
            tentatives = 0; // Reset tentatives si on reçoit des données
        } else if (bytes_received == 0) {
            printf("Connexion fermée par le serveur\n");
            return -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Pas de données disponibles, attendre un peu
                usleep(50000); // 50ms
                tentatives++;
                continue;
            } else {
                perror("Erreur réception");
                return -1;
            }
        }
        
        // Vérifier si on a une réponse complète
        if (strstr(buffer, "OK:") || strstr(buffer, "ERREUR:") ||
            strstr(buffer, "PREDICTION:") || strstr(buffer, "EVALUATION:") ||
            strstr(buffer, "SAUVEGARDE:") || strstr(buffer, "STATISTIQUES:") ||
            strstr(buffer, "Au revoir")) {
            break;
        }
    }

    if (tentatives >= 50) {
        printf("Timeout: Aucune réponse reçue du serveur\n");
        return -1;
    }

    return total_received;
}
*/

void fermer_connexion(client_connection_t* client) {
    if (client->socket >= 0) {
        envoyer_commande(client, "QUIT\n");
        close(client->socket);
        client->socket = -1;
        printf("\nConnexion fermée\n");
    }
}

int tester_connexion(const char* ip, int port) {
    client_connection_t test_client;
    printf("Test de connexion à %s:%d...\n", ip, port);

    if (connecter_serveur(&test_client, ip, port) == 0) {
        printf("✓ Connexion réussie\n");
        close(test_client.socket);
        return 1;
    } else {
        printf("✗ Échec de la connexion\n");
        return 0;
    }
}

void afficher_menu() {
    printf("\n========================================\n");
    printf("=== MENU CLIENT KNN RECOMMANDATION ===\n");
    printf("========================================\n");
    printf("1. Calculer la matrice de Pearson\n");
    printf("2. Prédiction individuelle (user, item)\n");
    printf("3. Effectuer toutes les prédictions sur le fichier de test\n");
    printf("4. Évaluer les performances\n");
    printf("5. Sauvegarder les résultats\n");
    printf("6. Afficher les statistiques\n");
    printf("0. Quitter\n");
    printf("========================================\n");
}

void menu_numerique(client_connection_t* client) {

    char buffer[BUFFER_SIZE];
    int choix = -1;

    // Fichiers par défaut
    char fichier_train[] = "data/KNN_TRAIN/Train.txt";
    char fichier_test[] = "data/KNN_TRAIN/Test.txt";

    printf("\n=== INTERFACE NUMÉRIQUE DU CLIENT ===\n");
    printf("Fichiers utilisés:\n");
    printf("  - Train: %s\n", fichier_train);
    printf("  - Test: %s\n", fichier_test);

    while (1) {
        afficher_menu();
        printf("Votre choix : ");

        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide ! Veuillez entrer un nombre.\n");
            while (getchar() != '\n'); // vider le buffer
            continue;
        }
        while (getchar() != '\n'); // consommer le \n restant

        char commande[MAX_COMMAND_LENGTH];
        memset(commande, 0, sizeof(commande));

        switch (choix) {
            case 1:
                printf("\n=== CALCUL DE LA MATRICE DE PEARSON ===\n");
                snprintf(commande, sizeof(commande), "PEARSON %s\n", fichier_train);
                break;

            case 2: {
                printf("\n=== PRÉDICTION INDIVIDUELLE ===\n");
                unsigned int id_user, id_article;
                printf("ID utilisateur : ");
                if (scanf("%u", &id_user) != 1) {
                    printf("ID utilisateur invalide\n");
                    while (getchar() != '\n');
                    continue;
                }
                printf("ID article : ");
                if (scanf("%u", &id_article) != 1) {
                    printf("ID article invalide\n");
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');
                snprintf(commande, sizeof(commande), "PREDICT %u %u\n", id_user, id_article);
                break;
            }

            case 3:
                printf("\n=== PRÉDICTIONS SUR FICHIER TEST ===\n");
                snprintf(commande, sizeof(commande), "PREDICT_ALL %s\n", fichier_test);
                break;

            case 4:
                printf("\n=== ÉVALUATION DES PERFORMANCES ===\n");
                strcpy(commande, "EVALUATE\n");
                break;

            case 5:
                printf("\n=== SAUVEGARDE DES RÉSULTATS ===\n");
                strcpy(commande, "SAVE\n");
                break;

            case 6:
                printf("\n=== STATISTIQUES DU SYSTÈME ===\n");
                strcpy(commande, "STATS\n");
                break;

            case 0:
                printf("\n=== DÉCONNEXION ===\n");
                strcpy(commande, "QUIT\n");
                envoyer_commande(client, commande);
                printf("Déconnexion...\n");
                return;

            default:
                printf("Choix invalide ! Veuillez choisir entre 0 et 6.\n");
                continue;
        }

        // Envoyer la commande
        printf("Envoi de la commande...\n");
        if (envoyer_commande(client, commande) < 0) {
            printf("Erreur lors de l'envoi de la commande.\n");
            break;
        }

        // Recevoir la réponse
        printf("En attente de la réponse du serveur...\n");
        if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
            printf("\n=== RÉPONSE DU SERVEUR ===\n");
            printf("%s\n", buffer);
        } else {
            printf("Erreur : aucune réponse reçue du serveur.\n");
            break;
        }
        
        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();
    }
}
