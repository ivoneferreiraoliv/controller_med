#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

#define DATABASE_FILE "medicamentos.db"

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

void criarTabelaMedicamentos() {
    char *sql = "CREATE TABLE IF NOT EXISTS medicamentos ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "nome TEXT NOT NULL,"
                "dosagem INTEGER NOT NULL,"
                "quantidade INTEGER NOT NULL,"
                "validade TEXT NOT NULL"
                ");";

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar a tabela de medicamentos: %s\n", errMsg);
        sqlite3_free(errMsg);
        exit(1);
    }
}

void criarTabelaHorariosDosagem() {
    char *sql = "CREATE TABLE IF NOT EXISTS horarios_dosagem ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "medicamento_id INTEGER NOT NULL,"
                "hora INTEGER NOT NULL,"
                "minuto INTEGER NOT NULL,"
                "FOREIGN KEY (medicamento_id) REFERENCES medicamentos(id)"
                ");";

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar a tabela de horários de dosagem: %s\n", errMsg);
        sqlite3_free(errMsg);
        exit(1);
    }
}
void registrarHorarioDosagem(int medicamento_id);

void cadastrarMedicamento() {
    char nome[50];
    int dosagem, quantidade;
    char validade[11];

    printf("Nome do Medicamento: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    printf("Dosagem (mg): ");
    scanf("%d", &dosagem);
    getchar();

    printf("Quantidade em Estoque: ");
    scanf("%d", &quantidade);
    getchar();

    printf("Data de Validade (dd/mm/aaaa): ");
    fgets(validade, sizeof(validade), stdin);
    validade[strcspn(validade, "\n")] = 0;

    char sql[1000];
    sprintf(sql, "INSERT INTO medicamentos (nome, dosagem, quantidade, validade) "
                 "VALUES ('%s', %d, %d, '%s');", nome, dosagem, quantidade, validade);

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao cadastrar medicamento: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Medicamento cadastrado com sucesso!\n");

        // Exemplo de chamada para registrar horários de dosagem
        int medicamento_id; // Defina o ID do medicamento cadastrado
        registrarHorarioDosagem(medicamento_id);
    }
}


void registrarHorarioDosagem(int medicamento_id) {
    int hora, minuto;

    printf("Hora do Horário de Dosagem (0-23): ");
    scanf("%d", &hora);
    getchar();

    printf("Minuto do Horário de Dosagem (0-59): ");
    scanf("%d", &minuto);
    getchar();

    char sql[1000];
    sprintf(sql, "INSERT INTO horarios_dosagem (medicamento_id, hora, minuto) "
                 "VALUES (%d, %d, %d);", medicamento_id, hora, minuto);

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao registrar horário de dosagem: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Horário de dosagem registrado com sucesso!\n");
    }
}

void registrarDoseTomada(int medicamento_id) {
    int doseTomada;

    printf("Você tomou a dose? (1 para Sim, 2 para Não): ");
    scanf("%d", &doseTomada);
    getchar();

    if (doseTomada == 1) {
        char sql[1000];
        sprintf(sql, "UPDATE medicamentos SET quantidade = quantidade - 1 WHERE id = %d;", medicamento_id);

        char *errMsg = 0;
        int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Erro ao atualizar a quantidade de medicamento: %s\n", errMsg);
            sqlite3_free(errMsg);
        } else {
            printf("A quantidade de medicamento foi atualizada com sucesso!\n");
        }
    } else if (doseTomada == 2) {
        printf("A dose não foi tomada.\n");
    } else {
        printf("Opção inválida!\n");
    }
}

void visualizarMedicamentos() {
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc != SQLITE_OK) {
        printf("Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_prepare_v2(db, "SELECT id, nome FROM Medicamentos", -1, &res, 0);

    if (rc != SQLITE_OK) {
        printf("Falha na preparação da declaração: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Medicamentos:\n");

    while (sqlite3_step(res) == SQLITE_ROW) {
        printf("%d: %s\n", sqlite3_column_int(res, 0), sqlite3_column_text(res, 1));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}


int main() {
   int opcao, medicamento_id;

   abrirBancoDeDados();
   criarTabelaMedicamentos();
   criarTabelaHorariosDosagem();

   do {
       printf("\n1. Cadastrar Medicamento\n");
       printf("2. Registrar dose tomada\n");
       printf("3. Visualizar Medicamentos\n");
       printf("4. Sair\n");
       printf("Escolha uma opção: ");
       scanf("%d", &opcao);
       getchar();

       switch (opcao) {
           case 1:
               cadastrarMedicamento();
               break;
           case 2:
               printf("Digite o ID do medicamento: ");
               scanf("%d", &medicamento_id);
               getchar();
               registrarDoseTomada(medicamento_id);
               break;
           case 3:
               visualizarMedicamentos();
               break;
           case 4:
               printf("Saindo...\n");
               break;
           default:
               printf("Opção inválida!\n");
       }
   } while (opcao != 4);

   fecharBancoDeDados();

   return 0;
}