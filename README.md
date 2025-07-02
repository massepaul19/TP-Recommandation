# RECOMMANDATION

Un système de recommandation complet implémenté en C, utilisant l'algorithme des k plus proches voisins (KNN) avec la corrélation de Pearson pour prédire les préférences des utilisateurs.

## Par rapport au systeme 

Ici nous avons choisi d'implementer 02 tests : 
- le test en localhost : qui consiste au client et au serveur d'être dans une même machine et de fonctionner ensemble
- le test en résau : ici nous utilisons le point d'acces pour mettre les machines (serveur + les clients ) en réseau et au préalable nous lançons l'execution côté client sur les machines clientes et nous démarrons le serveur

###Gestion des erreurs et conflits : acces à la ressource par plusieurs clients

Nous utilisons les threads pour pouvoir gérer ...
## 1. TRAITEMENT

## Resumé Makefile

Pour faire simple, pour savoir comment executer le projet taper la commande make help

### -------------------------------------------
### ===== Aide ==========
### -------------------------------------------

help:
	@echo '=== MENU D''AIDE POUR LA COMPILATION  Paulo Masse ======'
	@echo 'make               - compiler tout le programme'
	@echo 'make run           - Executer le programme'
	@echo 'make runstatic     - Executer avec Biblio statique'
	@echo 'make rundyn        - Executer avec Biblio dynamique'
	@echo 'make rundynessai   - Executer avec 03 Biblios dynamique'
	@echo 'make serveur       - Demarrer avec le serveur'
	@echo 'make stop-serveur  - Arrêter le serveur'
	@echo 'make client        - Demarrer côté client'
	@echo 'make adresse       - Voir les adresses disponibles'
	@echo 'make clean         - Nettoyage des fichiers'
	@echo 'make efface        - Nettoyage du terminal'


### Structure des Données

Le système utilise plusieurs structures pour gérer les données :
Plus d'informations dans chaque fichier d'entete sur les fonctions implementées.
Mais les autres structures utilisent la structure principale inclus grace à #include "reco.h"

structure principale

```c
typedef struct {
    unsigned int id_user;
    int nb_fois;
} User;

typedef struct {
    unsigned int id_article;
    unsigned int id_cat;
    int nb_fois;
} Article;

typedef struct {
    unsigned int id_cat;
    int nb_fois;
} Categorie;

typedef struct {
    int id_user;
    int id_article;
    int id_cat;
    float evaluation;
    double timestamp;
} Transaction;
```

structure pour gerer KNN

```
typedef struct {
    User users[MAX_USERS];
    Article articles[MAX_ARTICLES];
    Transaction transactions[MAX_USERS * MAX_ARTICLES];
    
    int nb_users;
    int nb_articles;
    int nb_transactions;
    
    float matrice_evaluations[MAX_USERS][MAX_ARTICLES];
    double matrice_similarite[MAX_USERS][MAX_USERS];
    
    double moyenne_utilisateur[MAX_USERS];
    int nb_eval_utilisateur[MAX_USERS];
    
    int k;
    int donnees_chargees;
    int matrice_similarite_calculee;
} RecommandeurKNN;

typedef struct {
    unsigned int id_user;
    unsigned int id_article;
    float note_predite;
    float note_reelle;
    double confiance;
} Prediction;

typedef struct {
    int index_utilisateur;
    unsigned int id_utilisateur;
    double similarite;
} Voisin;

```

structure pour gerer GRAPHE
```
typedef struct {
    int **matrice_adjacence;
    int taille_totale;
    int nb_users;
    int nb_articles;
    int *map_users;
    int *map_articles;
    int *reverse_map_users;
    int *reverse_map_articles;
    int max_user_id;
    int max_article_id;
} GrapheBipartite;

typedef struct {
    float *pagerank_vector;
    int nb_iterations;
    float convergence;
} ResultatPageRank;
```

### Structure par rapport au client serveur


##Fonctionnalités

### Fonctionnalités de Traitement

1. Afficher les statistiques
2. Extraire les transactions entre deux dates
3. Nettoyer les données de test
4. Filtrer les données
5. Recharger les données depuis donnees.txt
6. Sauvegarder les structures séparées
0. Retour au menu principal

