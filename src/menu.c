#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "reco_KNN.h"
#include "menu.h"

// Déclaration de la variable globale
extern RecommandeurKNN* recommandeur_global;

void Menu_Traitement() {
    User users[MAX_USERS] = {0};
    Article articles[MAX_ARTICLES] = {0};
    Categorie categories[MAX_CATEGORIES] = {0};
    Transaction transactions[MAX_ARTICLES];
    
    int nb_users = 0;
    int nb_articles = 0;
    int nb_categories = 0;
    int nb_transactions = 0;
    
    // Chargement initial des données
    charger_donnees(users, &nb_users, articles, &nb_articles, categories, &nb_categories, transactions, &nb_transactions);
    
    if (nb_transactions == 0) {
        printf("Aucune donnée trouvée. Vérifiez le fichier data/donnees.txt\n");
    }
    
    // Sauvegarder les structures dans les fichiers séparés
    ecrire_users("data/Users.txt", users, nb_users);
    ecrire_articles("data/Articles.txt", articles, nb_articles);
    ecrire_categories("data/Categories.txt", categories, nb_categories);

    int choix;

    while (1) {
        printf("\n\n=== MENU TRAITEMENT DES DONNÉES ===\n\n");
        printf("1. Afficher les statistiques\n");
        printf("2. Extraire les transactions entre deux dates\n");
        printf("3. Nettoyer les données de test\n");
        printf("4. Filtrer les données\n");
        printf("5. Recharger les données depuis donnees.txt\n");  
        printf("6. Sauvegarder les structures séparées\n");
        printf("0. Retour au menu principal\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                afficher_stats("data/donnees.txt");
                break;

            case 2: {
                struct tm start_tm = {0}, end_tm = {0};
                int jour, mois, annee;

                printf("\n--- Période de début ---\n");
                printf("Jour : "); scanf("%d", &jour);
                printf("Mois : "); scanf("%d", &mois);
                printf("Année : "); scanf("%d", &annee);
                start_tm.tm_mday = jour;
                start_tm.tm_mon = mois - 1;
                start_tm.tm_year = annee - 1900;

                printf("\n--- Période de fin ---\n");
                printf("Jour : "); scanf("%d", &jour);
                printf("Mois : "); scanf("%d", &mois);
                printf("Année : "); scanf("%d", &annee);
                end_tm.tm_mday = jour;
                end_tm.tm_mon = mois - 1;
                end_tm.tm_year = annee - 1900;

                time_t start_time = mktime(&start_tm);
                time_t end_time = mktime(&end_tm);

                extraire_transactions_par_periode("data/donnees.txt", "data/transactions_t1-t2.txt", start_time, end_time);
                printf("Extraction terminée. Voir : data/transactions_t1-t2.txt\n");
                break;
            }

            case 3: {
                int nb_trans = nettoyer_fichier_test("data/essai/Train.txt", "data/essai/Test.txt", "data/essai/Clean.txt");
                
                if (nb_trans > 0) {
                    printf("Nous avons %d transactions extraites dans Clean.txt\n", nb_trans);
                    printf("ATTENTION : Pour utiliser ces données nettoyées, copiez le contenu de\n");
                    printf("'data/essai/Clean.txt' dans 'data/donnees.txt'\n");
                    printf("puis utilisez l'option 5 pour recharger.\n");
                } else {
                    printf("Pas de transaction extraite\n");
                }
                break;
            }

            case 4: {
                // Menu de filtrage
                int choix_filtrage;
                printf("\n=== MENU DE FILTRAGE ===\n");
                printf("1. Filtrer par utilisateurs uniquement\n");
                printf("2. Filtrer par articles uniquement\n");
                printf("3. Filtrer par utilisateurs ET articles\n");
                printf("0. Retour au menu principal\n");
                printf("Votre choix : ");
                scanf("%d", &choix_filtrage);

                switch (choix_filtrage) {
                    case 1: {
                        int minU;
                        printf("\nFiltrage par utilisateurs\n");
                        printf("Entrer le nombre minimum d'occurrences pour les utilisateurs : ");
                        scanf("%d", &minU);
                        
                        int result = filtrer_transactions_par_min_U("data/donnees.txt", "data/transactions_filtrees_par_users.txt", minU);
                        
                        if (result == -1) {
                            printf("Erreur lors du filtrage des transactions.\n");
                        } else if (result == 0) {
                            printf("Aucune transaction n'a passé les filtres.\n");
                        } else {
                            printf("Filtrage terminé avec succès ! %d transactions conservées.\n", result);
                            printf("Résultat sauvé dans : data/transactions_filtrees_par_users.txt\n");
                            printf("ATTENTION : Pour utiliser les données filtrées, copiez le contenu de\n");
                            printf("'data/transactions_filtrees_par_users.txt' dans 'data/donnees.txt'\n");
                            printf("puis utilisez l'option 5 pour recharger.\n");
                        }
                        break;
                    }
                    
                    case 2: {
                        int minI;
                        printf("\nFiltrage par articles\n");
                        printf("Entrer le nombre minimum d'occurrences pour les articles : ");
                        scanf("%d", &minI);
                        
                        int result = filtrer_transactions_par_min_I("data/donnees.txt", "data/transactions_filtrees_par_articles.txt", minI);
                        
                        if (result == -1) {
                            printf("Erreur lors du filtrage des transactions.\n");
                        } else if (result == 0) {
                            printf("Aucune transaction n'a passé les filtres.\n");
                        } else {
                            printf("Filtrage terminé avec succès ! %d transactions conservées.\n", result);
                            printf("Résultat sauvé dans : data/transactions_filtrees_par_articles.txt\n");
                            printf("ATTENTION : Pour utiliser les données filtrées, copiez le contenu de\n");
                            printf("'data/transactions_filtrees_par_articles.txt' dans 'data/donnees.txt'\n");
                            printf("puis utilisez l'option 5 pour recharger.\n");
                        }
                        break;
                    }
                    
                    case 3: {
                        int minU, minI;
                        printf("\nFiltrage par utilisateurs ET articles\n");
                        printf("Entrer le nombre minimum d'occurrences pour les utilisateurs : ");
                        scanf("%d", &minU);
                        printf("Entrer le nombre minimum d'occurrences pour les articles : ");
                        scanf("%d", &minI);
                        
                        int result = filtrer_transactions_par_min_U_et_I("data/donnees.txt", "data/transactions_filtrees_complet.txt", minU, minI);
                        
                        if (result == -1) {
                            printf("Erreur lors du filtrage des transactions.\n");
                        } else if (result == 0) {
                            printf("Aucune transaction n'a passé les filtres.\n");
                        } else {
                            printf("Filtrage terminé avec succès ! %d transactions conservées.\n", result);
                            printf("Résultat sauvé dans : data/transactions_filtrees_complet.txt\n");
                            printf("ATTENTION : Pour utiliser les données filtrées, copiez le contenu de\n");
                            printf("'data/transactions_filtrees_complet.txt' dans 'data/donnees.txt'\n");
                            printf("puis utilisez l'option 5 pour recharger.\n");
                        }
                        break;
                    }
                    
                    case 0:
                        break;
                        
                    default:
                        printf("Choix invalide pour le filtrage.\n");
                        break;
                }
                break;
            }

            case 5: {
                // Recharger les données depuis donnees.txt
                printf("Rechargement des données depuis donnees.txt...\n");
                charger_donnees(users, &nb_users, articles, &nb_articles, categories, &nb_categories, transactions, &nb_transactions);
                
                // Mettre à jour les fichiers séparés
                ecrire_users("data/Users.txt", users, nb_users);
                ecrire_articles("data/Articles.txt", articles, nb_articles);
                ecrire_categories("data/Categories.txt", categories, nb_categories);
                printf("Structures synchronisées avec succès !\n");
                break;
            }

            case 6: {
                // Sauvegarder seulement les structures séparées (pas donnees.txt)
                ecrire_users("data/Users.txt", users, nb_users);
                ecrire_articles("data/Articles.txt", articles, nb_articles);
                ecrire_categories("data/Categories.txt", categories, nb_categories);
                printf("Structures séparées sauvegardées avec succès !\n");
                printf("ATTENTION : donnees.txt n'a pas été modifié.\n");
                break;
            }

            case 0:
                // Sauvegarder les structures avant de quitter (pas donnees.txt)
                printf("Sauvegarde des structures avant de quitter...\n");
                ecrire_users("data/Users.txt", users, nb_users);
                ecrire_articles("data/Articles.txt", articles, nb_articles);
                ecrire_categories("data/Categories.txt", categories, nb_categories);
                printf("Retour au menu principal.\n");
                return;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();
        getchar();
    }
}

void Menu_KNN(const char* train_file, const char* test_file) {
    double** matrice_similarite = NULL;
    Prediction predictions[10000]; // Ajuster selon les besoins
    int nb_predictions = 0;
    int choix;

    while (1) {
        printf("\n\n=== MENU SYSTÈME KNN ===\n\n");
        printf("1. Calculer la matrice de Pearson\n");
        printf("2. Tester des prédictions individuelles\n");
        printf("3. Effectuer toutes les prédictions sur le fichier de test\n");
        printf("4. Évaluer les performances des prédictions\n");
        printf("5. Sauvegarder les résultats des prédictions\n");
        printf("6. Afficher les statistiques du recommandeur\n");
        printf("0. Retour au menu principal\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                printf("\n=== CALCUL DE LA MATRICE DE PEARSON ===\n");
                printf("Fichier d'entrainement : %s\n", train_file);
                printf("Calcul en cours...\n");
                
                if (matrice_similarite) {
                    printf("Libération de l'ancienne matrice...\n");
                    if (recommandeur_global) {
                        liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
                    }
                }
                
                matrice_similarite = Pearson(train_file);
                if (matrice_similarite) {
                    printf("Matrice de Pearson calculée avec succès !\n");
                    if (recommandeur_global) {
                        printf("Nombre d'utilisateurs : %d\n", recommandeur_global->nb_users);
                        printf("Nombre d'articles : %d\n", recommandeur_global->nb_articles);
                        printf("Matrice de similarité : %dx%d\n", recommandeur_global->nb_users, recommandeur_global->nb_users);
                    }
                } else {
                    printf("Erreur lors du calcul de la matrice de Pearson\n");
                }
                break;
            }

            case 2: {
                printf("\n=== TEST DE PRÉDICTIONS INDIVIDUELLES ===\n");
                if (!recommandeur_global || !recommandeur_global->matrice_similarite_calculee) {
                    printf("Erreur : Vous devez d'abord calculer la matrice de Pearson (option 1)\n");
                } else {
                    printf("Test de quelques prédictions individuelles...\n");
                    tester_predictions(recommandeur_global);
                }
                break;
            }

            case 3: {
                printf("\n=== PRÉDICTIONS SUR L'ENSEMBLE DE TEST ===\n");
                if (!recommandeur_global || !recommandeur_global->matrice_similarite_calculee) {
                    printf("Erreur : Vous devez d'abord calculer la matrice de Pearson (option 1)\n");
                } else {
                    printf("Fichier de test : %s\n", test_file);
                    printf("Calcul des prédictions en cours...\n");
                    nb_predictions = Predict_all(recommandeur_global, test_file, predictions, 10000);
                    
                    if (nb_predictions > 0) {
                        printf("Prédictions terminées avec succès !\n");
                        printf("Nombre de prédictions effectuées : %d\n", nb_predictions);
                        
                        // Afficher quelques exemples
                        printf("\nPremières prédictions :\n");
                        int nb_exemples = (nb_predictions < 5) ? nb_predictions : 5;
                        for (int i = 0; i < nb_exemples; i++) {
                            printf("User %u, Item %u -> Prédiction: %.3f, Réel: %.3f\n", 
       				predictions[i].id_user, predictions[i].id_article, 
       				predictions[i].note_predite, predictions[i].note_reelle);
                        }
                        
                    } else {
                        printf("Aucune prédiction n'a pu être effectuée.\n");
                    }
                }
                break;
            }

            case 4: {
                printf("\n=== ÉVALUATION DES PERFORMANCES ===\n");
                if (nb_predictions == 0) {
                    printf("Erreur : Vous devez d'abord effectuer les prédictions (option 3)\n");
                } else {
                    printf("Évaluation des performances en cours...\n");
                    printf("Nombre de prédictions à évaluer : %d\n", nb_predictions);
                    afficher_metriques_evaluation(predictions, nb_predictions);
                }
                break;
            }

            case 5: {
                printf("\n=== SAUVEGARDE DES RÉSULTATS ===\n");
                if (nb_predictions == 0) {
                    printf("Erreur : Vous devez d'abord effectuer les prédictions (option 3)\n");
                } else {
                    char nom_fichier[256];
                    printf("Nom du fichier de sauvegarde (par défaut: resultats_predictions.csv) : ");
                    getchar(); // Consommer le caractère de nouvelle ligne
                    if (fgets(nom_fichier, sizeof(nom_fichier), stdin) != NULL) {
                        // Supprimer le caractère de nouvelle ligne
                        nom_fichier[strcspn(nom_fichier, "\n")] = 0;
                        
                        // Utiliser le nom par défaut si rien n'est saisi
                        if (strlen(nom_fichier) == 0) {
                            strcpy(nom_fichier, "resultats_predictions.csv");
                        }
                        
                        sauvegarder_predictions(nom_fichier, predictions, nb_predictions);
                        printf("Résultats sauvegardés dans : %s\n", nom_fichier);
                        printf("Format : id_user,id_article,prediction,note_reelle\n");
                    } else {
                        printf("Erreur de saisie.\n");
                    }
                }
                break;
            }

            case 6: {
                printf("\n=== STATISTIQUES DU RECOMMANDEUR ===\n");
                if (!recommandeur_global) {
                    printf("Erreur : Vous devez d'abord calculer la matrice de Pearson (option 1)\n");
                } else {
                    afficher_stats_recommandeur(recommandeur_global);
                }
                break;
            }

            case 0: {
                // Nettoyage avant de quitter
                if (matrice_similarite && recommandeur_global) {
                    printf("Nettoyage de la mémoire...\n");
                    liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
                    liberer_recommandeur(recommandeur_global);
                    recommandeur_global = NULL;
                }
                printf("Retour au menu principal.\n");
                return;
            }

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();
        getchar();
    }
}


//ce menu me permet d'extraire les données vers Train

void extraction() {

    int ligne_debut, ligne_fin;
    printf("\n=== EXTRACTION VERS Train.txt ===\n");
    printf("Fichier source : data/donnees.txt\n");

    printf("Entrez la première ligne à extraire : ");
    scanf("%d", &ligne_debut);

    printf("Entrez la dernière ligne à extraire : ");
    scanf("%d", &ligne_fin);

    extraire_vers_fichier("data/donnees.txt", "data/KNN_TRAIN/Train.txt", ligne_debut, ligne_fin);

    printf("\nAppuyez sur Entrée pour continuer...");
    getchar(); 
    getchar();
}


