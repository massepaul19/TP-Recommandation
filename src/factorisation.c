#include "factorisation.h"

MatriceComplete* matrice_complete = NULL;

// Initialise la matrice de factorisation
MatriceFactorisation* init_matrice_factorisation(int M, int N, int K) {
    MatriceFactorisation* mf = malloc(sizeof(MatriceFactorisation));
    if (!mf) return NULL;
    
    mf->M = M;
    mf->N = N;
    mf->K = K;
    mf->bias_global = 0.0;
    
    mf->U = malloc(M * sizeof(double*));
    mf->V = malloc(N * sizeof(double*));
    mf->bias_u = calloc(M, sizeof(double));
    mf->bias_v = calloc(N, sizeof(double));

    for (int i = 0; i < M; i++) {
        mf->U[i] = malloc(K * sizeof(double));
        for (int k = 0; k < K; k++) {
            mf->U[i][k] = aleatoire_normal(0.0, 0.1);
        }
    }

    for (int j = 0; j < N; j++) {
        mf->V[j] = malloc(K * sizeof(double));
        for (int k = 0; k < K; k++) {
            mf->V[j][k] = aleatoire_normal(0.0, 0.1);
        }
    }

    return mf;
}

void liberer_matrice_factorisation(MatriceFactorisation* mf) {
    if (!mf) return;
    for (int i = 0; i < mf->M; i++) free(mf->U[i]);
    for (int j = 0; j < mf->N; j++) free(mf->V[j]);
    free(mf->U);
    free(mf->V);
    free(mf->bias_u);
    free(mf->bias_v);
    free(mf);
}


double predire_note(MatriceFactorisation* mf, int user, int item) {
    if (user < 0 || user >= mf->M || item < 0 || item >= mf->N)
        return mf->bias_global;

    double pred = mf->bias_global + mf->bias_u[user] + mf->bias_v[item];
    for (int k = 0; k < mf->K; k++)
        pred += mf->U[user][k] * mf->V[item][k];

    return pred;
}

void entrainer_modele(MatriceFactorisation* mf, Transaction* data, int nb_data,
                     double alpha, double lambda, int nb_iterations) {
    printf("Entraînement (alpha=%.4f, lambda=%.4f, itérations=%d)\n",
           alpha, lambda, nb_iterations);

    double somme = 0.0;
    for (int i = 0; i < nb_data; i++)
        somme += data[i].evaluation;
    mf->bias_global = somme / nb_data;

    for (int iter = 0; iter < nb_iterations; iter++) {
        for (int idx = 0; idx < nb_data; idx++) {
            int u = data[idx].id_user;
            int i = data[idx].id_article;
            double r_ui = data[idx].evaluation;
            if (u >= mf->M || i >= mf->N) continue;

            double pred = predire_note(mf, u, i);
            double err = r_ui - pred;

            mf->bias_u[u] += alpha * (err - lambda * mf->bias_u[u]);
            mf->bias_v[i] += alpha * (err - lambda * mf->bias_v[i]);

            for (int k = 0; k < mf->K; k++) {
                double u_k = mf->U[u][k];
                double v_k = mf->V[i][k];

                mf->U[u][k] += alpha * (err * v_k - lambda * u_k);
                mf->V[i][k] += alpha * (err * u_k - lambda * v_k);
            }
        }

        if (iter % 10 == 0) {
            double rmse = calculer_erreur_rmse(mf, data, nb_data);
            printf("Itération %d: RMSE = %.4f\n", iter, rmse);
        }
    }
}

double calculer_erreur_rmse(MatriceFactorisation* mf, Transaction* data, int nb_data) {
    double erreur_totale = 0.0;
    int nb = 0;
    for (int i = 0; i < nb_data; i++) {
        int u = data[i].id_user;
        int it = data[i].id_article;
        if (u >= mf->M || it >= mf->N) continue;
        double err = data[i].evaluation - predire_note(mf, u, it);
        erreur_totale += err * err;
        nb++;
    }
    return (nb > 0) ? sqrt(erreur_totale / nb) : 0.0;
}