### Fonctionnalités extraction


###Système de recommandation KNN

Options :
1. Calculer la matrice de Pearson
2. Tester des prédictions individuelles Pearson(u , i)
3. Effectuer toutes les prédictions sur le fichier de test
4. Évaluer les performances
5. Sauvegarder les résultats
6. Afficher les statistiques


###Système de recommandation Factorisation Matricielle



###Système de recommandation avec Graphes

===== MENU RECOMMANDATION - GRAPHE + PAGERANK =====

Options :
1. Lire les données d'entraînement (Train.txt)
2. Construire le graphe bipartite
3. Appliquer PageRank
4. Sauvegarder les résultats PageRank
5. Afficher exemple de recommandations
0. Nettoyer la mémoire et quitter


1. Traitement et gestion des données                           ║
║  2. Extraction de données vers Train                         ║
║  3. Système de recommandation KNN                            ║
║  4. Système de recommandation Factorisation Matricielle      ║
║  5. Système de recommandation avec Graphes                   ║
║  6. À propos du système                                      ║
║  0. Quitter le programme 

### Fichiers de Traitement

- `include/reco.h` : Définitions des structures et fonctions de base
- `src/reco.c` : Implémentation des fonctions de traitement  
- `include/Algo_recommandation.h` : Interface pour les 3 tâches KNN
- `src/Algo_recommandation.c` : Implémentation des algorithmes KNN
- Limites : `MAX_USERS = 50`, `MAX_ARTICLES = 50`, `MAX_CATEGORIES = 50`

## 2. RECOMMANDATION KNN

### Principe de l'Algorithme

L'algorithme KNN (k-Nearest Neighbors) pour la recommandation fonctionne en trois étapes :

1. **Calcul de similarité** : Mesure la similitude entre utilisateurs
2. **Sélection des voisins** : Identifie les k utilisateurs les plus similaires
3. **Prédiction** : Estime les notes basées sur les préférences des voisins

### Structure Principale

```c
typedef struct {
    User users[MAX_USERS];
    Article articles[MAX_ARTICLES];
    Transaction transactions[MAX_USERS * MAX_ARTICLES];
    
    int nb_users;
    int nb_articles;
    int nb_transactions;
    
    float matrice_evaluations[MAX_USERS][MAX_ARTICLES];
    double matrice_similarite[MAX_USERS][MAX_USERS];
    double moyenne_utilisateur[MAX_USERS];
    
    int k;  // Nombre de voisins
} RecommandeurKNN;
```

### Trois Tâches Principales

#### 🎯 **TÂCHE 1 : Pearson(train-data)**
```c
double** Pearson(const char* filename);
```
- **Entrée** : Fichier de données d'entraînement
- **Sortie** : Matrice carrée de similarité entre utilisateurs
- **Objectif** : Construire le modèle de recommandation

**Processus :**
1. Chargement des transactions d'entraînement
2. Construction de la matrice utilisateur-article
3. Calcul des moyennes par utilisateur
4. Calcul de la corrélation de Pearson entre tous les utilisateurs

**Formule de Pearson :**
```
Pearson(u,v) = Σ(rui - r̄u)(rvi - r̄v) / √[Σ(rui - r̄u)² × Σ(rvi - r̄v)²]
```

#### 🎯 **TÂCHE 2 : Predict(ui)**
```c
float Predict(RecommandeurKNN* rec, unsigned int id_user, unsigned int id_article);
```
- **Entrée** : ID utilisateur et ID article
- **Sortie** : Note prédite pour cet utilisateur sur cet article
- **Objectif** : Prédiction individuelle

**Processus :**
1. Identification des k voisins les plus similaires
2. Sélection des voisins ayant noté l'article
3. Calcul de la prédiction pondérée par la similarité

**Formule de prédiction :**
```
P(u,i) = r̄u + Σ(sim(u,v) × (rvi - r̄v)) / Σ|sim(u,v)|
```

#### 🎯 **TÂCHE 3 : Predict-all(test-data)**
```c
int Predict_all(RecommandeurKNN* rec, const char* test_filename, 
                Prediction predictions[], int max_predictions);
```
- **Entrée** : Fichier de données de test
- **Sortie** : Ensemble de toutes les prédictions
- **Objectif** : Évaluation complète du modèle

