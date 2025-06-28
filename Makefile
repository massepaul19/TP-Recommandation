# Declaration des variables

CISSE = gcc
EXEC = cisse.out
DOSSIER_OBJET = ./obj/main.o ./obj/reco.o ./obj/reco_KNN.o ./obj/menu.o
ALERT = -Wall -Werror -Wextra

REP_stan = -Wl,-rpath,\$$ORIGIN/../lib


###########################################################

# -------------------------------------------
# Compilation principale 
# -------------------------------------------

all: $(EXEC)

$(EXEC): $(DOSSIER_OBJET)
	$(CISSE) $(DOSSIER_OBJET) -o ./bin/$(EXEC) $(ALERT) -lm

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


#------------------------------------------------------------------------------
#######################
# Compilation des fichiers serveur et client

#serveur
./obj/serveur.o: network/serveur.c
	mkdir -p obj
	$(CISSE) -c network/serveur.c -o ./obj/serveur.o -Iinclude $(ALERT)

#client
./obj/client.o: network/client.c
	mkdir -p obj
	$(CISSE) -c network/client.c -o ./obj/client.o -Iinclude $(ALERT)

#------------------------------------------------------------------------------

###########################################################

# -------------------------------------------
# === Bibliothèque statique ===
# -------------------------------------------

lib/libreco.a: ./obj/reco.o
	mkdir -p lib
	ar rcs lib/libreco.a ./obj/reco.o

bin/runstatic: test/main.c lib/libreco.a
	mkdir -p bin
	$(CISSE) test/main.c -Llib -lreco -o bin/runstatic -Iinclude $(REP_stan)

###########################################################

# -------------------------------------------
# === Bibliothèque dynamique ===
# -------------------------------------------

lib/libreco.so: ./obj/reco.o
	mkdir -p lib
	$(CISSE) -fPIC -shared -o lib/libreco.so ./obj/reco.o -Iinclude 

bin/rundyn: test/main.c lib/libreco.so
	mkdir -p bin
	$(CISSE) test/main.c -Llib -lreco -o bin/rundyn -Iinclude $(REP_stan)

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

###########################################################


# ----------------------------------------------------
# === Méthodes pour lancer le serveur et le client ===
# ----------------------------------------------------

#runstatic: bin/runstatic
#	./bin/runstatic

#rundyn: bin/rundyn
#	LD_LIBRARY_PATH=./lib ./bin/rundyn

###########################################################

# -------------------------------------------
# Nettoyage
# -------------------------------------------

clean:
	rm -f ./obj/*.o ./bin/$(EXEC) bin/runstatic bin/rundyn lib/*.a lib/*.so


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
	@echo 'make client        - Demarrer côté client'
	@echo 'make clean         - Nettoyage des fichiers'
	@echo 'make efface        - Nettoyage du terminal'
	

###########################################################

# -------------------------------------------
# ===== Clean terminal ==========
# -------------------------------------------

efface:
	clear

###########################################################

.PHONY: clean all run runstatic rundyn