void afficher_matrice_facteurs(MatriceFactorisation* mf) {
    printf("\n=== Matrice de Factorisation ===\n");
    printf("Dimensions: %d utilisateurs x %d items x %d facteurs\n", mf->M, mf->N, mf->K);
    printf("Biais global: %.4f\n", mf->bias_global);
    
    printf("\nPremiers facteurs utilisateurs (U):\n");
    for (int i = 0; i < (mf->M < 5 ? mf->M : 5); i++) {
        printf("User %d: bias=%.3f, facteurs=[", i, mf->bias_u[i]);
        for (int k = 0; k < mf->K; k++) {
            printf("%.3f", mf->U[i][k]);
            if (k < mf->K - 1) printf(", ");
        }
        printf("]\n");
    }
    
    printf("\nPremiers facteurs items (V):\n");
    for (int j = 0; j < (mf->N < 5 ? mf->N : 5); j++) {
        printf("Item %d: bias=%.3f, facteurs=[", j, mf->bias_v[j]);
        for (int k = 0; k < mf->K; k++) {
            printf("%.3f", mf->V[j][k]);
            if (k < mf->K - 1) printf(", ");
        }
        printf("]\n");
    }
}

MatriceComplete* MF(Transaction* train_data, int nb_train, int M, int N, int K, 
                   double alpha, double lambda, int nb_iterations) {
    
    printf("=== Fonction MF(train-data) ===\n");
    printf("Entraînement avec %d transactions sur matrice %dx%d avec %d facteurs\n", 
           nb_train, M, N, K);
    
    MatriceFactorisation* mf = init_matrice_factorisation(M, N, K);
    if (!mf) {
        printf("Erreur: Impossible d'initialiser la matrice de factorisation\n");
        return NULL;
    }
    
    entrainer_modele(mf, train_data, nb_train, alpha, lambda, nb_iterations);
    
    MatriceComplete* matrice_complete = creer_matrice_complete(M, N);
    if (!matrice_complete) {
        printf("Erreur: Impossible de créer la matrice complète\n");
        liberer_matrice_factorisation(mf);
        return NULL;
    }
    
    printf("Génération de la matrice complète de prédictions...\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrice_complete->matrice[i][j] = predire_note(mf, i, j);
        }
    }
    
    double rmse_final = calculer_erreur_rmse(mf, train_data, nb_train);
    printf("RMSE final du modèle: %.4f\n", rmse_final);
    
    liberer_matrice_factorisation(mf);
    printf("Matrice complète générée avec succès!\n\n");
    return matrice_complete;
}

double* Predict_all_MF(MatriceComplete* full_matrix, Transaction* test_data, int nb_test) {
    printf("=== Fonction Predict-all-MF(Full-Matrix, test-data) ===\n");
    printf("Prédiction pour %d transactions de test\n", nb_test);
    
    if (!full_matrix || !test_data || nb_test <= 0) {
        printf("Erreur: Paramètres invalides pour la prédiction\n");
        return NULL;
    }
    
    double* predictions = malloc(nb_test * sizeof(double));
    if (!predictions) {
        printf("Erreur: Impossible d'allouer le tableau des prédictions\n");
        return NULL;
    }
    
    int predictions_valides = 0;
    for (int i = 0; i < nb_test; i++) {
        int user = test_data[i].id_user;
        int item = test_data[i].id_article;
        
        if (user >= 0 && user < full_matrix->M && item >= 0 && item < full_matrix->N) {
            predictions[i] = full_matrix->matrice[user][item];
            predictions_valides++;
        } else {
            predictions[i] = 0.0;
            printf("Attention: Indices invalides pour transaction %d (user=%d, item=%d)\n", 
                   i, user, item);
        }
    }
    
    printf("Prédictions générées: %d valides sur %d total\n", predictions_valides, nb_test);
    return predictions;
}

