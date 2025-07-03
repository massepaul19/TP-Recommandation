#----------------------------------------------------------------
# ---------------------  TP RECOMMANDATION ----------------------
## --------------- MASSE MASSE PAUL - BASTHYLLE -----------------
#----------------------------------------------------------------


#--------------------------------------------------------------------------------------
#  ------  TP RECOMMANDATION Explication claires des étapes et des structures ---------
#--------------------------------------------------------------------------------------

Un système de recommandation complet implémenté en C, utilisant les algorithmes des k plus proches voisins (KNN) avec la corrélation de Pearson pour prédire les préférences des utilisateurs, la Factorisation Matricielle et le Page Rank avec les Graphes bipartis.

## Par rapport au systeme 

Ici nous avons choisi d'implementer 02 tests : 
- le test en localhost : qui consiste au client et au serveur d'être dans une même machine et de fonctionner ensemble
- le test en résau : ici nous utilisons le point d'acces pour mettre les machines (serveur + les clients ) en réseau et au préalable nous lançons l'execution côté client sur les machines clientes et nous démarrons le serveur

Pour tester en Réseau, Allez dans network, créer un dossier client et copiez les fichiers qui concernent le client:
c'est-à-dire:
copiez src/fonctions.client.c , include/fonctions.client.h et client.c dans la dossier que vous allez partager
dans ce nouveau dossier ouvrez le fichier client.c remplacer par l'ip que votre AP vous à fourni dans le premier define

Ex: voici mon adresse #define IP_SERVEUR "192.168.43.71" celle que mon AP m'offre changez la par la votre

NB: juste en bas il y'a le main rassurer vous d'avoir juste commenter le cas du localhost
Ex ici :

ligne 16

char server_ip[16] = IP_SERVEUR;         //ici je c'est l'adresse que mon ap me délivre pour mon serveur
//char server_ip[16] = IP_SERVEUR_LOCAL; //pour tester en local 

avant taper: make adresse : elle va fournir vos adresses et copier alors celle attribuée par l'AP

puis donner le dossier aux clients externes connectés à votre AP et qu'ils éxécutent.

###Gestion des erreurs et conflits : acces à la ressource par plusieurs clients

Nous utilisons les threads pour pouvoir gérer ...
## 1. TRAITEMENT

## Resumé Makefile

Pour faire simple, pour savoir comment executer le projet taper la commande make help

### -------------------------------------------
### ===== Aide ==========
### -------------------------------------------

```c
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
	@echo 'make genere        - Générer vos propres Transactions'

```

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

/ Structure pour gerer la factorisation matricielle
```c
typedef struct {
    double **U;         // Matrice des utilisateurs (M x K)
    double **V;         // Matrice des items (N x K)
    double *bias_u;     // Biais utilisateurs
    double *bias_v;     // Biais items
    double bias_global; // Biais global
    int M;              // Nombre d'utilisateurs
    int N;              // Nombre d'articles
    int K;              // Nombre de facteurs latents
} MatriceFactorisation;

// Matrice complète
typedef struct {
    double **matrice;  // Matrice des notes M x N
    int M;             // Nombre d'utilisateurs
    int N;             // Nombre d'articles
} MatriceComplete;
```

