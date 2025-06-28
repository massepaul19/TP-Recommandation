#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 8192
#define MAX_COMMAND_LENGTH 1024

// Structure pour stocker les informations de connexion
typedef struct {
    int socket;
    struct sockaddr_in server_addr;
    char server_ip[16];
    int server_port;
} client_connection_t;

// Fonction pour se connecter au serveur
int connecter_serveur(client_connection_t* client, const char* ip, int port) {
    // Créer le socket
    client->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket < 0) {
        perror("Erreur création socket client");
        return -1;
    }
    
    // Configurer l'adresse du serveur
    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {
        printf("Erreur: Adresse IP invalide\n");
        close(client->socket);
        return -1;
    }
    
    // Se connecter au serveur
    if (connect(client->socket, (struct sockaddr*)&client->server_addr, 
                sizeof(client->server_addr)) < 0) {
        perror("Erreur connexion au serveur");
        close(client->socket);
        return -1;
    }
    
    // Sauvegarder les informations de connexion
    strcpy(client->server_ip, ip);
    client->server_port = port;
    
    return 0;
}

// Fonction pour envoyer une commande au serveur
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
    
    return 0;
}

// Fonction pour recevoir une réponse du serveur
int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    
    int total_received = 0;
    int bytes_received;
    
    // Recevoir la réponse (peut être en plusieurs parties)
    while (total_received < buffer_size - 1) {
        bytes_received = recv(client->socket, buffer + total_received, 
                             buffer_size - total_received - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Connexion fermée par le serveur\n");
            } else {
                perror("Erreur réception");
            }
            return -1;
        }
        
        total_received += bytes_received;
        buffer[total_received] = '\0';
        
        // Vérifier si on a reçu une réponse complète
        // (simple heuristique basée sur le contenu)
        if (strstr(buffer, "OK:") || strstr(buffer, "ERREUR:") || 
            strstr(buffer, "===") || strstr(buffer, "Tapez votre commande:")) {
            break;
        }
        
        // Petit délai pour éviter de boucler trop rapidement
        usleep(10000);
    }
    
    return total_received;
}

// Fonction pour afficher l'aide du client
void afficher_aide_client() {
    printf("\n=== AIDE CLIENT KNN ===\n");
    printf("Commandes disponibles:\n\n");
    
    printf("1. PEARSON <fichier_train>\n");
    printf("   Exemple: PEARSON data/train.txt\n");
    printf("   -> Calculer la matrice de similarité Pearson\n\n");
    
    printf("2. PREDICT <id_user> <id_article>\n");
    printf("   Exemple: PREDICT 123 456\n");
    printf("   -> Prédire la note d'un utilisateur pour un article\n\n");
    
    printf("3. PREDICT_ALL <fichier_test>\n");
    printf("   Exemple: PREDICT_ALL data/test.txt\n");
    printf("   -> Effectuer toutes les prédictions sur le jeu de test\n\n");
    
    printf("4. EVALUATE\n");
    printf("   -> Évaluer les performances (MAE, RMSE)\n\n");
    
    printf("5. SAVE [nom_fichier]\n");
    printf("   Exemple: SAVE predictions.txt\n");
    printf("   -> Sauvegarder les prédictions\n\n");
    
    printf("6. STATS\n");
    printf("   -> Afficher les statistiques du serveur\n\n");
    
    printf("7. !help\n");
    printf("   -> Afficher cette aide\n\n");
    
    printf("8. !quit\n");
    printf("   -> Quitter le client\n\n");
    
    printf("Note: Les commandes commençant par '!' sont traitées localement\n");
    printf("=======================\n\n");
}

// Fonction pour traiter les commandes locales (commençant par !)
int traiter_commande_locale(const char* commande) {
    if (strcasecmp(commande, "!help") == 0) {
        afficher_aide_client();
        return 1;
    }
    
    if (strcasecmp(commande, "!quit") == 0) {
        printf("Déconnexion du client...\n");
        return 0;
    }
    
    return -1; // Commande locale non reconnue
}

// Fonction pour valider une commande avant de l'envoyer
int valider_commande(const char* commande) {
    char cmd[256], param1[256], param2[256];
    int nb_params = sscanf(commande, "%255s %255s %255s", cmd, param1, param2);
    
    if (strcasecmp(cmd, "PEARSON") == 0) {
        if (nb_params < 2) {
            printf("Erreur: PEARSON nécessite un nom de fichier\n");
            printf("Usage: PEARSON <fichier_train>\n");
            return 0;
        }
    } else if (strcasecmp(cmd, "PREDICT") == 0) {
        if (nb_params < 3) {
            printf("Erreur: PREDICT nécessite un ID utilisateur et un ID article\n");
            printf("Usage: PREDICT <id_user> <id_article>\n");
            return 0;
        }
        unsigned int id_user = (unsigned int)atoi(param1);
        unsigned int id_article = (unsigned int)atoi(param2);
        if (id_user == 0 || id_article == 0) {
            printf("Erreur: Les IDs doivent être des nombres entiers > 0\n");
            return 0;
        }
    } else if (strcasecmp(cmd, "PREDICT_ALL") == 0) {
        if (nb_params < 2) {
            printf("Erreur: PREDICT_ALL nécessite un nom de fichier\n");
            printf("Usage: PREDICT_ALL <fichier_test>\n");
            return 0;
        }
    }
    
    return 1; // Commande valide
}

