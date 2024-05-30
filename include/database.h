#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#define DATABASE_FILE "medicamentos.db"

extern sqlite3 *db;

void abrirBancoDeDados();
void fecharBancoDeDados();

#endif
