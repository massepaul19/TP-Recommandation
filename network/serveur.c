#include "fonctions_serveur.h"

#define PORT 8080
#define MAX_CLIENTS 10

// Variables globales
int server_running = 1;
int server_socket;

// Gestionnaire de signal pour arrêt propre
void gestionnaire_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                   ARRÊT DU SERVEUR EN COURS                  ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        server_running = 0;
        if (server_socket != -1) {
            close(server_socket);
        }
        exit(0);
    }
}

void afficher_banner() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║              SERVEUR DE RECOMMANDATIONS AVANCÉ               ║\n");
    printf("║                        Version 2.0 Pro                       ║\n");
    printf("║                                                              ║\n");
    printf("║                MASSE MASSE PAUL-BASTHYLLE                    ║\n");
    printf("║                                                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║ ALGORITHMES DISPONIBLES:                                     ║\n");
    printf("║   • KNN (K-Nearest Neighbors)                                ║\n");
    printf("║   • FACTORISATION (Factorisation Matricielle)                ║\n");
    printf("║   • GRAPHE (Graphe Bipartite + PageRank)                     ║\n");
    printf("║                                                              ║\n");
    printf("║ CONFIGURATION RÉSEAU:                                        ║\n");
    printf("║   • Port d'écoute: %-6d                                    ║\n", PORT);
    printf("║   • Clients maximum: %-3d                                     ║\n", MAX_CLIENTS);
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void afficher_commandes() {
    printf("╔═════════════════════════════════════════════════════════════╗\n");
    printf("║                      COMMANDES DISPONIBLES                  ║\n");
    printf("╠═════════════════════════════════════════════════════════════╣\n");
    printf("║                                                             ║\n");
    printf("║ 1 <user_id> <nb_reco>                                       ║\n");
    printf("║   Recommandations par K-Nearest Neighbors                   ║\n");
    printf("║   Exemple: 1 123 5                                          ║\n");
    printf("║                                                             ║\n");
    printf("║ 2 <user_id> <nb_reco>                                       ║\n");
    printf("║   Recommandations par Factorisation Matricielle             ║\n");
    printf("║   Exemple: 2 123 5                                          ║\n");
    printf("║                                                             ║\n");
    printf("║ 3 <user_id> <nb_reco>                                       ║\n");
    printf("║   Recommandations par Graphe Bipartite                      ║\n");
    printf("║   Exemple: 3 123 5                                          ║\n");
    printf("║                                                             ║\n");
    printf("║ 0 Fermer la connexion                                       ║\n");
    printf("║                                         		          ║\n");
    printf("╚═════════════════════════════════════════════════════════════╝\n");
}

