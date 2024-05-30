# Definições
CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -lsqlite3
SRC = $(wildcard src/*.c) script.c
OBJ = $(SRC:.c=.o)
EXEC = sistema

# Regra padrão
all: $(EXEC)

# Compilação do executável
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compilação dos arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação do arquivo objeto script.o
script.o: script.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos objeto e executável
clean:
	$(RM) $(OBJ) $(EXEC)