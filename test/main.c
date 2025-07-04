#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

//RecommandeurKNN* recommandeur_global = NULL;

// Prototypes des fonctions de menu

//void Menu_Traitement();
//void Menu_KNN(const char* train_file, const char* test_file);

// Fonction pour afficher le menu principal

void afficher_menu_principal() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   SYSTÈME DE RECOMMANDATION                  ║\n");
    printf("║                         Version 1.0                          ║\n");   
    printf("║                MASSE MASSE PAUL - BASTHYLLE                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  1. Traitement et gestion des données                        ║\n");
    printf("║  2. Extraction de données vers Train                         ║\n");    
    printf("║  3. Système de recommandation KNN                            ║\n");
    printf("║  4. Système de recommandation Factorisation Matricielle      ║\n");
    printf("║  5. Système de recommandation avec Graphes                   ║\n");
    printf("║  6. À propos du système                                      ║\n");
    printf("║  0. Quitter le programme                                     ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nVotre choix : ");
}

int main() {
    int choix;
    
    while (1) {
        afficher_menu_principal();
        scanf("%d", &choix);
        
        switch (choix) {
            case 1:
                printf("\n--- Traitement et gestion des données ---\n");
                Menu_Traitement();
                break;
                
            case 2:
                printf("\n--- Debut de l'extraction .... ---\n");
                extraction(); 
                break;
            
            case 3:
                printf("\n--- Système de recommandation KNN ---\n");
                Menu_KNN("data/KNN_TRAIN/Train.txt");
                break;
            
            case 4:
            	Menu_Factorisation();
            	break;
            	
            case 5:
            	Menu_GRAPHE();
            	break;
                
            case 6:
                printf("\n╔══════════════════════════════════════════════════════════════╗\n");
                printf("║                      À PROPOS DU SYSTÈME                     ║\n");
                printf("╠══════════════════════════════════════════════════════════════╣\n");
                printf("║                                                              ║\n");
                printf("║  Système de Recommandation - Version 1.0                     ║\n");
                printf("║  Utilise l'algorithme K-Nearest Neighbors (KNN) ,            ║\n");
                printf("║  Factorisation Matricielle et Pae Rank                       ║\n");
                printf("║  pour fournir des recommandations personnalisées             ║\n");
                printf("║                                                              ║\n");
                printf("║  Fonctionnalités :                                           ║\n");
                printf("║  • Traitement et gestion des données                         ║\n");
                printf("║  • Algorithme pour les recommandations                       ║\n");
                printf("║  • Génération des Transactions                               ║\n");
                printf("║  • Interface utilisateur intuitive                           ║\n");
                printf("║                                                              ║\n");     
                printf("║  Methodes de Recommandations :                               ║\n");
                printf("║  • Algorithme Facto. Matricielle pour les recommandations    ║\n");
                printf("║  • Algorithme KNN pour les recommandations                   ║\n");
                printf("║  • Algorithme Page Rank avec les Graphes                     ║\n");
                printf("║                                                              ║\n");
                printf("║  Connexion Client serveur :                                  ║\n");
                printf("║  • Grace aux sockets et aux threads nous avons la possibilité║\n");
                printf("║  d'avoir un serveur et plusieurs clints connectés            ║\n");
                printf("║                                                              ║\n");
                printf("║  Côté Réseau :                                 	       ║\n");
                printf("║   Notre app offre une facilité d'adaptation au               ║\n");
                printf("║   réseau pour les tests:                                     ║\n");
                printf("║  d'avoir un serveur et plusieurs clints connectés            ║\n");
                printf("║                                                              ║\n");
                printf("║  	• Option 1 : Localhost avec 127.0.0.1                  ║\n");
                printf("║  	• Option 2 : Grace à un AP (en réseau)                 ║\n");
                printf("║                                                              ║\n");
                printf("║                                             Paulo Masse      ║\n");
                printf("╚══════════════════════════════════════════════════════════════╝\n");
                printf("\nAppuyez sur Entrée pour continuer...");
                getchar(); // Pour consommer le '\n' restant
                getchar(); // Pour attendre l'appui sur Entrée
                break;
                
            case 0:
                printf("\n╔══════════════════════════════════════════════════════════════╗\n");
                printf("║                      MERCI D'AVOIR UTILISÉ                   ║\n");
                printf("║                  LE SYSTÈME DE RECOMMANDATION                ║\n");
                printf("║                                                              ║\n");
                printf("║                     Au revoir !   Paulo Masse                ║\n");
                printf("╚══════════════════════════════════════════════════════════════╝\n");
                exit(0);
                
            default:
                printf("\n⚠️  Choix invalide ! Veuillez sélectionner une option valide (0-3).\n");
                printf("Appuyez sur Entrée pour continuer...");
                getchar(); // Pour consommer le '\n' restant
                getchar(); // Pour attendre l'appui sur Entrée
        }
    }
    
    return 0;
}
