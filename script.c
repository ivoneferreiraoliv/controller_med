#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "database.h"

#define DATABASE_FILE "medicamentos.db"

void criarTabelaMedicamentos() {
    char *errMsg = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS medicamentos("  \
                "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                "nome TEXT NOT NULL," \
                "dosagem INT NOT NULL," \
                "quantidade INT NOT NULL," \
                "estoque_inicial INT NOT NULL);";

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar tabela: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Tabela criada com sucesso ou já existe\n");
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

typedef struct {
    int id;
    char nome[100];
    int dosagem;
    int quantidade;
} Medicamento;

void registrarHorarioDosagem(int medicamento_id);

void alertaEstoque() {
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc != SQLITE_OK) {
        printf("Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    }

    char *sql = "SELECT nome, quantidade, estoque_inicial FROM medicamentos WHERE quantidade < (estoque_inicial / 2);";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        printf("Falha ao buscar dados: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(res) == SQLITE_ROW) {
        printf("\nAlerta Crítico de Estoque para o medicamento: %s\n", sqlite3_column_text(res, 0));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}

void cadastrarMedicamento() {
    char nome[50];
    int dosagem, quantidade;

    printf("Nome do Medicamento: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    printf("Dosagem (mg): ");
    scanf("%d", &dosagem);
    getchar();

    printf("Quantidade em Estoque: ");
    scanf("%d", &quantidade);
    getchar();

    char sql[1000];
    sprintf(sql, "INSERT INTO medicamentos (nome, dosagem, quantidade, estoque_inicial) "
                 "VALUES ('%s', %d, %d, %d);", nome, dosagem, quantidade, quantidade);

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao cadastrar medicamento: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Medicamento cadastrado com sucesso!\n");

        // Exemplo de chamada para registrar horários de dosagem
        int medicamento_id = sqlite3_last_insert_rowid(db);
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
    alertaEstoque();
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

int buscarTodosMedicamentos(Medicamento* medicamentos) {
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc != SQLITE_OK) {
        printf("Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    char sql[] = "SELECT id, nome, dosagem, quantidade FROM medicamentos;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        printf("Falha ao buscar medicamentos: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    int i = 0;
    while (sqlite3_step(res) == SQLITE_ROW) {
        medicamentos[i].id = sqlite3_column_int(res, 0);
        strcpy(medicamentos[i].nome, (char*)sqlite3_column_text(res, 1));
        medicamentos[i].dosagem = sqlite3_column_int(res, 2);
        medicamentos[i].quantidade = sqlite3_column_int(res, 3);
        i++;
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return i; // Retorna o número de medicamentos encontrados
}




Medicamento* buscarMedicamento(Medicamento* medicamentos, int num_medicamentos, char* consulta) {
    if (num_medicamentos == 0) {
        return NULL; // Base case: se a lista estiver vazia, retorne NULL
    } else if (strstr(medicamentos[0].nome, consulta) != NULL) {
        printf("Quantidade em estoque: %d\n", medicamentos[0].quantidade);
        return &medicamentos[0]; // Base case: se o nome do primeiro medicamento contém a consulta, retorne esse medicamento
    } else {
        return buscarMedicamento(medicamentos + 1, num_medicamentos - 1, consulta); // Recursive case: chame a função recursivamente com o restante da lista
    }
}

void editarMedicamento(int medicamento_id) {
    sqlite3 *db;
    
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc != SQLITE_OK) {
        printf("Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    }

    char novo_nome[100];
    int nova_dosagem, nova_quantidade;

    printf("Digite o novo nome do medicamento: ");
    fgets(novo_nome, 100, stdin);
    novo_nome[strcspn(novo_nome, "\n")] = 0; // Remove o '\n' do final

    printf("Digite a nova dosagem do medicamento: ");
    scanf("%d", &nova_dosagem);
    getchar(); // Limpa o buffer do teclado

    printf("Digite a nova quantidade do medicamento: ");
    scanf("%d", &nova_quantidade);
    getchar(); // Limpa o buffer do teclado

    char sql[1000];
    sprintf(sql, "UPDATE medicamentos SET nome = '%s', dosagem = %d, quantidade = %d WHERE id = %d;", novo_nome, nova_dosagem, nova_quantidade, medicamento_id);

    rc = sqlite3_exec(db, sql, 0, 0, 0);

    if (rc != SQLITE_OK) {
        printf("Falha ao editar medicamento: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Medicamento editado com sucesso.\n");
    }

    sqlite3_close(db);
}

void apagarMedicamento(int medicamento_id) {
    sqlite3 *db;
    
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc != SQLITE_OK) {
        printf("Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    }

    char sql[1000];
    sprintf(sql, "DELETE FROM medicamentos WHERE id = %d;", medicamento_id);

    rc = sqlite3_exec(db, sql, 0, 0, 0);

    if (rc != SQLITE_OK) {
        printf("Falha ao apagar medicamento: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Medicamento apagado com sucesso.\n");
    }

    sqlite3_close(db);
}


int main() {
   int opcao, medicamento_id;

    Medicamento medicamentos[100]; // Array para armazenar os medicamentos
    int num_medicamentos = buscarTodosMedicamentos(medicamentos); // Preenche o array com os medicamentos do banco de dados

   abrirBancoDeDados();
   criarTabelaMedicamentos();
   criarTabelaHorariosDosagem();

    do {
        printf("\n1. Cadastrar Medicamento\n");
        printf("2. Registrar dose tomada\n");
        printf("3. Visualizar Medicamentos\n");
        printf("4. Editar Medicamento\n");
        printf("5. Apagar Medicamento\n");
        printf("6. Buscar Medicamento\n");
        printf("7. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
           case 1:
               cadastrarMedicamento();
               num_medicamentos = buscarTodosMedicamentos(medicamentos);
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
                printf("Digite o ID do medicamento a ser editado: ");
                scanf("%d", &medicamento_id);
                getchar();
                editarMedicamento(medicamento_id);
                break;
            case 5:
                printf("Digite o ID do medicamento a ser apagado: ");
                scanf("%d", &medicamento_id);
                getchar();
                apagarMedicamento(medicamento_id);
                break;
                case 6:
                char consulta[100];
                printf("Digite o nome do medicamento a ser buscado: ");
                fgets(consulta, 100, stdin);
                consulta[strcspn(consulta, "\n")] = 0; // Remove o '\n' do final
                Medicamento* medicamento = buscarMedicamento(medicamentos, num_medicamentos, consulta);
                if (medicamento != NULL) {
                    printf("Medicamento encontrado: %s\n", medicamento->nome);
                } else {
                    printf("Medicamento não encontrado.\n");
                }
                break;
        }
    } while (opcao != 7);

   fecharBancoDeDados();

   return 0;
}