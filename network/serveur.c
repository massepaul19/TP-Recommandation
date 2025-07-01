#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8081
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10

// Structures pour les données de recommandation
typedef struct {
    int id_user;
    int id_article;
    float rating;
} Transaction;

typedef struct {
    int nb_users;
    int nb_articles;
    int max_user_id;
    int max_article_id;
    int* map_users;
    int* map_articles;
    int* reverse_map_users;
    int* reverse_map_articles;
} GrapheBipartite;

typedef struct {
    float* pagerank_vector;
    int size;
} ResultatPageRank;

typedef struct {
    float** matrice_utilisateurs;
    float** matrice_articles;
    int nb_users;
    int nb_articles;
    int nb_factors;
} FactorisationMatricielle;

typedef struct {
    int client_socket;
    int client_id;
    GrapheBipartite* graphe;
    ResultatPageRank* pagerank;
    FactorisationMatricielle* factorisation;
    Transaction* transactions;
    int nb_transactions;
} client_data_t;

// Variables globales
int server_socket = -1;
int server_running = 1;

// Fonctions de recommandation (à adapter selon vos implémentations)
void recommander_knn_buffer(int id_user, int nb_reco, Transaction* transactions, 
                           int nb_transactions, char* buffer) {
    snprintf(buffer, BUFFER_SIZE, "Recommandations KNN pour l'utilisateur %d:\n", id_user);
    int offset = strlen(buffer);
    
    // Simuler des recommandations KNN (à remplacer par votre algorithme)
    for (int i = 0; i < nb_reco && i < 5; i++) {
        int article_id = 100 + (id_user * 10) + i;
        float similarity = 0.9f - (i * 0.1f);
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, 
                         "%d. Article %d (Similarité: %.2f)\n", 
                         i + 1, article_id, similarity);
    }
    
    if (nb_reco == 0) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "Aucune recommandation disponible.\n");
    }
}

void recommander_factorisation_buffer(int id_user, int nb_reco, 
                                    FactorisationMatricielle* factorisation, 
                                    char* buffer) {
    snprintf(buffer, BUFFER_SIZE, "Recommandations Factorisation Matricielle pour l'utilisateur %d:\n", id_user);
    int offset = strlen(buffer);
    
    // Simuler des recommandations par factorisation (à remplacer par votre algorithme)
    for (int i = 0; i < nb_reco && i < 5; i++) {
        int article_id = 200 + (id_user * 10) + i;
        float score = 4.5f - (i * 0.3f);
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, 
                         "%d. Article %d (Score prédit: %.2f)\n", 
                         i + 1, article_id, score);
    }
    
    if (nb_reco == 0) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "Aucune recommandation disponible.\n");
    }
}

void recommander_articles_buffer(int id_user, int nb_reco, GrapheBipartite *graphe, 
                               ResultatPageRank *pagerank, Transaction *transactions,
                               int nb_transactions, char* buffer) {
    snprintf(buffer, BUFFER_SIZE, "Recommandations PageRank pour l'utilisateur %d:\n", id_user);
    int offset = strlen(buffer);
    
    // Vérification de l'existence de l'utilisateur
    if (id_user < 0 || (graphe && id_user >= graphe->max_user_id)) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "Erreur : Utilisateur inconnu.\n");
        return;
    }
    
    // Simuler des recommandations PageRank (à remplacer par votre algorithme)
    for (int i = 0; i < nb_reco && i < 5; i++) {
        int article_id = 300 + (id_user * 10) + i;
        float score = 0.8f - (i * 0.1f);
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, 
                         "%d. Article %d (Score PageRank: %.4f)\n", 
                         i + 1, article_id, score);
    }
    
    if (nb_reco == 0) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "Aucune recommandation disponible.\n");
    }
}