**Métriques d'évaluation :**
- **RMSE** : `√(Σ(prédite - réelle)² / n)`
- **MAE** : `Σ|prédite - réelle| / n`
- **Précision** : Pourcentage de prédictions dans une marge d'erreur

### Structures Auxiliaires

```c
typedef struct {
    unsigned int id_user;
    unsigned int id_article;
    float note_predite;
    double confiance;
} Prediction;

typedef struct {
    int index_utilisateur;
    unsigned int id_utilisateur;
    double similarite;
} Voisin;
```

## 3. UTILISATION

### Compilation

Le projet utilise un Makefile pour la compilation :

```bash
# Compilation complète
make

# Compilation version statique
make static

# Compilation version dynamique  
make dynamic

# Nettoyage
make clean

# Exécution
./bin/cisse.out
```

### Bibliothèques Disponibles

Au choix.
- **libreco.a** : Bibliothèque statique contenant les fonctions de traitement
- **libreco.so** : Bibliothèque dynamique pour une utilisation modulaire

Pour des tests sans le serveur ni  client, nous avons proposé à l'utilisateur 03 méthodes d'éxécution 
Mais, dans le cadre du serveur et client nous avons utilisé la bibliotheque dynamique
Pourquoi ? parcequ'elle consomme moins et elle est chargée à l'éxécution ce qui nous fait un gain

### Exemple d'Utilisation Complète

#

## 4. FORMAT DES DONNÉES

### Fichier de Transactions (data/essai/Train.txt, Test.txt , data/données.txt)
```
id_user id_article id_categorie evaluation timestamp
1       101        5           4.5        1609459200.0
1       102        3           3.0        1609545600.0
2       101        5           5.0        1609632000.0
```

data/données.txt : est notre fichier contenant des données brutes que nous avons pu génerer à partir de script python 
data/essai/Train.txt , data/Train_KNN/Train.txt contient une partie de données de tests recupérées dans nos données brutes

### Fichiers Auxiliaires

- `data/Users.txt` : Liste des utilisateurs extraits
- `data/Articles.txt` : Liste des articles extraits  
- `data/Categories.txt` : Liste des catégories
- `data/donnees.txt` : Données brutes complètes
- `data/transactions_filtrees.txt` : Données après filtrage par fréquence

### Fichier de Sortie des Prédictions
```
id_user id_article note_reelle note_predite confiance
1       103        4.0         3.8          0.85
2       104        3.5         3.2          0.72
```

## 5. PARAMÈTRES ET CONFIGURATION

### Paramètres Principaux
- **k** : Nombre de voisins (recommandé : 5-20)
- **Seuil de similarité** : Minimum pour considérer un voisin
- **Taille maximale** : Définie par les constantes MAX_*

### Optimisations Possibles
- **Filtrage préalable** : Suppression des utilisateurs/articles peu actifs
- **Normalisation** : Centrage des notes par utilisateur
- **Pondération temporelle** : Prise en compte de l'ancienneté des évaluations

## 6. STRUCTURE DU PROJET

```
.
├── bin/                        # Exécutables compilés
│   ├── cisse.out              # Exécutable principal
│   ├── rundyn                 # Version dynamique
│   └── runstatic              # Version statique
├── data/                      # Données du projet
│   ├── Articles.txt           # Liste des articles
│   ├── Categories.txt         # Liste des catégories
│   ├── Users.txt              # Liste des utilisateurs
│   ├── donnees.txt            # Données brutes complètes
│   ├── transactions_filtrees.txt  # Données filtrées
│   ├── transactions_t1-t2.txt     # Données par période
│   └── essai/                 # Jeu de données d'essai
│       ├── Train.txt          # Données d'entraînement
│       ├── Test.txt           # Données de test
│       └── Clean.txt          # Données nettoyées
├── include/                   # Fichiers d'en-tête
│   ├── reco.h                # Structures de base et traitement
│   ├── Algo_recommandation.h # Interface KNN (vos 3 tâches)
│   └── menu.h                # Interface utilisateur
├── src/                      # Code source
│   ├── reco.c                # Implémentation du traitement
│   ├── Algo_recommandation.c # Implémentation KNN
│   └── menu.c                # Interface utilisateur
├── lib/                      # Bibliothèques
│   ├── libreco.a             # Bibliothèque statique
│   └── libreco.so            # Bibliothèque dynamique
├── obj/                      # Fichiers objets compilés
│   ├── main.o
│   └── reco.o
├── test/                     # Tests et programme principal
│   └── main.c                # Point d'entrée du programme
├── Makefile                  # Script de compilation
└── TP3-INF 3621_Programmation Système.pdf  # Documentation du TP
```

