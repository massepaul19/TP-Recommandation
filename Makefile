# Declaration des variables

CISSE = gcc
EXEC = cisse.out
DOSSIER_OBJET = ./obj/main.o ./obj/reco.o
ALERT = -Wall

REP_stan = -Wl,-rpath,\$$ORIGIN/../lib


# -------------------------------------------
# Compilation principale
# -------------------------------------------

all: $(EXEC)

$(EXEC): $(DOSSIER_OBJET)
	$(CISSE) $(DOSSIER_OBJET) -o ./bin/$(EXEC) $(ALERT)

# -------------------------------------------
# Compilation des fichiers objets
# -------------------------------------------

./obj/main.o: test/main.c include/reco.h
	mkdir -p obj
	$(CISSE) -c test/main.c -o ./obj/main.o -Iinclude $(ALERT)

./obj/reco.o: src/reco.c include/reco.h
	mkdir -p obj
	$(CISSE) -c src/reco.c -o ./obj/reco.o -Iinclude $(ALERT)

# -------------------------------------------
# === Bibliothèque statique ===
# -------------------------------------------

lib/libreco.a: ./obj/reco.o
	mkdir -p lib
	ar rcs lib/libreco.a ./obj/reco.o

bin/runstatic: test/main.c lib/libreco.a
	mkdir -p bin
	$(CISSE) test/main.c -Llib -lreco -o bin/runstatic -Iinclude $(REP_stan)

# -------------------------------------------
# === Bibliothèque dynamique ===
# -------------------------------------------

lib/libreco.so: ./obj/reco.o
	mkdir -p lib
	$(CISSE) -fPIC -shared -o lib/libreco.so ./obj/reco.o -Iinclude 

bin/rundyn: test/main.c lib/libreco.so
	mkdir -p bin
	$(CISSE) test/main.c -Llib -lreco -o bin/rundyn -Iinclude $(REP_stan)

# -------------------------------------------
# === Méthodes d'exécution que je propose ===
# -------------------------------------------

run: $(EXEC)
	./bin/$(EXEC)

runstatic: bin/runstatic
	./bin/runstatic

rundyn: bin/rundyn
	LD_LIBRARY_PATH=./lib ./bin/rundyn

# -------------------------------------------
# Nettoyage
# -------------------------------------------

clean:
	rm -f ./obj/*.o ./bin/$(EXEC) bin/runstatic bin/rundyn lib/*.a lib/*.so

.PHONY: clean all run runstatic rundyn

