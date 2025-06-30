#MASSE MASSE Paul - Basthylle
# Declaration des variables

CISSE = gcc
EXEC = cisse.out

DOSSIER_OBJET = ./obj/main.o ./obj/reco.o ./obj/reco_KNN.o ./obj/menu.o ./obj/graphe.o
 
OBJET_biblio_dyn = ./obj/reco_pic.o ./obj/reco_KNN_pic.o ./obj/graphe_pic.o
OBJET_biblio_stac = ./obj/reco.o ./obj/reco_KNN.o ./obj/graphe.o


ALERT = -Wall -Werror -Wextra -lm

REP_stan = -Wl,-rpath,\$$ORIGIN/../lib


###########################################################

# -------------------------------------------
# Compilation principale 
# -------------------------------------------

all: $(EXEC)

$(EXEC): $(DOSSIER_OBJET)
	$(CISSE) $(DOSSIER_OBJET) -o ./bin/$(EXEC) $(ALERT)

###########################################################
# -------------------------------------------
# Compilation des fichiers objets
# -------------------------------------------

./obj/main.o: test/main.c include/reco.h
	mkdir -p obj
	$(CISSE) -c test/main.c -o ./obj/main.o -Iinclude $(ALERT)

./obj/reco.o: src/reco.c include/reco.h
	mkdir -p obj
	$(CISSE) -c src/reco.c -o ./obj/reco.o -Iinclude $(ALERT)

#######################
# Compilation des menus
./obj/menu.o: src/menu.c include/menu.h
	mkdir -p obj
	$(CISSE) -c src/menu.c -o ./obj/menu.o -Iinclude $(ALERT)

#######################
# Compilation des fichiers d'algorithmes

./obj/reco_KNN.o: src/reco_KNN.c include/reco_KNN.h
	mkdir -p obj
	$(CISSE) -c src/reco_KNN.c -o ./obj/reco_KNN.o -Iinclude $(ALERT)

########

./obj/graphe.o: src/graphe.c include/graphe.h
	mkdir -p obj
	$(CISSE) -c src/graphe.c -o ./obj/graphe.o -Iinclude $(ALERT)

###########################################################

# -------------------------------------------
# === Bibliothèque statique ===
# -------------------------------------------

lib/librecommantion.a: $(OBJET_biblio)
	mkdir -p lib
	ar rcs lib/librecommantion.a $(OBJET_biblio_stac)

bin/runstatic: test/main.c lib/librecommantion.a ./obj/menu.o
	mkdir -p bin
	$(CISSE) test/main.c ./obj/menu.o -Llib -lrecommantion -o bin/runstatic -Iinclude $(REP_stan) $(ALERT)

###########################################################

# -------------------------------------------
# === Bibliothèque dynamique ===
# -------------------------------------------
lib/librecommantion.so: $(OBJET_biblio)
	mkdir -p lib
	$(CISSE) -fPIC -c src/reco.c -o ./obj/reco_pic.o -Iinclude $(ALERT)    		#je compile avec fpic
	$(CISSE) -fPIC -c src/reco_KNN.c -o ./obj/reco_KNN_pic.o -Iinclude $(ALERT)
	$(CISSE) -fPIC -c src/graphe.c -o ./obj/graphe_pic.o -Iinclude $(ALERT)
	
	$(CISSE) -fPIC -shared -o lib/librecommantion.so $(OBJET_biblio_dyn) -Iinclude $(ALERT)

bin/rundyn: test/main.c lib/librecommantion.so ./obj/menu.o
	mkdir -p bin
	$(CISSE) test/main.c ./obj/menu.o -Llib -lrecommantion -o bin/rundyn -Iinclude $(REP_stan) $(ALERT)
###########################################################

# -------------------------------------------
# === Méthodes d'exécution que je propose ===
# -------------------------------------------

run: $(EXEC)
	./bin/$(EXEC)

runstatic: bin/runstatic
	./bin/runstatic

rundyn: bin/rundyn
	LD_LIBRARY_PATH=./lib ./bin/rundyn


#------------------------------------------------------------------------------
#----------------- BLOC POUR GERER LES CLIENTS ET LES SERVEURS ----------------
#------------------------------------------------------------------------------

###########################################################

# ----------------------------------------------------
# ========== Démarrage Côté serveur ==================
# ----------------------------------------------------
bin/serveur: network/serveur.c network/src/fonctions_serveur.c lib/librecommantion.so
	@mkdir -p bin
	$(CC) network/serveur.c network/src/fonctions_serveur.c -o bin/serveur -Iinclude -Inetwork/include -Llib -lrecommantion $(ALERT) $(REP_stan)

serveur: bin/serveur
	@echo "Démarrage du serveur..."
	LD_LIBRARY_PATH=./lib ./bin/serveur

stop-serveur:
	@echo "Arrêt du serveur..."
	@pkill -f "./bin/serveur" || echo "Aucun serveur à arrêter"

########################
# ----------------------------------------------------
# ==============  Démarrage Côté client ==============
# ----------------------------------------------------

bin/client: network/client.c network/src/fonctions_client.c
	@mkdir -p bin
	$(CC) network/client.c network/src/fonctions_client.c -o bin/client -Iinclude -Inetwork/include $(ALERT)

client: bin/client
	@echo "Démarrage du client..."
	./bin/client


###########################################################
#-------------------------------------------------------------------------------
#------------------- Voir les adresses disponibles -----------------------------
#-------------------------------------------------------------------------------

adresse:
	@echo "Les adresses disponibles..."
	hostname -I

###########################################################
# -------------------------------------------
# Nettoyage
# -------------------------------------------
clean:
	@echo "Nettoyage des fichiers générés..."
	rm -f ./obj/*.o ./bin/$(EXEC) bin/runstatic bin/rundyn bin/serveur bin/client lib/*.a lib/*.so
	@echo "Nettoyage terminé"

###########################################################

# -------------------------------------------
# ===== Aide ==========
# -------------------------------------------

help:
	@echo '=== MENU D''AIDE POUR LA COMPILATION  Paulo Masse ======'
	@echo 'make               - compiler tout le programme'
	@echo 'make run           - Executer le programme'
	@echo 'make runstatic     - Executer avec Biblio statique'
	@echo 'make rundyn        - Executer avec Biblio dynamique'
	@echo 'make serveur       - Demarrer avec le serveur'
	@echo 'make stop-serveur  - Arrêter le serveur'
	@echo 'make client        - Demarrer côté client'
	@echo 'make adresse       - Voir les adresses disponibles'
	@echo 'make clean         - Nettoyage des fichiers'
	@echo 'make efface        - Nettoyage du terminal'
	

###########################################################

# -------------------------------------------
# ===== Clean terminal ==========
# -------------------------------------------

efface:
	clear

###########################################################

.PHONY: clean all run runstatic rundyn help efface