void traiter_requete(client_data_t* client_data, const char* requete, char* reponse) {
    char commande[32];
    int id_user, nb_reco;
    
    // Parser la requête
    if (sscanf(requete, "%s %d %d", commande, &id_user, &nb_reco) != 3) {
        snprintf(reponse, BUFFER_SIZE, "Erreur: Format de requête invalide.\n");
        return;
    }
    
    // Validation des paramètres
    if (id_user < 0 || nb_reco <= 0 || nb_reco > 20) {
        snprintf(reponse, BUFFER_SIZE, "Erreur: Paramètres invalides (ID utilisateur: %d, Nb recommandations: %d).\n", 
                id_user, nb_reco);
        return;
    }
    
    // Traiter selon la commande
    if (strcmp(commande, "KNN") == 0) {
        printf("[Client %d] Requête KNN: utilisateur %d, %d recommandations\n", 
               client_data->client_id, id_user, nb_reco);
        recommander_knn_buffer(id_user, nb_reco, client_data->transactions, 
                             client_data->nb_transactions, reponse);
    }
    else if (strcmp(commande, "MATRIX") == 0) {
        printf("[Client %d] Requête FACTORISATION: utilisateur %d, %d recommandations\n", 
               client_data->client_id, id_user, nb_reco);
        recommander_factorisation_buffer(id_user, nb_reco, client_data->factorisation, reponse);
    }
    else if (strcmp(commande, "PAGERANK") == 0) {
        printf("[Client %d] Requête PAGERANK: utilisateur %d, %d recommandations\n", 
               client_data->client_id, id_user, nb_reco);
        recommander_articles_buffer(id_user, nb_reco, client_data->graphe, 
                                  client_data->pagerank, client_data->transactions,
                                  client_data->nb_transactions, reponse);
    }
    else {
        snprintf(reponse, BUFFER_SIZE, "Erreur: Commande inconnue '%s'.\n", commande);
    }
}

