#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "reco.h"

int main() {
    User users[MAX_USERS] = {0};
    Article articles[MAX_ARTICLES] = {0};
    Categorie categories[MAX_CATEGORIES] = {0};
    Transaction transactions[MAX_ARTICLES];

    int nb_users = 0;
    int nb_articles = 0;
    int nb_categories = 0;
    int nb_transactions = 0;
    
    // Chargement initial du fichier
    
    FILE* file = fopen("data/donnees.txt", "r");
    if (file == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier donnees.txt\n");
        return 1;
    }

    Transaction t;
    while (fscanf(file, "%d %d %d %f %lf", &t.id_user, &t.id_article, &t.id_cat, &t.evaluation, &t.timestamp) == 5) {
        ajouter_transaction(users, &nb_users, articles, &nb_articles,
                            categories, &nb_categories, transactions, &nb_transactions, t);
    }
    fclose(file);

    // Sauvegarde des données
    
    ecrire_users("data/Users.txt", users, nb_users);
    ecrire_articles("data/Articles.txt", articles, nb_articles);
    ecrire_categories("data/Categories.txt", categories, nb_categories);
    ecrire_transactions("data/donnees.txt", transactions, nb_transactions);

    int choix;

    while (1) {
        printf("\n\n=== MENU PRINCIPAL ===\n\n");
        printf("1. Afficher les statistiques\n");
        printf("2. Extraire les transactions entre deux dates\n");
        printf("3. Filtrer les transactions \n");
        printf("4. Nettoyer les données de test \n");
        printf("0. Quitter\n");
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
	    
	    case 3:{
	    	unsigned int minU , minI;
	    	printf("\nEntrer le nombre min d'users: ");
	    	scanf("%d",&minU);
	    	printf("Entrer le nombre min d'articles: ");
	    	scanf("%d",&minI);
	    	
	    	int result = filtrer_transactions("data/donnees.txt", "data/transactions_filtrees.txt", minU, minI);

		    if (result == -1) {
			printf("Erreur lors du filtrage des transactions.\n");
			return 1;
		    } else if (result == 0) {
			printf("Aucune transaction n'a passé les filtres.\n");
		    } else {
			printf("Filtrage terminé avec succès ! ");
		    }
		 break;
	    }
	    
	    case 4: {
	    	int nb_trans = nettoyer_fichier_test("data/essai/Train.txt", "data/essai/Test.txt", "data/essai/Clean.txt");
	    	
	    	if (nb_trans > 0 )
		    	printf("Nous avons %d transactions extraites", nb_trans);
 		else
 			printf("Pas de transaction extraite");
 		break;	    	
	    }

            case 0:
                printf("Fin du programme. Merci !\n");
                return 0;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar(); // vide le buffer après scanf
        getchar(); // attend la touche entrée
    }

    return 0;
}