void afficher_statistiques(int clients_connectes, int total_requetes) {
    time_t maintenant = time(NULL);
    char* time_str = ctime(&maintenant);
    // Enlever le \n de ctime
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    STATISTIQUES SERVEUR                      ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║ Clients connectés actuellement: %-3d                         ║\n", clients_connectes);
    printf("║ Total requêtes traitées: %-6d                                ║\n", total_requetes);
    printf("║ Serveur démarré: %-40s                                       ║\n", time_str);
    printf("║ État: EN FONCTIONNEMENT                                      ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

void* thread_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len);
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  NOUVEAU CLIENT CONNECTÉ                     ║\n");
    printf("║   Adresse: %-16s:%-6d                                        ║\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Envoyer le message de bienvenue
    char welcome_msg[2000];
    snprintf(welcome_msg, sizeof(welcome_msg),
        "╔══════════════════════════════════════════════════════════════╗\n"
        "║            BIENVENUE SUR LE SERVEUR DE RECOMMANDATIONS       ║\n"
        "║                                                              ║\n"
        "║                 MASSE MASSE PAUL-BASTHYLLE                   ║\n"
        "╠══════════════════════════════════════════════════════════════╣\n"
        "║                                                              ║\n"
        "║ COMMANDES DISPONIBLES:                                       ║\n"
        "║   1 <user_id> <nb_reco>  - KNN (K-Nearest Neighbors)         ║\n"
        "║   2 <user_id> <nb_reco>  - FACTORISATION (Matricielle)       ║\n"
        "║   3 <user_id> <nb_reco>  - GRAPHE (Bipartite)                ║\n"
        "║   0                      - QUIT (Déconnexion)                ║\n"
        "║                                                              ║\n"
        "║ Tapez votre commande et appuyez sur Entrée                   ║\n"
        "║ Le serveur est prêt à traiter vos requêtes!                  ║\n"
        "║                                                              ║\n"
        "╚══════════════════════════════════════════════════════════════╝\n"
        "Prêt > ");
    
    send(client_sock, welcome_msg, strlen(welcome_msg), 0);
    
    // Traitement du client
    process_client(client_sock);
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    CLIENT DÉCONNECTÉ                         ║\n");
    printf("║   Adresse: %-16s:%-6d                                 	   ║\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return NULL;
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock;
    int clients_connectes = 0;
    int total_requetes = 0;
    
    // Configuration du gestionnaire de signal
    signal(SIGINT, gestionnaire_signal);
    
    // Affichage du banner et des commandes
    afficher_banner();
    afficher_commandes();
    
    // Initialisation du mutex (si nécessaire)
    if (pthread_mutex_init(&mutex_recommandeur, NULL) != 0) {
        printf("║ ERREUR: Impossible d'initialiser le mutex                   ║\n");
        return 1;
    }
    
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        printf("║ ERREUR: Impossible de créer le socket                       ║\n");
        return 1;
    }
    
    // Configuration pour réutiliser l'adresse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("║ AVERTISSEMENT: Impossible de configurer SO_REUSEADDR        ║\n");
    }
    
    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Liaison du socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                     ERREUR CRITIQUE                          ║\n");
        printf("║   Impossible de lier le socket au port %-4d                  ║\n", PORT);
        printf("║   Conseil: Vérifiez que le port n'est pas déjà utilisé       ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        close(server_socket);
        return 1;
    }
    
    // Écoute des connexions
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        printf("║ ERREUR: Impossible d'écouter sur le socket                  ║\n");
        close(server_socket);
        return 1;
    }
    
    // Messages de démarrage réussi
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SERVEUR DÉMARRÉ AVEC SUCCÈS!                 ║\n");
    printf("║                                                              ║\n");
    printf("║ En écoute sur le port %-4d                                   ║\n", PORT);
    printf("║ En attente de connexions clients...                          ║\n");
    printf("║                                                              ║\n");
    printf("║ Utilisez Ctrl+C pour arrêter le serveur                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Boucle principale du serveur
    while (server_running) {
        client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock == -1) {
            if (server_running) {
                printf("║ Erreur lors de l'acceptation d'une connexion               ║\n");
            }
            continue;
        }
        
        // Création d'un thread pour chaque client
        pthread_t thread_id;
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;
        
        if (pthread_create(&thread_id, NULL, thread_client, client_sock_ptr) != 0) {
            printf("║ Erreur: Impossible de créer un thread pour le client       ║\n");
            close(client_sock);
            free(client_sock_ptr);
        } else {
            pthread_detach(thread_id);
            clients_connectes++;
            total_requetes++;
            
            // Affichage des statistiques toutes les 5 connexions
            if (total_requetes % 5 == 0) {
                afficher_statistiques(clients_connectes, total_requetes);
            }
        }
    }
    
    // Nettoyage final
    close(server_socket);
    pthread_mutex_destroy(&mutex_recommandeur);
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SERVEUR ARRÊTÉ PROPREMENT                    ║\n");
    printf("║          Toutes les ressources ont été libérées              ║\n");
    printf("║            Merci d'avoir utilisé notre serveur!              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