.
├── bin
│   ├── cisse.out
│   ├── rundyn
│   └── runstatic
├── data
│   ├── Articles.txt
│   ├── Categories.txt
│   ├── donnees.txt
│   ├── essai
│   │   ├── Clean.txt
│   │   ├── Test.txt
│   │   └── Train.txt
│   ├── KNN_TRAIN
│   │   ├── resultats_predictions.txt
│   │   ├── Test.txt
│   │   └── Train.txt
│   ├── transactions_filtrees.txt
│   ├── transactions_t1-t2.txt
│   └── Users.txt
├── données.txt
├── genere.py
├── include
│   ├── factorisation.h
│   ├── graphe.h
│   ├── menu.h
│   ├── reco.h
│   └── reco_KNN.h
├── lib
│   ├── librecommantion.a
│   └── librecommantion.so
├── Makefile
├── network
│   ├── client.c
│   ├── include
│   │   ├── fonctions_client.h
│   │   └── fonctions_serveur.h
│   ├── serveur.c
│   └── src
│       ├── fonctions_client.c
│       └── fonctions_serveur.c
├── obj
│   ├── graphe.o
│   ├── graphe_pic.o
│   ├── main.o
│   ├── menu.o
│   ├── reco_KNN.o
│   ├── reco_KNN_pic.o
│   ├── reco.o
│   └── reco_pic.o
├── pagerank_results.txt
├── projet
│   ├── include
│   │   └── factorisation.h
│   ├── Makefile
│   ├── network
│   │   ├── client.c
│   │   ├── include
│   │   │   ├── fonctions_client.h
│   │   │   └── fonctions_serveur.h
│   │   ├── serveur.c
│   │   └── src
│   │       ├── fonctions_client.c
│   │       └── fonctions_serveur.c
│   ├── src
│   │   └── factorisation.c
│   └── test
│       └── main.c
├── README.md
├── reseau
├── src
│   ├── factorisation.c
│   ├── graphe.c
│   ├── menu.c
│   ├── reco.c
│   └── reco_KNN.c
├── test
│   └── main.c
├── test.c
├── Text.txt
├── TP3-INF 3621_ Programmation Système.pdf
├── Tp-recommandation_sockets.pdf
└── Train.txt

## 7. RÉFÉRENCES

- **Algorithme KNN** : Collaborative Filtering Recommender Systems
- **Corrélation de Pearson** : Mesure de similarité statistique
- **Évaluation** : Métriques RMSE et MAE pour les systèmes de recommandation

---

*Développé dans le cadre d'un projet de système de recommandation utilisant l'algorithme des k plus proches voisins avec corrélation de Pearson.*

## 7. Structures : Pourquoi avons nous choisi celà ?

Dans le traitement, le code utilise une fonction ecriretransaction telle que lorsque quelles est appelées sur un fichier, celle si mets à jour tous les autres fichiers

Nous avons créé plusieurs autres fichiers adptés à notre structure afin de répondre plus facilement aux questions du traitement qui est la base de notre travail

## Explication des fichiers objets

Nous avons respectivement 

DOSSIER_OBJET = ./obj/main.o ./obj/reco.o ./obj/reco_KNN.o ./obj/menu.o ./obj/graphe.o   #ici c'est pour lancer l'executable principal
OBJET_biblio_dyn = ./obj/reco_pic.o ./obj/reco_KNN_pic.o ./obj/graphe_pic.o // c'est pour lancer avec la biblio dynamique
OBJET_biblio_stac = ./obj/reco.o ./obj/reco_KNN.o ./obj/graphe.o //c'est pour lancer la biblio statique

## 7. Explication du Makefile

