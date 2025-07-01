#ifndef RECOMMANDEUR_H
#define RECOMMANDEUR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recommandation avec les graphes */

void recommander_articles_buffer(
    int id_user,
    int nb_reco,
    GrapheBipartite* graphe,
    ResultatPageRank* pagerank,
    Transaction* transactions,
    int nb_trans,
    char* buffer_out
) 

#endif

