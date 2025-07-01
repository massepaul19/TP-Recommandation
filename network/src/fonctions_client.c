#include "fonctions_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int tester_connexion(const char* ip, int port) {
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (test_socket < 0) return 0;
    
    struct sockaddr_in test_addr = {0};
    test_addr.sin_family = AF_INET;
    test_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &test_addr.sin_addr);
    
    int result = connect(test_socket, (struct sockaddr*)&test_addr, sizeof(test_addr));
    close(test_socket);
    return (result == 0);
}

int connecter_serveur(client_connection_t* client, const char* ip, int port) {
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        perror("Erreur création socket");
        return -1;
    }
    
    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {
        printf("Erreur: Adresse IP invalide\n");
        close(client->socket_fd);
        return -1;
    }
    
    if (connect(client->socket_fd, (struct sockaddr*)&client->server_addr, 
                sizeof(client->server_addr)) < 0) {
        perror("Erreur connexion");
        close(client->socket_fd);
        return -1;
    }
    
    return 0;
}

int recevoir_reponse(client_connection_t* client, char* buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    int bytes_received = recv(client->socket_fd, buffer, buffer_size - 1, 0);
    if (bytes_received < 0) {
        perror("Erreur réception");
        return -1;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

int envoyer_requete(client_connection_t* client, const char* message) {
    int bytes_sent = send(client->socket_fd, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Erreur envoi");
        return -1;
    }
    return bytes_sent;
}

void fermer_connexion(client_connection_t* client) {
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
}

void afficher_menu(void) {
    printf("\n========== MENU RECOMMANDATION ==========\n");
    printf("1. KNN (K-Plus Proches Voisins)\n");
    printf("2. Factorisation Matricielle\n");
    printf("3. PageRank sur Graphe Bipartite\n");
    printf("4. Quitter\n");
    printf("=========================================\n");
    printf("Votre choix: ");
}

void menu_numerique(client_connection_t* client) {
    char buffer[BUFFER_SIZE];
    char message[256];
    int choix, id_user, nb_reco;
    
    while (1) {
        afficher_menu();
        
        if (scanf("%d", &choix) != 1) {
            printf("❌ Entrée invalide. Veuillez entrer un numéro.\n");
            while (getchar() != '\n'); // Vider le buffer
            continue;
        }
        
        switch (choix) {
            case 1:
                printf("\n=== RECOMMANDATION KNN ===\n");
                printf("Entrez votre ID utilisateur: ");
                scanf("%d", &id_user);
                printf("Nombre de recommandations souhaitées: ");
                scanf("%d", &nb_reco);
                
                snprintf(message, sizeof(message), "KNN %d %d", id_user, nb_reco);
                if (envoyer_requete(client, message) < 0) {
                    printf("❌ Erreur envoi requête KNN\n");
                    break;
                }
                
                if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
                    printf("\n=== RÉSULTATS KNN ===\n");
                    printf("%s\n", buffer);
                }
                break;
                
            case 2:
                printf("\n=== RECOMMANDATION FACTORISATION MATRICIELLE ===\n");
                printf("Entrez votre ID utilisateur: ");
                scanf("%d", &id_user);
                printf("Nombre de recommandations souhaitées: ");
                scanf("%d", &nb_reco);
                
                snprintf(message, sizeof(message), "MATRIX %d %d", id_user, nb_reco);
                if (envoyer_requete(client, message) < 0) {
                    printf("❌ Erreur envoi requête Factorisation\n");
                    break;
                }
                
                if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
                    printf("\n=== RÉSULTATS FACTORISATION ===\n");
                    printf("%s\n", buffer);
                }
                break;
                
            case 3:
                printf("\n=== RECOMMANDATION PAGERANK ===\n");
                printf("Entrez votre ID utilisateur: ");
                scanf("%d", &id_user);
                printf("Nombre de recommandations souhaitées: ");
                scanf("%d", &nb_reco);
                
                snprintf(message, sizeof(message), "PAGERANK %d %d", id_user, nb_reco);
                if (envoyer_requete(client, message) < 0) {
                    printf("❌ Erreur envoi requête PageRank\n");
                    break;
                }
                
                if (recevoir_reponse(client, buffer, sizeof(buffer)) > 0) {
                    printf("\n=== RÉSULTATS PAGERANK ===\n");
                    printf("%s\n", buffer);
                }
                break;
                
            case 4:
                printf("\n👋 Au revoir !\n");
                envoyer_requete(client, "QUIT");
                return;
                
            default:
                printf("❌ Choix invalide. Veuillez choisir entre 1 et 4.\n");
                break;
        }
        
        printf("\nAppuyez sur Entrée pour continuer...");
        getchar(); // Vider le buffer
        getchar(); // Attendre l'entrée utilisateur
    }
}
