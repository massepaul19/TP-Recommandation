#include "fonctions_serveur.h"

#define PORT 8080
#define MAX_PENDING 5

// Variables globales
RecommandeurKNN* recommandeur_global = NULL;

void afficher_aide() {
    printf("\n=== SERVEUR DE RECOMMANDATION KNN ===\n");
    printf("Commandes disponibles pour les clients :\n");
    printf("  PEARSON <fichier_train>        - Calculer la matrice de Pearson\n");
    printf("  PREDICT <user_id> <item_id>    - Prédire une note\n");
    printf("  PREDICT_ALL <fichier_test>     - Prédire toutes les notes du fichier test\n");
    printf("  EVALUATE                       - Évaluer les prédictions (MAE, RMSE)\n");
    printf("  SAVE                           - Sauvegarder les résultats\n");
    printf("  STATS                          - Afficher les statistiques\n");
    printf("  QUIT                           - Déconnecter le client\n");
    printf("\nServeur en écoute sur le port %d...\n", PORT);
    printf("Utilisez Ctrl+C pour arrêter le serveur.\n\n");
}

void nettoyer_ressources() {
    printf("\nNettoyage des ressources...\n");

    pthread_mutex_lock(&mutex_recommandeur);
    if (matrice_similarite && recommandeur_global) {
        liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
        matrice_similarite = NULL;
    }

    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }
    pthread_mutex_unlock(&mutex_recommandeur);

    printf("Ressources libérées.\n");
}

void gerer_signal(int sig) {
    printf("\nSignal %d reçu. Arrêt du serveur...\n", sig);
    nettoyer_ressources();
    exit(0);
}

// Thread qui gère chaque client
void* thread_gerer_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg); // libère la mémoire allouée dans main()

    // Traiter les commandes du client
    // IMPORTANT : Si process_client utilise recommandeur_global ou matrice_similarite,
    // il doit gérer lui-même la synchronisation (mutex) ou on peut l'encapsuler ici.
    
    // Exemple simple : verrouillage avant appel, déverrouillage après.
    pthread_mutex_lock(&mutex_recommandeur);
    process_client(client_sock);
    pthread_mutex_unlock(&mutex_recommandeur);

    close(client_sock);
    printf("Client déconnecté (socket %d)\n", client_sock);
    return NULL;
}

int main() {
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    printf("=== DÉMARRAGE DU SERVEUR KNN ===\n");

    signal(SIGINT, gerer_signal);
    signal(SIGTERM, gerer_signal);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Erreur création socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, MAX_PENDING) < 0) {
        perror("Erreur listen");
        close(server_sock);
        exit(1);
    }

    printf("✓ Serveur démarré avec succès\n");
    afficher_aide();

    while (1) {
        int* client_sock_ptr = malloc(sizeof(int));
        if (!client_sock_ptr) {
            perror("malloc");
            continue;
        }

        *client_sock_ptr = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (*client_sock_ptr < 0) {
            perror("Erreur accept");
            free(client_sock_ptr);
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("✓ Connexion de %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        const char* bienvenue =
            "=== SERVEUR KNN CONNECTÉ ===\n"
            "Commandes: PEARSON, PREDICT, PREDICT_ALL, EVALUATE, SAVE, STATS, QUIT\n"
            "Tapez une commande:\n";
        send(*client_sock_ptr, bienvenue, strlen(bienvenue), 0);

        // Création du thread client
        if (pthread_create(&tid, NULL, thread_gerer_client, client_sock_ptr) != 0) {
            perror("Erreur pthread_create");
            close(*client_sock_ptr);
            free(client_sock_ptr);
            continue;
        }

        pthread_detach(tid);  // Pour éviter fuites mémoire liées aux threads terminés
    }

    close(server_sock);
    nettoyer_ressources();
    return 0;
}

