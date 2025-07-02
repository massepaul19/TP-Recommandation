# Démarrage
# Déclaration des variables
CC = gcc
EXEC = cisse.out
ALERT = -Wall -Wextra -Werror

# ----------------------------------------------------
# ==============  Démarrage Côté client ==============
# ----------------------------------------------------

bin/client: client.c src/fonctions_client.c
	@mkdir -p bin
	$(CC) client.c src/fonctions_client.c -o bin/client -Iinclude -Inetwork/include $(ALERT)

client: bin/client
	@echo "Démarrage du client..."
	./bin/client