MatriceComplete* creer_matrice_complete(int M, int N) {
    MatriceComplete* mc = malloc(sizeof(MatriceComplete));
    if (!mc) return NULL;
    
    mc->M = M;
    mc->N = N;
    mc->matrice = malloc(M * sizeof(double*));
    if (!mc->matrice) {
        free(mc);
        return NULL;
    }
    
    for (int i = 0; i < M; i++) {
        mc->matrice[i] = malloc(N * sizeof(double));
        if (!mc->matrice[i]) {
            for (int j = 0; j < i; j++) {
                free(mc->matrice[j]);
            }
            free(mc->matrice);
            free(mc);
            return NULL;
        }
        
        for (int j = 0; j < N; j++) {
            mc->matrice[i][j] = 0.0;
        }
    }
    
    return mc;
}

void liberer_matrice_complete(MatriceComplete* mc) {
    if (!mc) return;
    
    for (int i = 0; i < mc->M; i++) {
        free(mc->matrice[i]);
    }
    free(mc->matrice);
    free(mc);
}

void afficher_matrice_complete(MatriceComplete* mc) {
    if (!mc) {
        printf("Matrice nulle\n");
        return;
    }
    
    printf("\n=== Matrice Complète %dx%d ===\n", mc->M, mc->N);
    printf("User\\Item");
    for (int j = 0; j < (mc->N < 10 ? mc->N : 10); j++) {
        printf("%8d", j);
    }
    printf("\n");
    
    for (int i = 0; i < (mc->M < 10 ? mc->M : 10); i++) {
        printf("%8d ", i);
        for (int j = 0; j < (mc->N < 10 ? mc->N : 10); j++) {
            printf("%8.2f", mc->matrice[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

double aleatoire_normal(double mean, double std) {
    static int have_spare = 0;
    static double spare;
    
    if (have_spare) {
        have_spare = 0;
        return spare * std + mean;
    }
    
    have_spare = 1;
    double u, v, s;
    do {
        u = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        v = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);
    
    s = sqrt(-2.0 * log(s) / s);
    spare = v * s;
    return mean + std * u * s;
}

void melanger_transactions(Transaction* data, int nb_data) {
    for (int i = nb_data - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Transaction temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

void diviser_donnees(Transaction* transactions, int nb_total, 
                    Transaction** train_data, int* nb_train,
                    Transaction** test_data, int* nb_test,
                    double ratio_train) {
    
    *nb_train = (int)(nb_total * ratio_train);
    *nb_test = nb_total - *nb_train;
    
    *train_data = malloc(*nb_train * sizeof(Transaction));
    *test_data = malloc(*nb_test * sizeof(Transaction));
    
    for (int i = 0; i < *nb_train; i++) {
        (*train_data)[i] = transactions[i];
    }
    
    for (int i = 0; i < *nb_test; i++) {
        (*test_data)[i] = transactions[*nb_train + i];
    }
}

void evaluer_predictions(double* predictions, Transaction* test_data, int nb_test) {
    double erreur_totale = 0.0;
    int nb_valides = 0;
    
    printf("\n=== Évaluation des prédictions ===\n");
    printf("Comparaison réel vs prédit :\n");
    
    for (int i = 0; i < nb_test; i++) {
        double reel = test_data[i].evaluation;
        double predit = predictions[i];
        double erreur = reel - predit;
        
        printf("Test %d: User %d, Item %d, Réel=%.1f, Prédit=%.2f, Erreur=%.2f\n",
               i, test_data[i].id_user, test_data[i].id_article, 
               reel, predit, erreur);
        
        erreur_totale += erreur * erreur;
        nb_valides++;
    }
    
    if (nb_valides > 0) {
        double rmse = sqrt(erreur_totale / nb_valides);
        printf("\nRMSE sur les données de test : %.4f\n", rmse);
    }
}

Transaction* creer_donnees_test(int* nb_trans) {
    *nb_trans = 9;
    Transaction* transactions = malloc(*nb_trans * sizeof(Transaction));
    
    transactions[0] = (Transaction){0, 0, 1, 5.0, 1234567890.0};
    transactions[1] = (Transaction){0, 1, 1, 3.0, 1234567891.0};
    transactions[2] = (Transaction){0, 3, 2, 1.0, 1234567892.0};
    transactions[3] = (Transaction){1, 0, 1, 4.0, 1234567893.0};
    transactions[4] = (Transaction){1, 3, 2, 1.0, 1234567894.0};
    transactions[5] = (Transaction){2, 0, 3, 1.0, 1234567895.0};
    transactions[6] = (Transaction){2, 1, 3, 1.0, 1234567896.0};
    transactions[7] = (Transaction){2, 3, 2, 5.0, 1234567897.0};
    transactions[8] = (Transaction){3, 0, 1, 1.0, 1234567898.0};
    
    return transactions;
}


char* traiter_recommandation_matricielle(int id_user, int nb_reco) {
    static char buffer[2048];
    buffer[0] = '\0';
    
    static Transaction* train_data = NULL;
    static int nb_train = 0;
    static MatriceComplete* matrice_complete = NULL;
    static int matrice_construite = 0;
    
    if (!train_data) {
        train_data = creer_donnees_test(&nb_train);
        if (!train_data) {
            snprintf(buffer, 2048, "Erreur : Impossible de charger les données d'entraînement.\n");
            return buffer;
        }
    }
    
    if (!matrice_construite) {
        int M = 4;
        int N = 4;
        int K = 10;
        double alpha = 0.01;
        double lambda = 0.02;
        int nb_iterations = 100;
        
        matrice_complete = MF(train_data, nb_train, M, N, K, alpha, lambda, nb_iterations);
        if (!matrice_complete) {
            snprintf(buffer, 2048, "Erreur : Échec de la construction de la matrice de factorisation.\n");
            return buffer;
        }
        matrice_construite = 1;
    }
    
    int offset = snprintf(buffer, 2048, "Recommandations matricielles pour l'utilisateur %d:\n", id_user);
    
    if (id_user < 0 || id_user >= matrice_complete->M) {
        snprintf(buffer + offset, 2048 - offset, "Erreur : ID utilisateur invalide (0-%d).\n", 
                matrice_complete->M - 1);
        return buffer;
    }
    
    int* deja_evalues = calloc(matrice_complete->N, sizeof(int));
    for (int i = 0; i < nb_train; i++) {
        if (train_data[i].id_user == id_user && 
            train_data[i].id_article >= 0 && 
            train_data[i].id_article < matrice_complete->N) {
            deja_evalues[train_data[i].id_article] = 1;
        }
    }
    
    typedef struct {
        int item_id;
        double score;
    } Recommandation;
    
    Recommandation* recs = malloc(matrice_complete->N * sizeof(Recommandation));
    int nb_recs_disponibles = 0;
    
    for (int j = 0; j < matrice_complete->N; j++) {
        if (!deja_evalues[j]) {
            recs[nb_recs_disponibles].item_id = j;
            recs[nb_recs_disponibles].score = matrice_complete->matrice[id_user][j];
            nb_recs_disponibles++;
        }
    }
    
    for (int i = 0; i < nb_recs_disponibles - 1; i++) {
        for (int j = 0; j < nb_recs_disponibles - i - 1; j++) {
            if (recs[j].score < recs[j + 1].score) {
                Recommandation temp = recs[j];
                recs[j] = recs[j + 1];
                recs[j + 1] = temp;
            }
        }
    }
    
    int nb_a_afficher = (nb_reco < nb_recs_disponibles) ? nb_reco : nb_recs_disponibles;
    
    if (nb_a_afficher == 0) {
        snprintf(buffer + offset, 2048 - offset, "Aucune recommandation disponible.\n");
    } else {
        for (int i = 0; i < nb_a_afficher; i++) {
            offset += snprintf(buffer + offset, 2048 - offset, 
                             "%d. Article %d (Score: %.4f)\n", 
                             i + 1, recs[i].item_id, recs[i].score);
        }
    }
    
    free(deja_evalues);
    free(recs);
    return buffer;
}



void demander_id_et_recommandations(MatriceComplete* matrice_complete , int* id_user, int* nb_reco) {
    if (!matrice_complete) {
        printf("Erreur : matrice complète non initialisée.\n");
        *id_user = -1;
        *nb_reco = -1;
        return;
    }

    printf("\nEntrez l'ID de l'utilisateur (0-%d): ", matrice_complete->M - 1);
    while (scanf("%d", id_user) != 1 || *id_user < 0 || *id_user >= matrice_complete->M) {
        printf("ID invalide. Veuillez entrer un nombre entre 0 et %d: ", matrice_complete->M - 1);
        while (getchar() != '\n'); // Vide le buffer
    }

    printf("Entrez le nombre de recommandations à générer: ");
    while (scanf("%d", nb_reco) != 1 || *nb_reco <= 0) {
        printf("Nombre invalide. Veuillez entrer un nombre positif : ");
        while (getchar() != '\n'); // Vide le buffer
    }
}


//###############################################################################
//------------------- VERSION FACTORISATION SEULEMENT --------------------------

char* traiter_recommandation_factorisation(int id_user, int nb_reco) {
    static char resultat[1024];
    resultat[0] = '\0';
    Transaction* train_data = NULL;
    int nb_train = 0;

    // Étape 1 : Charger les données
    train_data = lire_fichier_train("Train.txt", &nb_train);
    if (!train_data || nb_train == 0) {
        snprintf(resultat, sizeof(resultat), "ERREUR: Chargement des données impossible.\n");
        return resultat;
    }
    printf("[INFO] Données chargées : %d transactions.\n", nb_train);

    // Étape 2 : Calcul des dimensions
    int M = 0, N = 0;
    for (int i = 0; i < nb_train; i++) {
        if (train_data[i].id_user > M) M = train_data[i].id_user;
        if (train_data[i].id_article > N) N = train_data[i].id_article;
    }
    M++; N++;

    // Étape 3 : Entraîner le modèle
    if (matrice_complete) {
        liberer_matrice_complete(matrice_complete);
        matrice_complete = NULL;
    }

    int K = 10;
    double alpha = 0.01;
    double lambda = 0.02;
    int nb_iterations = 100;

    printf("[INFO] Début de l'entraînement...\n");
    matrice_complete = MF(train_data, nb_train, M, N, K, alpha, lambda, nb_iterations);
    if (!matrice_complete) {
        snprintf(resultat, sizeof(resultat), "ERREUR: Entraînement du modèle échoué.\n");
        free(train_data);
        return resultat;
    }
    printf("[INFO] Modèle entraîné avec succès.\n");

    // Étape 4 : Générer recommandations
    if (id_user < 0 || id_user >= matrice_complete->M) {
        snprintf(resultat, sizeof(resultat), "ERREUR: ID utilisateur %d invalide.\n", id_user);
        free(train_data);
        return resultat;
    }

    double* scores = malloc(matrice_complete->N * sizeof(double));
    if (!scores) {
        snprintf(resultat, sizeof(resultat), "ERREUR: Mémoire insuffisante pour scores.\n");
        free(train_data);
        return resultat;
    }

    for (int i = 0; i < matrice_complete->N; i++) {
        scores[i] = matrice_complete->matrice[id_user][i];
    }

    for (int r = 0; r < nb_reco; r++) {
        int best_idx = -1;
        double best_score = -1e9;
        for (int i = 0; i < matrice_complete->N; i++) {
            if (scores[i] > best_score) {
                best_score = scores[i];
                best_idx = i;
            }
        }
        if (best_idx >= 0) {
            char ligne[128];
            snprintf(ligne, sizeof(ligne), "Article %d - score %.3f\n", best_idx, best_score);
            strncat(resultat, ligne, sizeof(resultat) - strlen(resultat) - 1);
            scores[best_idx] = -1e9;
        }
    }

    free(scores);
    free(train_data);
    return resultat;
}

