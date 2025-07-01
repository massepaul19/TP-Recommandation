#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "reco_KNN.h"
#include "menu.h"
#include "factorisation.h"

int matrice_construite = 0;

// Variables globales

static Transaction* train_data = NULL;
static int nb_train = 0;
static MatriceComplete* matrice_complete = NULL;

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
        printf("7. Test d'automatisation des calculs(Etapes)\n");
        printf("0. Menu principal\n");
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
	    
	    case 7:
            	    unsigned int id_user , nb_reco;
            	    
		    printf("Entrer l'ID Utilisateur: ");
		    scanf("%d", &id_user);
		    printf("Nombre de recommandations: ");
		    scanf("%d", &nb_reco);
		    char* resultat = traiter_recommandation_knn(id_user, nb_reco);
		    printf("\n%s\n", resultat);
            break;
	    
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


void Menu_GRAPHE() {
    int choix;
    int nb_train = 0;
    Transaction* train_data = NULL;
    GrapheBipartite graphe = {0};
    ResultatPageRank *pagerank = NULL;

    do {
        printf("\n===== MENU RECOMMANDATION - GRAPHE + PAGERANK =====\n");    
        printf("=====================================================\n");
        printf("1. Lire les données d'entraînement (Train.txt)\n");
        printf("2. Construire le graphe bipartite\n");
        printf("3. Appliquer PageRank\n");
        printf("4. Sauvegarder les résultats PageRank\n");
        printf("5. Afficher exemple de recommandations\n");
        printf("6. Test d'automatisation des calculs(Etapes)\n");
        printf("0. Nettoyer la mémoire et quitter\n");    
        printf("=====================================================\n");
        printf("Votre choix : ");
        scanf("%d", &choix);
        getchar();  // Consomme le \n laissé dans le buffer

        switch (choix) {
            case 1:
                if (train_data) free(train_data);
                train_data = lire_fichier_train("Train.txt", &nb_train);
                break;

            case 2:
                if (!train_data || nb_train == 0) {
                    printf("Vous devez d'abord lire les données !\n");
                } else {
                    creer_mappings_optimise(train_data, nb_train, &graphe);
                    construire_matrice_adjacence_optimise(train_data, nb_train, &graphe);
                    printf("\nGraphe construit avec succès.\n");
                }
                break;

            case 3:
                if (!graphe.matrice_adjacence) {
                    printf("Vous devez d'abord construire le graphe !\n");
                } else {
                    pagerank = pagerank_optimise(&graphe, 0.85, 1e-6, 50);
                    printf("\nPageRank calculé avec succès.\n");
                }
                break;

            case 4:
                if (!pagerank) {
                    printf("Vous devez d'abord exécuter PageRank !\n");
                } else {
                    sauvegarder_pagerank(&graphe, pagerank, "pagerank_results.txt");
                }
                break;
                
	    case 5:
		    if (!pagerank || graphe.nb_users == 0) {
			printf("Vous devez d'abord exécuter PageRank !\n");
		    } else {
			int id_user, nb_recommandations;
			printf("Entrez l'identifiant de l'utilisateur à recommander : ");
			scanf("%d", &id_user);
			getchar(); // Nettoyage du buffer

			if (graphe.map_users[id_user] == -1) {
			    printf("❌ L'utilisateur %d n'existe pas dans les données.\n", id_user);
			} else {
			    printf("Entrez le nombre de recommandations souhaitées : ");
			    scanf("%d", &nb_recommandations);
			    getchar(); // Nettoyage du buffer

			    recommander_articles(id_user, nb_recommandations, &graphe, pagerank, train_data, nb_train);
			}
		    }
	    break;

 	    case 6: { 
		    int id_user_test = 123; 
		    int nb_reco_test = 5;   
		    
		    printf("\nTest automatisation - Lancement du calcul...\n");
		    char *resultat = traiter_recommandation_graphe(id_user_test, nb_reco_test);
		    printf("Recommandations générées :\n%s\n", resultat);
	    break;
	    }

	    case 0:
                printf("Nettoyage mémoire...\n");
                if (graphe.matrice_adjacence) {
                    for (int i = 0; i < graphe.taille_totale; i++) {
                        free(graphe.matrice_adjacence[i]);
                    }
                    free(graphe.matrice_adjacence);
                }
                free(graphe.map_users);
                free(graphe.map_articles);
                free(graphe.reverse_map_users);
                free(graphe.reverse_map_articles);
                if (pagerank) {
                    free(pagerank->pagerank_vector);
                    free(pagerank);
                }
                if (train_data) {
                    free(train_data);
                }
                printf("Mémoire libérée. À bientôt !\n");
                break;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();

    } while (choix != 0);
}



void afficher_stats_matricielle() {
    if (!train_data || nb_train == 0) {
        printf("Aucune donnée d'entraînement chargée.\n");
        return;
    }

    int max_user = 0;
    int max_article = 0;
    double sum_ratings = 0.0;

    for (int i = 0; i < nb_train; i++) {
        if (train_data[i].id_user > max_user) max_user = train_data[i].id_user;
        if (train_data[i].id_article > max_article) max_article = train_data[i].id_article;
        sum_ratings += train_data[i].evaluation;
    }

    printf("\n=== Statistiques des données ===\n");
    printf("Nombre de transactions: %d\n", nb_train);
    printf("Nombre d'utilisateurs uniques: %d\n", max_user + 1);
    printf("Nombre d'articles uniques: %d\n", max_article + 1);
    printf("Note moyenne: %.2f\n\n", sum_ratings / nb_train);
}

void tester_systeme_matricielle() {
    if (!matrice_complete) {
        printf("\nErreur: Vous devez d'abord entraîner le modèle (option 3)\n");
        return;
    }

    int id_user, nb_reco;
    demander_id_et_recommandations(matrice_complete , &id_user, &nb_reco);

    printf("\n=== Recommandations pour l'utilisateur %d ===\n", id_user);
    char* resultat = traiter_recommandation_matricielle(id_user, nb_reco);
    if (resultat) {
        printf("%s\n", resultat);
    } else {
        printf("Erreur lors de la génération des recommandations.\n");
    }
}

void afficher_menu() {
    printf("\n============================================================\n");
    printf("=== MENU PRINCIPAL - SYSTÈME DE RECOMMANDATION ===\n");
    printf("============================================================\n");
    printf("1. Charger les données d'entraînement\n");
    printf("2. Afficher les statistiques des données\n");
    printf("3. Entraîner le modèle de factorisation matricielle\n");
    printf("4. Tester le système matriciel\n");
    printf("5. Évaluer le système complet\n");
    printf("0. Quitter\n");
    printf("============================================================\n");
}

void evaluer_systeme_complet() {
    if (!train_data || nb_train == 0) {
        printf("Aucune donnée d'entraînement chargée.\n");
        return;
    }

    Transaction* train_subset = NULL;
    Transaction* test_subset = NULL;
    int nb_train_subset = 0, nb_test_subset = 0;

    // Division des données 80/20
    diviser_donnees(train_data, nb_train, &train_subset, &nb_train_subset,
                   &test_subset, &nb_test_subset, 0.8);

    printf("\n=== Évaluation du système ===\n");
    printf("Données d'entraînement: %d transactions\n", nb_train_subset);
    printf("Données de test: %d transactions\n", nb_test_subset);

    // Trouver les dimensions maximales
    int M = 0, N = 0;
    for (int i = 0; i < nb_train_subset; i++) {
        if (train_subset[i].id_user > M) M = train_subset[i].id_user;
        if (train_subset[i].id_article > N) N = train_subset[i].id_article;
    }
    M++; N++; // Conversion d'indices en comptage

    // Entraînement du modèle
    MatriceComplete* mc = MF(train_subset, nb_train_subset, M, N, 10, 0.01, 0.02, 100);
    if (mc) {
        double* predictions = Predict_all_MF(mc, test_subset, nb_test_subset);
        if (predictions) {
            evaluer_predictions(predictions, test_subset, nb_test_subset);
            free(predictions);
        }
        liberer_matrice_complete(mc);
    }

    // Libération mémoire
    free(train_subset);
    free(test_subset);
}

void Menu_Factorisation() {
    int choix;
    do {
        afficher_menu();
        printf("Votre choix: ");
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Veuillez entrer un nombre.\n");
            while (getchar() != '\n'); // Vide le buffer
            continue;
        }

        switch (choix) {
            case 1: // Charger les données
                if (train_data) {
                    free(train_data);
                    train_data = NULL;
                    nb_train = 0;
                }
                train_data = lire_fichier_train("Train.txt", &nb_train);
                if (train_data) {
                    printf("Données chargées avec succès (%d transactions).\n", nb_train);
                } else {
                    printf("Erreur lors du chargement des données.\n");
                }
                break;
                
            case 2: // Afficher statistiques
                afficher_stats_matricielle();
                break;
                
            case 3: { // Entraîner modèle
                if (!train_data || nb_train == 0) {
                    printf("Veuillez d'abord charger les données.\n");
                    break;
                }
                
                // Trouver les dimensions maximales
                int M = 0, N = 0;
                for (int i = 0; i < nb_train; i++) {
                    if (train_data[i].id_user > M) M = train_data[i].id_user;
                    if (train_data[i].id_article > N) N = train_data[i].id_article;
                }
                M++; N++; // Conversion d'indices en comptage
                
                // Paramètres du modèle
                int K = 10;
                double alpha = 0.01;
                double lambda = 0.02;
                int nb_iterations = 100;
                
                // Nettoyer l'ancien modèle si existant
                if (matrice_complete) {
                    liberer_matrice_complete(matrice_complete);
                    matrice_complete = NULL;
                }
                
                // Entraînement
                printf("Début de l'entraînement du modèle...\n");
                matrice_complete = MF(train_data, nb_train, M, N, K, alpha, lambda, nb_iterations);
                if (matrice_complete) {
                    printf("Modèle entraîné avec succès!\n");
                } else {
                    printf("Erreur lors de l'entraînement du modèle.\n");
                }
                break;
            }
                
            case 4: // Tester système
                tester_systeme_matricielle();
                break;
                
            case 5: // Évaluer système
                evaluer_systeme_complet();
                break;
                
            case 0: // Quitter
                printf("Nettoyage de la mémoire...\n");
                if (train_data) {
                    free(train_data);
                    train_data = NULL;
                    nb_train = 0;
                }
                if (matrice_complete) {
                    liberer_matrice_complete(matrice_complete);
                    matrice_complete = NULL;
                }
                printf("Au revoir!\n");
                break;
                
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
}

