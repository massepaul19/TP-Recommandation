#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reco_KNN.h"
#include "graphe.h"

// ========== Fonction pour afficher le menu principal ==========
void afficher_menu_principal();

// ========== Fonction pour afficher les menus ==========

void Menu_Traitement();
void Menu_KNN(const char* train_file, const char* test_file);
void Menu_GRAPHE();;

// ========= Extraction des données vers Train ==========

void extraction();

#endif
