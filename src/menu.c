#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "reco_KNN.h"
#include "menu.h"

// Déclaration de la variable globale

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


void Menu_KNN(const char* train_file, const char* test_file) {

    double** matrice_similarite = NULL;
    Prediction predictions[10000];
    int nb_predictions = 0;
    int choix;
    
    // Variables pour les fichiers (modifiables localement)
    char fichier_train[256];
    char fichier_test[256];
    
    // Initialiser avec les paramètres ou les valeurs par défaut
    if (train_file != NULL) {
        strcpy(fichier_train, train_file);
    } else {
        strcpy(fichier_train, "data/KNN_TRAIN/Train.txt");
    }
    
    if (test_file != NULL) {
        strcpy(fichier_test, test_file);
    } else {
        strcpy(fichier_test, "data/KNN_TRAIN/Test.txt");
    }
    
    while (1) {
        printf("\n\n=== MENU SYSTÈME KNN ===\n\n");
        printf("=========================================\n");
        printf("Fichier train : %s\n", fichier_train);
        printf("Fichier test  : %s\n", fichier_test);
        printf("\nOptions :\n");
        printf("1. Calculer la matrice de Pearson\n");
        printf("2. Tester des prédictions individuelles Pearson(u , i)\n");
        printf("3. Effectuer toutes les prédictions sur le fichier de test\n");
        printf("4. Évaluer les performances\n");
        printf("5. Sauvegarder les résultats\n");
        printf("6. Afficher les statistiques\n");
        printf("7. Changer les fichiers d'entrée\n");
        printf("0. Retour au menu principal\n");
        printf("=========================================\n");
        printf("Votre choix : ");
      
	printf("Votre choix : ");
	char buffer[32]; // petit buffer suffisant
	if (!fgets(buffer, sizeof(buffer), stdin)) {
	    printf("Saisie invalide (entrée vide)!\n");
	    continue;
	}

	if (sscanf(buffer, "%d", &choix) != 1) {
	    printf("Saisie invalide (entrez un nombre entre 0 et 7)\n");
	    continue;
	}

        switch (choix) {
            case 1: {
                printf("\n=== CALCUL DE LA MATRICE DE PEARSON ===\n");
                printf("Fichier d'entrainement : %s\n", fichier_train);
                printf("Calcul en cours...\n");
                
                // Libérer l'ancienne matrice si elle existe
                if (matrice_similarite && recommandeur_global) {
                    printf("Libération de l'ancienne matrice...\n");
                    liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
                    matrice_similarite = NULL;
                }
                
                // Libérer l'ancien recommandeur si il existe
                if (recommandeur_global) {
                    liberer_recommandeur(recommandeur_global);
                    recommandeur_global = NULL;
                }
                
                // Calculer la nouvelle matrice
                printf("DEBUG MENU: Appel de la fonction Pearson\n");
                matrice_similarite = Pearson(fichier_train);
                printf("DEBUG MENU: Fonction Pearson terminée, matrice_similarite = %p\n", (void*)matrice_similarite);
                printf("DEBUG MENU: recommandeur_global = %p\n", (void*)recommandeur_global);
                
                // Vérification détaillée
                if (matrice_similarite != NULL) {
                    printf("DEBUG MENU: Matrice non-NULL\n");
                    if (recommandeur_global != NULL) {
                        printf("✓ Matrice de Pearson calculée avec succès !\n");
                        printf("  Nombre d'utilisateurs : %d\n", recommandeur_global->nb_users);
                        printf("  Nombre d'articles : %d\n", recommandeur_global->nb_articles);
                        printf("  Matrice de similarité : %dx%d\n", 
                               recommandeur_global->nb_users, recommandeur_global->nb_users);
                        printf("  Nombre de transactions : %d\n", recommandeur_global->nb_transactions);
                        
                        // Vérifier quelques valeurs de la matrice pour debug
                        printf("  Exemple de corrélations : (0,0)=%.3f", matrice_similarite[0][0]);
                        if (recommandeur_global->nb_users > 1) {
                            printf(", (0,1)=%.3f", matrice_similarite[0][1]);
                        }
                        printf("\n");
                    } else {
                        printf("✗ Erreur : Matrice calculée mais recommandeur global NULL\n");
                        // Estimer la taille pour libérer (utiliser une valeur par défaut)
                        printf("DEBUG: Tentative de libération de matrice orpheline\n");
                        // Ne pas libérer ici car on ne connaît pas la taille exacte
                        matrice_similarite = NULL;
                    }
                } else {
                    printf("✗ Erreur lors du calcul de la matrice de Pearson\n");
                    printf("  La fonction Pearson a retourné NULL\n");
                }
                break;
            }

            case 2: {
                printf("\n=== TEST DE PRÉDICTIONS INDIVIDUELLES ===\n");
                
                if (!recommandeur_global || !matrice_similarite) {
                    printf("✗ Erreur : Calculez d'abord la matrice de Pearson (option 1)\n");
                    break;
                }
                
                unsigned int user, item;
                printf("Entrez user_id et item_id (ex: 98 1088) : ");
                
                if (scanf("%u %u", &user, &item) == 2) {
                    // Vérifier que les IDs existent
                    int user_idx = trouver_index_utilisateur(recommandeur_global, user);
                    int item_idx = trouver_index_article(recommandeur_global, item);
                    
                    if (user_idx == -1) {
                        printf("✗ Erreur : Utilisateur %u non trouvé dans les données\n", user);
                    } else if (item_idx == -1) {
                        printf("✗ Erreur : Article %u non trouvé dans les données\n", item);
                    } else {
                        printf("Calcul de la prédiction...\n");
                        float pred = Predict(recommandeur_global, user, item);
                        float note_reelle = recommandeur_global->matrice_evaluations[user_idx][item_idx];
                        
                        printf("Résultat :\n");
                        printf("  User ID : %u (index: %d)\n", user, user_idx);
                        printf("  Item ID : %u (index: %d)\n", item, item_idx);
                        printf("  Prédiction : %.3f\n", pred);
                        
                        if (note_reelle > 0) {
                            printf("  Note réelle : %.3f\n", note_reelle);
                            float erreur = fabs(pred - note_reelle);
                            printf("  Erreur absolue : %.3f\n", erreur);
                        } else {
                            printf("  Note réelle : Non évaluée\n");
                        }
                    }
                } else {
                    printf("✗ Saisie invalide! Format attendu : 'ID_USER ID_ITEM'\n");
                }
                break;
            }

            case 3: {
                printf("\n=== PRÉDICTIONS SUR L'ENSEMBLE DE TEST ===\n");
                
                if (!recommandeur_global || !matrice_similarite) {
                    printf("✗ Erreur : Calculez d'abord la matrice de Pearson (option 1)\n");
                    break;
                }
                
                printf("Fichier de test : %s\n", fichier_test);
                printf("Calcul des prédictions en cours...\n");
                
                nb_predictions = Predict_all(recommandeur_global, fichier_test, predictions, 10000);
                
                if (nb_predictions > 0) {
                    printf("✓ Prédictions terminées avec succès !\n");
                    printf("  Nombre de prédictions effectuées : %d\n", nb_predictions);
                    
                    // Afficher quelques exemples
                    printf("\nPremières prédictions :\n");
                    int nb_exemples = (nb_predictions < 5) ? nb_predictions : 5;
                    for (int i = 0; i < nb_exemples; i++) {
                        printf("  User %u, Item %u -> Prédiction: %.3f, Réel: %.3f\n", 
                               predictions[i].id_user, predictions[i].id_article, 
                               predictions[i].note_predite, predictions[i].note_reelle);
                    }
                    
                } else {
                    printf("✗ Aucune prédiction n'a pu être effectuée.\n");
                }
                break;
            }

            case 4: {
                printf("\n=== ÉVALUATION DES PERFORMANCES ===\n");
                
                if (nb_predictions == 0) {
                    printf("✗ Erreur : Effectuez d'abord les prédictions (option 3)\n");
                    break;
                }
                
                printf("Évaluation des performances en cours...\n");
                printf("Nombre de prédictions à évaluer : %d\n", nb_predictions);
                
                // Calcul des métriques
                double mae = calculer_mae(predictions, nb_predictions);
                double rmse = calculer_rmse(predictions, nb_predictions);
                
                printf("\n=== RÉSULTATS D'ÉVALUATION ===\n");
                printf("MAE  (Mean Absolute Error) : %.3f\n", mae);
                printf("RMSE (Root Mean Square Error) : %.3f\n", rmse);
                
                afficher_metriques_evaluation(predictions, nb_predictions);
                break;
            }

            case 5: {
		    printf("\n=== SAUVEGARDE DES RÉSULTATS ===\n");
		    
		    if (nb_predictions == 0) {
			printf("✗ Erreur : Effectuez d'abord les prédictions (option 3)\n");
			break;
		    }
		    
		    // Chemin de sauvegarde fixe
		    const char* chemin_fichier = "data/KNN_TRAIN/resultats_predictions.txt";
		    
		    printf("Sauvegarde en cours dans : %s\n", chemin_fichier);
		    
		    sauvegarder_predictions(chemin_fichier, predictions, nb_predictions);
		    
		    printf("✓ Résultats sauvegardés avec succès dans : %s\n", chemin_fichier);
		    break;
		}


            case 6: {
                printf("\n=== STATISTIQUES DU RECOMMANDEUR ===\n");
                
                if (!recommandeur_global) {
                    printf("✗ Erreur : Calculez d'abord la matrice de Pearson (option 1)\n");
                    break;
                }
                
                printf("=== INFORMATIONS GÉNÉRALES ===\n");
                printf("Nombre d'utilisateurs : %d\n", recommandeur_global->nb_users);
                printf("Nombre d'articles : %d\n", recommandeur_global->nb_articles);
                printf("Nombre d'évaluations : %d\n", recommandeur_global->nb_transactions);
                
                // Statistiques supplémentaires
                if (matrice_similarite) {
                    printf("Matrice de similarité : Calculée (%dx%d)\n", 
                           recommandeur_global->nb_users, recommandeur_global->nb_users);
                } else {
                    printf("Matrice de similarité : Non calculée\n");
                }
                
                if (nb_predictions > 0) {
                    printf("Prédictions disponibles : %d\n", nb_predictions);
                }
                
                // Utiliser recommandeur_global au lieu de rec
                afficher_stats_recommandeur(recommandeur_global);
                break;
            }

            case 7: {
                printf("\n=== MODIFICATION DES FICHIERS ===\n");
                char new_train[256], new_test[256];
                
                printf("Fichier train actuel : %s\n", fichier_train);
                printf("Nouveau fichier train (ou Entrée pour garder l'actuel) : ");
                if (scanf("%255s", new_train) == 1 && strlen(new_train) > 0) {
                    strcpy(fichier_train, new_train);
                    printf("✓ Fichier train mis à jour : %s\n", fichier_train);
                    
                    // Invalider la matrice car le fichier d'entrainement a changé
                    if (matrice_similarite) {
                        printf("⚠ La matrice de Pearson devra être recalculée.\n");
                    }
                }
                
                printf("Fichier test actuel : %s\n", fichier_test);
                printf("Nouveau fichier test (ou Entrée pour garder l'actuel) : ");
                if (scanf("%255s", new_test) == 1 && strlen(new_test) > 0) {
                    strcpy(fichier_test, new_test);
                    printf("✓ Fichier test mis à jour : %s\n", fichier_test);
                    
                    // Invalider les prédictions car le fichier de test a changé
                    if (nb_predictions > 0) {
                        printf("⚠ Les prédictions devront être recalculées.\n");
                        nb_predictions = 0;
                    }
                }
                break;
            }

            case 0: {
                printf("\n=== NETTOYAGE ET SORTIE ===\n");
                
                // Nettoyage avant de quitter
                if (matrice_similarite && recommandeur_global) {
                    printf("Libération de la matrice de similarité...\n");
                    liberer_matrice_similarite(matrice_similarite, recommandeur_global->nb_users);
                }
                
                if (recommandeur_global) {
                    printf("Libération du recommandeur...\n");
                    liberer_recommandeur(recommandeur_global);
                    recommandeur_global = NULL;
                }
                
                printf("✓ Nettoyage terminé. Retour au menu principal.\n");
                return;
            }

            default:
                printf("✗ Choix invalide (%d). Veuillez réessayer.\n", choix);
                break;
        }

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();
        getchar(); // Pour capturer l'Entrée après scanf
    }
}