void* gerer_client(void* arg) {
    client_data_t* client_data = (client_data_t*)arg;
    char buffer[BUFFER_SIZE];
    char reponse[BUFFER_SIZE];
    
    printf("✓ Client %d connecté\n", client_data->client_id);
    
    // Envoyer message de bienvenue
    snprintf(buffer, sizeof(buffer), 
             "=== SERVEUR DE RECOMMANDATION ===\n"
             "Bienvenue ! Connecté en tant que client %d\n"
             "Algorithmes disponibles: KNN, MATRIX, PAGERANK\n"
             "Format: <ALGORITHME> <ID_USER> <NB_RECO>\n"
             "==================================", 
             client_data->client_id);
    
    if (send(client_data->client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Erreur envoi bienvenue");
        goto cleanup;
    }
    
    // Boucle de traitement des requêtes
    while (server_running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_data->client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client %d déconnecté proprement\n", client_data->client_id);
            } else {
                printf("Erreur réception client %d: %s\n", client_data->client_id, strerror(errno));
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        // Retirer les caractères de fin de ligne
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        char* carriage = strchr(buffer, '\r');
        if (carriage) *carriage = '\0';
        
        printf("[Client %d] Requête reçue: '%s'\n", client_data->client_id, buffer);
        
        // Traiter la requête QUIT
        if (strcmp(buffer, "QUIT") == 0) {
            printf("Client %d a demandé la déconnexion\n", client_data->client_id);
            break;
        }
        
        // Traiter la requête et générer la réponse
        traiter_requete(client_data, buffer, reponse);
        
        // Envoyer la réponse
        if (send(client_data->client_socket, reponse, strlen(reponse), 0) < 0) {
            perror("Erreur envoi réponse");
            break;
        }
        
        printf("[Client %d] Réponse envoyée\n", client_data->client_id);
    }
    
cleanup:
    close(client_data->client_socket);
    free(client_data);
    printf("✗ Client %d déconnecté\n", client_data->client_id);
    return NULL;
}

void signal_handler(int signal) {
    printf("\n🛑 Arrêt du serveur demandé (signal %d)\n", signal);
    server_running = 0;
    if (server_socket >= 0) {
        close(server_socket);
    }
    exit(0);
}

GrapheBipartite* initialiser_graphe_demo() {
    GrapheBipartite* graphe = malloc(sizeof(GrapheBipartite));
    graphe->nb_users = 100;
    graphe->nb_articles = 200;
    graphe->max_user_id = 1000;
    graphe->max_article_id = 2000;
    
    // Initialisation simple pour la démo
    graphe->map_users = calloc(graphe->max_user_id, sizeof(int));
    graphe->map_articles = calloc(graphe->max_article_id, sizeof(int));
    graphe->reverse_map_users = calloc(graphe->nb_users, sizeof(int));
    graphe->reverse_map_articles = calloc(graphe->nb_articles, sizeof(int));
    
    return graphe;
}

ResultatPageRank* initialiser_pagerank_demo() {
    ResultatPageRank* pagerank = malloc(sizeof(ResultatPageRank));
    pagerank->size = 300;  // 100 users + 200 articles
    pagerank->pagerank_vector = calloc(pagerank->size, sizeof(float));
    
    // Valeurs simulées
    for (int i = 0; i < pagerank->size; i++) {
        pagerank->pagerank_vector[i] = 0.1f + (rand() % 100) / 1000.0f;
    }
    
    return pagerank;
}

FactorisationMatricielle* initialiser_factorisation_demo() {
    FactorisationMatricielle* factorisation = malloc(sizeof(FactorisationMatricielle));
    factorisation->nb_users = 100;
    factorisation->nb_articles = 200;
    factorisation->nb_factors = 10;
    
    // Allocation et initialisation simple pour la démo
    factorisation->matrice_utilisateurs = malloc(factorisation->nb_users * sizeof(float*));
    factorisation->matrice_articles = malloc(factorisation->nb_articles * sizeof(float*));
    
    for (int i = 0; i < factorisation->nb_users; i++) {
        factorisation->matrice_utilisateurs[i] = calloc(factorisation->nb_factors, sizeof(float));
    }
    
    for (int i = 0; i < factorisation->nb_articles; i++) {
        factorisation->matrice_articles[i] = calloc(factorisation->nb_factors, sizeof(float));
    }
    
    return factorisation;
}

Transaction* initialiser_transactions_demo(int* nb_transactions) {
    *nb_transactions = 1000;
    Transaction* transactions = malloc(*nb_transactions * sizeof(Transaction));
    
    // Générer des transactions simulées
    for (int i = 0; i < *nb_transactions; i++) {
        transactions[i].id_user = rand() % 100;
        transactions[i].id_article = rand() % 200;
        transactions[i].rating = 1.0f + (rand() % 50) / 10.0f;
    }
    
    return transactions;
}

int main(int argc, char* argv[]) {
    int port = PORT;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_counter = 0;
    
    // Gestion des signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("========================================\n");
    printf("=== SERVEUR DE RECOMMANDATION v3.0 ===\n");
    printf("========================================\n");
    
    // Traitement des arguments
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            printf("Erreur: Port invalide (%d)\n", port);
            return 1;
        }
    }
    
    printf("🚀 Initialisation des données de recommandation...\n");
    
    // Initialiser les structures de données (version démo)
    GrapheBipartite* graphe = initialiser_graphe_demo();
    ResultatPageRank* pagerank = initialiser_pagerank_demo();
    FactorisationMatricielle* factorisation = initialiser_factorisation_demo();
    int nb_transactions;
    Transaction* transactions = initialiser_transactions_demo(&nb_transactions);
    
    printf("✓ Données initialisées:\n");
    printf("  - Graphe: %d utilisateurs, %d articles\n", graphe->nb_users, graphe->nb_articles);
    printf("  - PageRank: %d éléments\n", pagerank->size);
    printf("  - Factorisation: %d facteurs\n", factorisation->nb_factors);
    printf("  - Transactions: %d éléments\n", nb_transactions);
    
    // Créer le socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur création socket");
        return 1;
    }
    
    // Réutiliser l'adresse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erreur setsockopt");
        close(server_socket);
        return 1;
    }
    
    // Configuration de l'adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Lier le socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_socket);
        return 1;
    }
    
    // Écouter les connexions
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        close(server_socket);
        return 1;
    }
    
    printf("🔊 Serveur en écoute sur le port %d\n", port);
    printf("📞 En attente de connexions...\n");
    printf("========================================\n");
    
    // Boucle principale d'acceptation des clients
    while (server_running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (server_running) {
                perror("Erreur accept");
            }
            continue;
        }
        
        // Créer les données du client
        client_data_t* client_data = malloc(sizeof(client_data_t));
        client_data->client_socket = client_socket;
        client_data->client_id = ++client_counter;
        client_data->graphe = graphe;
        client_data->pagerank = pagerank;
        client_data->factorisation = factorisation;
        client_data->transactions = transactions;
        client_data->nb_transactions = nb_transactions;
        
        printf("📞 Nouvelle connexion de %s (Client %d)\n", 
               inet_ntoa(client_addr.sin_addr), client_data->client_id);
        
        // Créer un thread pour gérer le client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, gerer_client, client_data) != 0) {
            perror("Erreur création thread");
            close(client_socket);
            free(client_data);
            continue;
        }
        
        // Détacher le thread pour libération automatique
        pthread_detach(client_thread);
    }
    
    // Nettoyage
    close(server_socket);
    printf("\n🛑 Serveur arrêté\n");
    return 0;
}
