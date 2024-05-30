#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>

sqlite3 *db;

void abrirBancoDeDados() {
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc) {
        fprintf(stderr, "Erro ao abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

void fecharBancoDeDados() {
    sqlite3_close(db);
}
