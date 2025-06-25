# RECOMMANDATION

Un système de recommandation complet implémenté en C, utilisant l'algorithme des k plus proches voisins (KNN) avec la corrélation de Pearson pour prédire les préférences des utilisateurs.

## 1. TRAITEMENT

### Structure des Données

Le système utilise plusieurs structures pour gérer les données :

```c
typedef struct {
    int id_user;
    int id_article;
    int id_cat;
    float evaluation;
    double timestamp;
} Transaction;

typedef struct {
    unsigned int id_user;
    int nb_fois;
} User;

typedef struct {
    unsigned int id_article;
    unsigned int id_cat;
    int nb_fois;
} Article;
```

### Fonctionnalités de Traitement

- **Comptage et extraction** : Analyse des utilisateurs, articles et catégories
- **Filtrage par fréquence** : Élimination des utilisateurs/articles peu actifs
- **Nettoyage des données** : Suppression des incohérences
- **Extraction temporelle** : Filtrage des transactions par période
- **Statistiques descriptives** : Analyse exploratoire des données

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
- **libreco.a** : Bibliothèque statique contenant les fonctions de traitement
- **libreco.so** : Bibliothèque dynamique pour une utilisation modulaire

### Exemple d'Utilisation Complète

```c
#include "include/Algo_recommandation.h"
#include "include/reco.h"

int main() {
    // Étape 1 : Entraînement du modèle
    printf("=== ENTRAÎNEMENT ===\n");
    double** matrice_sim = Pearson("data/essai/Train.txt");
    RecommandeurKNN* rec = initialiser_recommandeur_depuis_fichier("data/essai/Train.txt");
    
    // Étape 2 : Prédiction individuelle
    printf("=== PRÉDICTION INDIVIDUELLE ===\n");
    float note = Predict(rec, 123, 456);
    printf("Note prédite pour utilisateur 123 et article 456 : %.2f\n", note);
    
    // Étape 3 : Évaluation sur le jeu de test
    printf("=== ÉVALUATION COMPLÈTE ===\n");
    Prediction predictions[10000];
    int nb_pred = Predict_all(rec, "data/essai/Test.txt", predictions, 10000);
    
    printf("Nombre de prédictions : %d\n", nb_pred);
    afficher_metriques_evaluation(predictions, nb_pred);
    
    // Nettoyage
    liberer_matrice_similarite(matrice_sim, rec->nb_users);
    liberer_recommandeur(rec);
    
    return 0;
}
```

## 4. FORMAT DES DONNÉES

### Fichier de Transactions (data/essai/Train.txt, Test.txt)
```
id_user id_article id_categorie evaluation timestamp
1       101        5           4.5        1609459200.0
1       102        3           3.0        1609545600.0
2       101        5           5.0        1609632000.0
```

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

## 7. RÉFÉRENCES

- **Algorithme KNN** : Collaborative Filtering Recommender Systems
- **Corrélation de Pearson** : Mesure de similarité statistique
- **Évaluation** : Métriques RMSE et MAE pour les systèmes de recommandation

---

*Développé dans le cadre d'un projet de système de recommandation utilisant l'algorithme des k plus proches voisins avec corrélation de Pearson.*
