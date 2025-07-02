# Démarrage
# Déclaration des variables
CC = gcc
EXEC = cisse.out
ALERT = -Wall -Wextra -Werror

# ----------------------------------------------------
# ==============  Démarrage Côté client ==============
# ----------------------------------------------------

bin/clien: client.c src/fonctions_client.c
	@mkdir -p bin
	$(CC) client.c src/fonctions_client.c -o bin/clien -Iinclude -Inetwork/include $(ALERT)

clien: bin/clien
	@echo "Démarrage du client..."
	./bin/client

