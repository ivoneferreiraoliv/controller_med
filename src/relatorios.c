#include <stdio.h>
#include <sqlite3.h>
#include "../include/relatorios.h"
#include "../include/database.h"

void historicoDosagens() {
    char *sql = "SELECT dosagens.data, dosagens.hora, dosagens.minuto, dosagens.dosagem, medicamentos.nome "
                "FROM dosagens "
                "JOIN medicamentos ON dosagens.medicamento_id = medicamentos.id "
                "ORDER BY dosagens.data, dosagens.hora, dosagens.minuto;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Histórico de Dosagens:\n");
    printf("Data | Hora | Minuto | Dosagem (mg) | Nome do Medicamento\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *data = sqlite3_column_text(stmt, 0);
        int hora = sqlite3_column_int(stmt, 1);
        int minuto = sqlite3_column_int(stmt, 2);
        int dosagem = sqlite3_column_int(stmt, 3);
        const unsigned char *nome = sqlite3_column_text(stmt, 4);
        printf("%s | %02d:%02d | %d mg | %s\n", data, hora, minuto, dosagem, nome);
    }
    sqlite3_finalize(stmt);
}

void estoqueAtualMedicamentos() {
    char *sql = "SELECT nome, quantidade, validade FROM medicamentos;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Estoque Atual de Medicamentos:\n");
    printf("Nome | Quantidade | Validade\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int quantidade = sqlite3_column_int(stmt, 1);
        const unsigned char *validade = sqlite3_column_text(stmt, 2);
        printf("%s | %d | %s\n", nome, quantidade, validade);
    }
    sqlite3_finalize(stmt);
}

void alertasReposicaoEstoque() {
    char *sql = "SELECT nome, quantidade FROM medicamentos WHERE quantidade < 10;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Alertas de Reposição de Estoque:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int quantidade = sqlite3_column_int(stmt, 1);
        printf("Medicamento %s com estoque baixo: %d unidades restantes.\n", nome, quantidade);
    }
    sqlite3_finalize(stmt);
}

void resumoConsumo() {
    char *sql = "SELECT strftime('%Y-%m', data) AS mes, SUM(dosagem) AS total_dosagem "
                "FROM dosagens GROUP BY mes ORDER BY mes;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Resumo de Consumo (Mensal):\n");
    printf("Mês | Total de Dosagens (mg)\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *mes = sqlite3_column_text(stmt, 0);
        int total_dosagem = sqlite3_column_int(stmt, 1);
        printf("%s | %d mg\n", mes, total_dosagem);
    }
    sqlite3_finalize(stmt);
}

void estatisticasUtilizacao() {
    char *sql;
    sqlite3_stmt *stmt;
    int rc;

    // Média diária
    sql = "SELECT AVG(dosagem) FROM dosagens;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_diaria = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    // Média semanal
    sql = "SELECT AVG(dosagem) FROM (SELECT SUM(dosagem) AS dosagem FROM dosagens GROUP BY strftime('%W', data));";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_semanal = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    // Média mensal
    sql = "SELECT AVG(dosagem) FROM (SELECT SUM(dosagem) AS dosagem FROM dosagens GROUP BY strftime('%Y-%m', data));";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_mensal = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    printf("Estatísticas de Utilização:\n");
    printf("Média de Consumo Diário: %.2f mg\n", media_diaria);
    printf("Média de Consumo Semanal: %.2f mg\n", media_semanal);
    printf("Média de Consumo Mensal: %.2f mg\n", media_mensal);

    // Medicamento mais utilizado
    sql = "SELECT nome, SUM(dosagem) AS total_dosagem FROM dosagens "
          "JOIN medicamentos ON dosagens.medicamento_id = medicamentos.id "
          "GROUP BY medicamentos.id ORDER BY total_dosagem DESC LIMIT 1;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    const unsigned char *medicamento_mais_utilizado = sqlite3_column_text(stmt, 0);
    int total_dosagem_mais_utilizado = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    // Medicamento menos utilizado
    sql = "SELECT nome, SUM(dosagem) AS total_dosagem FROM dosagens "
          "JOIN medicamentos ON dosagens.medicamento_id = medicamentos.id "
          "GROUP BY medicamentos.id ORDER BY total_dosagem ASC LIMIT 1;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    const unsigned char *medicamento_menos_utilizado = sqlite3_column_text(stmt, 0);
    int total_dosagem_menos_utilizado = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    printf("Medicamento Mais Utilizado: %s (%d mg)\n", medicamento_mais_utilizado, total_dosagem_mais_utilizado);
    printf("Medicamento Menos Utilizado: %s (%d mg)\n", medicamento_menos_utilizado, total_dosagem_menos_utilizado);
}

void informacoesMedicamentos() {
    char *sql = "SELECT nome, dosagem, validade FROM medicamentos;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("Informações sobre Medicamentos:\n");
    printf("Nome | Dosagem (mg) | Validade\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int dosagem = sqlite3_column_int(stmt, 1);
        const unsigned char *validade = sqlite3_column_text(stmt, 2);
        printf("%s | %d mg | %s\n", nome, dosagem, validade);
    }
    sqlite3_finalize(stmt);
}

void gerarRelatorioDeUtilizacao() {
    printf("\n=== Relatório de Utilização de Medicamentos ===\n\n");

    // Histórico de Dosagens
    historicoDosagens();
    printf("\n");

    // Estoque Atual de Medicamentos
    estoqueAtualMedicamentos();
    printf("\n");

    // Alertas de Reposição de Estoque
    alertasReposicaoEstoque();
    printf("\n");

    // Resumo de Consumo
    resumoConsumo();
    printf("\n");

    // Estatísticas de Utilização
    estatisticasUtilizacao();
    printf("\n");

    // Informações sobre Medicamentos
    informacoesMedicamentos();
}
