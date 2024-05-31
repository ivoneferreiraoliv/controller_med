#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "database.h"
#include "relatorios.h"

#define DATABASE_FILE "medicamentos.db"

//estrutura de um usuário
typedef struct {
    int id;
    char nome[100];
    char email[100];
    char senha[100];
} Usuario;

//estrutura de um medicamento
typedef struct {
    int id;
    char nome[100];
    int dosagem;
    int quantidade;
} Medicamento;

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void criarTabelaMedicamentos() {
    char *errMsg = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS medicamentos("  \
                "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                "nome TEXT NOT NULL," \
                "dosagem INT NOT NULL," \
                "quantidade INT NOT NULL," \
                "estoque_inicial INT NOT NULL," \
                "hora_dosagem TEXT NOT NULL," \
                "hora_dose_tomada TEXT);";

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar tabela: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Tabela criada com sucesso ou já existe\n");
    }
}

void criarTabelaUsuarios() {
    sqlite3* db;
    char* errMsg = 0;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc) {
        fprintf(stderr, "Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        fprintf(stderr, "Banco de dados aberto com sucesso\n");
    }

    char* sql = "CREATE TABLE IF NOT EXISTS Usuarios("  \
                "ID INT PRIMARY KEY     NOT NULL," \
                "NOME           TEXT    NOT NULL," \
                "EMAIL          TEXT    NOT NULL," \
                "SENHA          TEXT    NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        fprintf(stdout, "Tabela criada com sucesso ou já existe\n");
    }

    sqlite3_close(db);
}

void inserirUsuario(Usuario usuario);

void inserirUsuario(Usuario usuario) {
    sqlite3* db;
    char* errMsg = 0;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc) {
        fprintf(stderr, "Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        fprintf(stderr, "Banco de dados aberto com sucesso\n");
    }

    char sql[500];
    sprintf(sql, "INSERT INTO Usuarios (ID,NOME,EMAIL,SENHA) "  \
                 "VALUES (%d, '%s', '%s', '%s' );", 
                 usuario.id, usuario.nome, usuario.email, usuario.senha);

    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        fprintf(stdout, "Usuário inserido com sucesso\n");
    }

    sqlite3_close(db);
}

int fazerLogin() {
    char email[100];
    char senha[100];

    printf("Digite o email: ");
    fgets(email, 100, stdin);
    email[strcspn(email, "\n")] = 0; // Remove o '\n' do final

    printf("Digite a senha: ");
    fgets(senha, 100, stdin);
    senha[strcspn(senha, "\n")] = 0; // Remove o '\n' do final

    sqlite3* db;
    char *errMsg = 0;
    int rc;

    rc = sqlite3_open("medicamentos.db", &db);

    if (rc) {
        fprintf(stderr, "Não foi possível abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    char sql[256];
    sprintf(sql, "SELECT * FROM Usuarios WHERE EMAIL='%s' AND SENHA='%s'", email, senha);
    // printf("Executando consulta SQL: %s\n", sql);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Falha na preparação: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        fprintf(stdout, "Login bem-sucedido\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    } else if (rc == SQLITE_DONE) {
        fprintf(stdout, "Falha no login. Tente novamente.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
}

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
    char nome[100];
    int quantidade, dosagem;
    int hora, minuto;

    printf("Digite o nome do medicamento: ");
    fgets(nome, 100, stdin);
    nome[strcspn(nome, "\n")] = 0;  // remove o '\n' do final

    printf("Digite a quantidade do medicamento: ");
    scanf("%d", &quantidade);
    getchar();

    printf("Digite a dosagem do medicamento (em mg): ");
    scanf("%d", &dosagem);
    getchar();

    printf("Digite a hora da dosagem (formato 24h): ");
    scanf("%d:%d", &hora, &minuto);
    getchar();

    char sql[1000];
    sprintf(sql, "INSERT INTO medicamentos (nome, quantidade, dosagem, hora_dosagem, estoque_inicial) VALUES ('%s', %d, %d, '%02d:%02d', %d);", nome, quantidade, dosagem, hora, minuto, quantidade);
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao cadastrar medicamento: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Medicamento cadastrado com sucesso!\n");
    }
}


void registrarDoseTomada(int medicamento_id) {
    int doseTomada;
    int hora, minuto;

    printf("Você tomou a dose? (1 para Sim, 2 para Não): ");
    scanf("%d", &doseTomada);
    getchar();

    if (doseTomada == 1) {
        printf("Digite a hora em que a dose foi tomada (formato 24h): ");
        scanf("%d:%d", &hora, &minuto);
        getchar();

        char sql[1000];
        sprintf(sql, "UPDATE medicamentos SET hora_dose_tomada = '%02d:%02d', quantidade = quantidade - 1 WHERE id = %d;", hora, minuto, medicamento_id);

        char *errMsg = 0;
        int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Erro ao registrar a dose tomada: %s\n", errMsg);
            sqlite3_free(errMsg);
        } else {
            printf("A dose tomada foi registrada com sucesso!\n");
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
    int opcao;
    int autenticado = 0;
    int medicamento_id;
    Medicamento medicamentos[100]; // Array para armazenar os medicamentos

    abrirBancoDeDados();
    criarTabelaMedicamentos();
    criarTabelaUsuarios();

    do {
        printf("1- Registrar 2- Login 3- Sair: ");
        scanf("%d", &opcao);
        while ((getchar()) != '\n');

        switch (opcao) {
            case 1:
                Usuario usuario;
            printf("Digite o ID do usuário: ");
            scanf("%d", &usuario.id);
            getchar();

            printf("Digite o nome do usuário: ");
            fgets(usuario.nome, 100, stdin);
            usuario.nome[strcspn(usuario.nome, "\n")] = 0; // Remove o '\n' do final

            printf("Digite o email do usuário: ");
            fgets(usuario.email, 100, stdin);
            usuario.email[strcspn(usuario.email, "\n")] = 0; // Remove o '\n' do final

            printf("Digite a senha do usuário: ");
            fgets(usuario.senha, 100, stdin);
            usuario.senha[strcspn(usuario.senha, "\n")] = 0; // Remove o '\n' do final

            inserirUsuario(usuario);
            
            break;
            
            case 2:
                autenticado = fazerLogin();
                if (autenticado) {
                    int num_medicamentos = buscarTodosMedicamentos(medicamentos); // Preenche o array com os medicamentos do banco de dados

                    do {
                        printf("\n1. Cadastrar Medicamento\n");
                        printf("2. Registrar dose tomada\n");
                        printf("3. Visualizar Medicamentos\n");
                        printf("4. Editar Medicamento\n");
                        printf("5. Apagar Medicamento\n");
                        printf("6. Buscar Medicamento\n");
                        printf("7. Gerar relatório de uso\n");
                        printf("8. Sair\n");
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
                        case 7:
                            gerarRelatorioDeUtilizacao();
                            break;
                        }
                    } while (opcao != 8);
                } else {
                    printf("Falha no login. Tente novamente.\n");
                }
                break;
            case 3:
                fecharBancoDeDados();
                return 0;
        }
    } while (opcao != 3);

    fecharBancoDeDados();

    return 0;
}
