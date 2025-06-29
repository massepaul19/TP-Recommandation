#include "fonctions_serveur.h"

#define PORT 8080
#define MAX_PENDING 5

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

    if (matrice_similarite && recommandeur_global) {
        liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
        matrice_similarite = NULL;
    }

    if (recommandeur_global) {
        liberer_recommandeur(recommandeur_global);
        recommandeur_global = NULL;
    }

    printf("Ressources libérées.\n");
}

void gerer_signal(int sig) {
    printf("\nSignal %d reçu. Arrêt du serveur...\n", sig);
    nettoyer_ressources();
    exit(0);
}

int main() {

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

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
        printf("En attente de connexion...\n");

        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Erreur accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("✓ Connexion de %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        const char* bienvenue =
            "=== SERVEUR KNN CONNECTÉ ===\n"
            "Commandes: PEARSON, PREDICT, PREDICT_ALL, EVALUATE, SAVE, STATS, QUIT\n"
            "Tapez une commande:\n";
        send(client_sock, bienvenue, strlen(bienvenue), 0);

        process_client(client_sock);

        printf("Connexion fermée avec %s:%d\n", client_ip, ntohs(client_addr.sin_port));
    }

    close(server_sock);
    nettoyer_ressources();
    return 0;
}

