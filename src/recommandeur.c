#include "recommanduer.h"

/* Recommandation avec les graphes */

void recommander_articles_buffer(
    int id_user,
    int nb_reco,
    GrapheBipartite* graphe,
    ResultatPageRank* pagerank,
    Transaction* transactions,
    int nb_trans,
    char* buffer_out
) {
    int user_idx = graphe->map_users[id_user];
    if (user_idx == -1) {
        sprintf(buffer_out, " Utilisateur %d inconnu dans les données.\n", id_user);
        return;
    }

    // Marquer les articles déjà vus
    char* deja_vus = calloc(graphe->max_article_id + 1, sizeof(char));
    for (int i = 0; i < nb_trans; i++) {
        if (transactions[i].id_user == id_user) {
            deja_vus[transactions[i].id_article] = 1;
        }
    }

    // Préparer les scores non vus
    typedef struct {
        int id_article;
        float score;
    } ArticleScore;

    ArticleScore* scores = malloc(sizeof(ArticleScore) * graphe->nb_articles);
    int nb_scores = 0;

    for (int i = 0; i < graphe->nb_articles; i++) {
        int id_article = graphe->reverse_map_articles[i];
        if (!deja_vus[id_article]) {
            int global_idx = graphe->nb_users + i;
            scores[nb_scores].id_article = id_article;
            scores[nb_scores].score = pagerank->pagerank_vector[global_idx];
            nb_scores++;
        }
    }

    // Trier par score décroissant
    int comparer(const void* a, const void* b) {
        float diff = ((ArticleScore*)b)->score - ((ArticleScore*)a)->score;
        return (diff > 0) - (diff < 0);
    }
    qsort(scores, nb_scores, sizeof(ArticleScore), comparer);

    // Remplir buffer_out
    sprintf(buffer_out, "Recommandations pour l'utilisateur %d :\n", id_user);
    char ligne[128];
    for (int i = 0; i < nb_scores && i < nb_reco; i++) {
        snprintf(ligne, sizeof(ligne), " - Article %d (score: %.5f)\n", scores[i].id_article, scores[i].score);
        strcat(buffer_out, ligne);
    }

    // Nettoyage
    free(deja_vus);
    free(scores);
}