structure pour gerer GRAPHE
```c
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

structure pour gerer le CLIENT
```
typedef struct {
    int socket_fd;
    struct sockaddr_in server_addr;
} client_connection_t;
```

SERVEUR
```
extern pthread_mutex_t mutex_recommandeur;  #me permet de gerer la connexion simultanée de plusieurs clients
```
## A Propos

### MENU PRINCIPAL CÔTE SERVEUR

╔══════════════════════════════════════════════════════════════╗
║                   SYSTÈME DE RECOMMANDATION                  ║
║                         Version 1.0                          ║
║                MASSE MASSE PAUL - BASTHYLLE                  ║
╠══════════════════════════════════════════════════════════════╣
║                                                              ║
║  1. Traitement et gestion des données                        ║
║  2. Extraction de données vers Train                         ║
║  3. Système de recommandation KNN                            ║
║  4. Système de recommandation Factorisation Matricielle      ║
║  5. Système de recommandation avec Graphes                   ║
║  6. À propos du système                                      ║
║  0. Quitter le programme                                     ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝

### About

═=═════════════════════════════════════════════════════════════╗
║                      À PROPOS DU SYSTÈME                     ║
╠══════════════════════════════════════════════════════════════╣
║                                                              ║
║  Système de Recommandation - Version 1.0                     ║
║  Utilise l'algorithme K-Nearest Neighbors (KNN) ,            ║
║  Factorisation Matricielle et Pae Rank                       ║
║  pour fournir des recommandations personnalisées             ║
║                                                              ║
║  Fonctionnalités :                                           ║
║  • Traitement et gestion des données                         ║
║  • Algorithme pour les recommandations                       ║
║  • Génération des Transactions                               ║
║  • Interface utilisateur intuitive                           ║
║                                                              ║
║  Methodes de Recommandations :                               ║
║  • Algorithme Facto. Matricielle pour les recommandations    ║
║  • Algorithme KNN pour les recommandations                   ║
║  • Algorithme Page Rank avec les Graphes                     ║
║                                                              ║
║  Connexion Client serveur :                                  ║
║  • Grace aux sockets et aux threads nous avons la possibilité║
║  d'avoir un serveur et plusieurs clints connectés            ║
║                                                              ║
║  Côté Réseau :                                 	       ║
║   Notre app offre une facilité d'adaptation au               ║
║   réseau pour les tests:                                     ║
║  d'avoir un serveur et plusieurs clints connectés            ║
║                                                              ║
║  	• Option 1 : Localhost avec 127.0.0.1                  ║
║  	• Option 2 : Grace à un AP (en réseau)                 ║
║                                                              ║
║                                             Paulo Masse      ║
╚══════════════════════════════════════════════════════════════╝

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
cette option permet de recuperer une partie des données et les mettre dans un autre fichier Train.txt afin de ne pas tester avec toutes nos données qui sont prêt de 6000 transactions.
Ici l'user entre la zone de selection (ligne de début et celle de la fin)

###Système de recommandation KNN

Options :
1. Calculer la matrice de Pearson
2. Tester des prédictions individuelles Pearson(u , i)
3. Effectuer toutes les prédictions sur le fichier de test
4. Évaluer les performances
5. Sauvegarder les résultats
6. Afficher les statistiques
7. Test d'automatisation des calculs(Etapes)
0. Menu principal


###Système de recommandation Factorisation Matricielle

1. Charger les données d'entraînement
2. Afficher les statistiques des données
3. Entraîner le modèle de factorisation matricielle
4. Tester le système matriciel
5. Évaluer le système complet
6. Automatisation et Recommandation
0. Quitter

###Système de recommandation avec Graphes

===== MENU RECOMMANDATION - GRAPHE + PAGERANK =====

Options :
1. Lire les données d'entraînement (Train.txt)
2. Construire le graphe bipartite
3. Appliquer PageRank
4. Sauvegarder les résultats PageRank
5. Afficher exemple de recommandations
6. Test d'automatisation des calculs(Etapes)
0. Nettoyer la mémoire et quitter

### Fichiers de Traitement

- `include/reco.h` : Définitions des structures et fonctions de base
- `src/reco.c` : Implémentation des fonctions de traitement  
- `include/Algo_recommandation.h` : Interface pour les 3 tâches KNN
- `src/Algo_recommandation.c` : Implémentation des algorithmes KNN
- Limites : `MAX_USERS = 50`, `MAX_ARTICLES = 50`, `MAX_CATEGORIES = 50`

## UTILISATION

Pour les trois algorithmes, veuillez suivre les étapes en ordre
Autre tester directement les recommandations au dernier point de chaque menu concu pour automatiser toutes les autres etapes

## 3. UTILISATION

### Modes d'exécution

**Pour les tests locaux (sans réseau) :**
```bash
make run         # Version standard
make runstatic   # Avec bibliothèque statique
make rundyn      # Avec bibliothèque dynamique
make rundynessai # Avec 3 bibliothèques séparées
```

**Pour le mode client-serveur :**
```bash
make serveur     # Démarre le serveur
make client      # Démarre le client
```

### Bibliothèques disponibles

- **librecommantion.a** : Bibliothèque statique (intégrée au moment de la compilation)
- **librecommantion.so** : Bibliothèque dynamique principale (chargée à l'exécution)
- **Bibliothèques spécialisées** :
  - `libreco_KNN.so` : Algorithme KNN
  - `libGraphe.so` : Recommandations par graphes
  - `libFactorisation.so` : Factorisation matricielle

### Choix de la bibliothèque dynamique

Pour le système client-serveur, nous utilisons les bibliothèques dynamiques car :
- **Consommation réduite** : Code partagé entre processus
- **Chargement à l'exécution** : Optimise l'utilisation mémoire
- **Modularité** : Permet de charger seulement les algorithmes nécessaires

### Utilisation des algorithmes

**Important :** Pour chaque algorithme (KNN, Graphes, Factorisation) :
1. Suivre **pas à pas** chaque étape du menu
2. **OU** utiliser la dernière option qui automatise toutes les étapes et fournit directement les recommandations

### Génération de données

```bash
make genere      # Génère des transactions de test
make adresse     # Affiche les adresses réseau disponibles
```

## 4. FORMAT DES DONNÉES

### Fichier de Transactions (data/essai/Train.txt, Train.txt , data/données.txt)
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

## 5. STRUCTURE DU PROJET

```
.
├── bin/                        # Exécutables compilés
│   ├── cisse.out              		# Exécutable principal
│   ├── client                 		# Client réseau
│   ├── serveur                		# Serveur réseau
│   ├── rundyn                 		# Version dynamique
│   ├── rundynessai            		# Version dynamique test avec les 03 bibio
│   └── runstatic              		# Version statique
├── data/                      # Données du projet
│   ├── Articles.txt           		# Liste des articles
│   ├── Categories.txt         		# Liste des catégories
│   ├── Users.txt              		# Liste des utilisateurs
│   ├── donnees.txt            		# Données brutes complètes
│   ├── transactions_filtrees.txt  # Données filtrées
│   ├── transactions_t1-t2.txt     # Données par période
│   ├── essai/                 # Jeu de données d'essai Traitement des données
│   │   ├── Clean.txt          		# Données nettoyées
│   │   ├── Test.txt           		# Données de test
│   │   └── Train.txt          		# Données d'entraînement
│   └── KNN_TRAIN/             # Données pour KNN
│       ├── resultats_predictions.txt   # Résultats des prédictions
│       ├── Test.txt           		# Données de test en general
│       └── Train.txt          		# Données d'entraînement en general et KNN aussi
├── include/                   # Fichiers d'en-tête (déclarations)
│   ├── factorisation.h        		# Déclarations factorisation matricielle
│   ├── graphe.h               		# Déclarations graphes de recommandation
│   ├── menu.h                 		# Déclarations interface utilisateur
│   ├── reco.h                 		# Déclarations structures de base et traitement des données
│   └── reco_KNN.h             		# Déclarations algorithme KNN
├── src/                       # Code source 
│   ├── factorisation.c        		# Implémentation factorisation
│   ├── graphe.c               		# Implémentation graphes
│   ├── menu.c                 		# Interface utilisateur
│   ├── reco.c                 		# Implémentation traitement de base
│   └── reco_KNN.c             		# Implémentation KNN
├── lib/                       # Bibliothèques
│   ├── libFactorisation.so    		# Bibliothèque dynamique factorisation
│   ├── libGraphe.so           		# Bibliothèque dynamique graphes
│   ├── libreco_KNN.so         		# Bibliothèque dynamique KNN
│   ├── librecommantion.a      		# Bibliothèque statique principale
│   └── librecommantion.so     		# Bibliothèque dynamique principale
├── obj/                       # Fichiers objets compilés
│   ├── factorisation.o       	 	# Objet factorisation
│   ├── factorisation_pic.o    		# Objet factorisation (PIC)
│   ├── graphe.o               		# Objet graphes
│   ├── graphe_pic.o           		# Objet graphes (PIC)
│   ├── main.o                 		# Objet principal
│   ├── menu.o                 		# Objet menu
│   ├── reco_KNN.o             		# Objet KNN
│   ├── reco_KNN_pic.o         		# Objet KNN (PIC)
│   ├── reco.o                 		# Objet recommandation
│   └── reco_pic.o             		# Objet recommandation (PIC)
├── network/                   	# Module réseau client-serveur
│   ├── client.c               		# Client réseau
│   ├── serveur.c              		# Serveur réseau
│   ├── include/               		# En-têtes réseau (déclarations)
│   │   ├── fonctions_client.h 		# Déclarations fonctions client
│   │   └── fonctions_serveur.h 	# Déclarations fonctions serveur
│   └── src/                   		# Sources réseau
│       ├── fonctions_client.c 		# Implémentation client
│       └── fonctions_serveur.c 	# Implémentation serveur
├── test/                      	# Tests et programme principal
│   └── main.c                 		# Point d'entrée du programme
├── Images Exemples/           		# Screenshots et exemples
│   ├── config_local.png      	        # Configuration localhost
│   ├── config_pour_ap.png    	        # Configuration point d'accès
│   ├── test_avec_ap.png       		# Test avec AP
│   └── test_avec_localhost.png 	# Test en local
├── Makefile                  	        # Script de compilation
├── README.md                  		# Documentation du projet
├── genere.py                  		# Script de génération de données
├── données.txt               	        # Données supplémentaires
├── Train.txt                  		# Données d'entraînement globales
├── transactions_generées.txt  		# Données générées
├── TP3-INF 3621_ Programmation Système.pdf  # Documentation TP sur le traitement des données
└── Tp-recommandation_sockets.pdf            # Documentation sockets
```

*Développé dans le cadre d'un projet de système de recommandation utilisant l'algorithme des k plus proches voisins avec corrélation de Pearson.*

## 6. Structures : Pourquoi avons nous choisi celà ?

Dans le traitement, le code utilise une fonction ecrire_transaction est telle que lorsqu'elle est appelée sur un fichier, celle si mets à jour tous les autres fichiers

Nous avons créé plusieurs autres fichiers adptés à notre structure afin de répondre plus facilement aux questions du traitement qui est la base de notre travail

### Explication des fichiers objets

Nous avons respectivement 

DOSSIER_OBJET = ./obj/main.o ./obj/reco.o ./obj/reco_KNN.o ./obj/menu.o ./obj/graphe.o   #ici c'est pour lancer l'executable principal
OBJET_biblio_dyn = ./obj/reco_pic.o ./obj/reco_KNN_pic.o ./obj/graphe_pic.o // c'est pour lancer avec la biblio dynamique
OBJET_biblio_stac = ./obj/reco.o ./obj/reco_KNN.o ./obj/graphe.o //c'est pour lancer la biblio statique

## 7. Explication du Makefile

### Variables de configuration
```makefile
CISSE = gcc                        # Compilateur
EXEC = cisse.out                   # Nom de l'exécutable principal
ALERT = -Wall -Werror -Wextra -lm  # Options de compilation strictes
```

NB: -lm me permet de lier la biblio math.h 
mkdir -p me permet de creer un dossier s'il n'existe pas encore

### Compilation principale
```makefile
all: $(EXEC)                  # Cible par défaut
$(EXEC): $(DOSSIER_OBJET)     # Compile l'exécutable principal
```

### Compilation des objets
- Chaque fichier `.c` est compilé en `.o` dans le dossier `obj/`
- Inclut les algorithmes : KNN, graphes, factorisation
- Crée les versions PIC (Position Independent Code) pour les bibliothèques dynamiques

### Exécutables
- `bin/runstatic` : Version avec bibliothèque statique
- `bin/rundyn`    : Version avec bibliothèque dynamique
- `bin/rundynessai` : Version avec 3 bibliothèques séparées

### Réseau client-serveur
```makefile
bin/serveur                   # Serveur de recommandation
bin/client                    # Client réseau
```

make               # Compiler tout le programme
make run           # Exécuter le programme
make runstatic     # Exécuter avec bibliothèque statique
make rundyn        # Exécuter avec bibliothèque dynamique
make rundynessai   # Exécuter avec 3 bibliothèques dynamiques
make serveur       # Démarrer le serveur
make stop-serveur  # Arrêter le serveur
make client        # Démarrer le client
make adresse       # Voir les adresses disponibles
make clean         # Nettoyage des fichiers
make efface        # Nettoyage du terminal
make genere        # Générer vos propres transactions