// Fonction principale d'interaction avec le serveur
void boucle_interaction(client_connection_t* client) {
    char buffer[BUFFER_SIZE];
    char commande[MAX_COMMAND_LENGTH];
    
    printf("\n=== CLIENT KNN CONNECTÉ ===\n");
    printf("Serveur: %s:%d\n", client->server_ip, client->server_port);
    printf("Tapez '!help' pour l'aide, '!quit' pour quitter\n");
    printf("===============================\n\n");
    
    // Recevoir le message de bienvenue du serveur
    if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
        printf("%s\n", buffer);
    }
    
    // Boucle principale d'interaction
    while (1) {
        printf("KNN> ");
        fflush(stdout);
        
        // Lire la commande de l'utilisateur
        if (!fgets(commande, sizeof(commande), stdin)) {
            printf("\nErreur lecture commande\n");
            break;
        }
        
        // Nettoyer la commande
        char* newline = strchr(commande, '\n');
        if (newline) *newline = '\0';
        
        // Ignorer les commandes vides
        if (strlen(commande) == 0) continue;
        
        // Traiter les commandes locales
        int resultat_local = traiter_commande_locale(commande);
        if (resultat_local == 0) {
            // Commande !quit
            break;
        } else if (resultat_local == 1) {
            // Commande locale traitée
            continue;
        }
        
        // Valider la commande avant de l'envoyer
        if (!valider_commande(commande)) {
            continue;
        }
        
        // Ajouter un retour à la ligne pour le serveur
        strcat(commande, "\n");
        
        // Envoyer la commande au serveur
        if (envoyer_commande(client, commande) < 0) {
            printf("Erreur: Impossible d'envoyer la commande\n");
            break;
        }
        
        // Recevoir et afficher la réponse
        if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
            printf("\n%s\n", buffer);
        } else {
            printf("Erreur: Pas de réponse du serveur\n");
            break;
        }
    }
}

// Fonction pour fermer la connexion proprement
void fermer_connexion(client_connection_t* client) {
    if (client->socket >= 0) {
        // Envoyer QUIT au serveur
        envoyer_commande(client, "QUIT\n");
        
        // Fermer le socket
        close(client->socket);
        client->socket = -1;
        
        printf("Connexion fermée\n");
    }
}

// Fonction pour tester la connexion
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

int main(int argc, char* argv[]) {
    client_connection_t client;
    char server_ip[16] = "127.0.0.1";  // localhost par défaut
    int server_port = 8080;            // port par défaut
    
    printf("========================================\n");
    printf("=== CLIENT KNN RECOMMANDATION ===\n");
    printf("========================================\n");
    
    // Parser les arguments de ligne de commande
    if (argc >= 2) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
        server_ip[sizeof(server_ip) - 1] = '\0';
    }
    
    if (argc >= 3) {
        server_port = atoi(argv[2]);
        if (server_port <= 0 || server_port > 65535) {
            printf("Erreur: Port invalide (%d)\n", server_port);
            printf("Usage: %s [adresse_ip] [port]\n", argv[0]);
            return 1;
        }
    }
    
    // Afficher les paramètres de connexion
    printf("Serveur cible: %s:%d\n", server_ip, server_port);
    printf("========================================\n\n");
    
    // Tester la connexion
    if (!tester_connexion(server_ip, server_port)) {
        printf("\nImpossible de se connecter au serveur.\n");
        printf("Vérifiez que le serveur est démarré et accessible.\n");
        printf("\nUsage: %s [adresse_ip] [port]\n", argv[0]);
        printf("Exemple: %s 192.168.1.100 8080\n", argv[0]);
        return 1;
    }
    
    // Se connecter au serveur
    if (connecter_serveur(&client, server_ip, server_port) < 0) {
        printf("Échec de la connexion au serveur\n");
        return 1;
    }
    
    printf("Connexion établie avec succès!\n");
    
    // Lancer la boucle d'interaction
    boucle_interaction(&client);
    
    // Fermer la connexion proprement
    fermer_connexion(&client);
    
    printf("\nClient terminé.\n");
    return 0;
}
