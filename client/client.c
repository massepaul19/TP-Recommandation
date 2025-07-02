#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "fonctions_client.h"

#define PORT 8080

#define IP_SERVEUR "192.168.43.71"
#define IP_SERVEUR_LOCAL "127.0.0.1"

int main(int argc, char* argv[]) {
    client_connection_t client;
    char server_ip[16] = IP_SERVEUR; 
    //char server_ip[16] = IP_SERVEUR_LOCAL;
    int server_port = PORT;                 
    
    afficher_Masse_client();
    
    // Analyse des arguments : IP et port
    if (argc >= 2) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
        server_ip[sizeof(server_ip) - 1] = '\0';
    }
    if (argc >= 3) {
        int p = atoi(argv[2]);
        if (p <= 0 || p > 65535) {
            printf("Erreur: Port invalide (%d)\n", p);
            printf("Usage: %s [adresse_ip] [port]\n", argv[0]);
            return 1;
        }
        server_port = p;
    }
    
    printf("Serveur cible : %s:%d\n", server_ip, server_port);
    printf("========================================\n");
    
    // Test rapide de connexion avant de lancer la vraie connexion
    if (!tester_connexion(server_ip, server_port)) {
        printf("\n Impossible de se connecter au serveur.\n");
        printf("Vérifiez que le serveur est démarré sur %s:%d\n", server_ip, server_port);
        return 1;
    }
    
    // Connexion réelle
    if (connecter_serveur(&client, server_ip, server_port) < 0) {
        printf(" Échec de la connexion au serveur\n");
        return 1;
    }
    printf("✓ Connexion établie avec succès !\n");
    
    // Recevoir le message de bienvenue du serveur
    
    char welcome[BUFFER_SIZE] = {0};
    if (recevoir_reponse(&client, welcome, sizeof(welcome)) > 0) {
        printf("\n=== MESSAGE DU SERVEUR ===\n");
        printf("%s\n", welcome);
    }
    
    // Lancer le menu numérique (interface utilisateur)
    
    printf("[DEBUG] Lancement du menu...\n");
    menu_numerique(&client);
    printf("[DEBUG] Menu terminé.\n");
    
    // Fermer proprement la connexion
    fermer_connexion(&client);
    printf("\nClient terminé.\n");
    return 0;
}
