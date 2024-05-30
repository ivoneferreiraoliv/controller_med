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

void estoqueAtualMedicamentos(FILE *file) {
    char *sql;
    sqlite3_stmt *stmt;
    int rc;

    // Estoque atual
    sql = "SELECT nome, SUM(quantidade) AS quantidade_atual FROM medicamentos GROUP BY nome;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    fprintf(file, "Estoque Atual de Medicamentos:\n");
    fprintf(file, "Nome,Quantidade Atual\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int quantidade_atual = sqlite3_column_int(stmt, 1);
        fprintf(file, "%s,%d\n", nome, quantidade_atual);
    }
    sqlite3_finalize(stmt);
}

void resumoConsumo(FILE *file) {
    char *sql;
    sqlite3_stmt *stmt;
    int rc;

    // Resumo de consumo
    sql = "SELECT nome, SUM(dosagem * quantidade) AS total_dosagem FROM medicamentos GROUP BY nome;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    fprintf(file, "Resumo de Consumo:\n");
    fprintf(file, "Nome,Total Dosagem\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int total_dosagem = sqlite3_column_int(stmt, 1);
        fprintf(file, "%s,%d\n", nome, total_dosagem);
    }
    sqlite3_finalize(stmt);
}

void estatisticasUtilizacao(FILE *file) {
    char *sql;
    sqlite3_stmt *stmt;
    int rc;

    // Média diária
    sql = "SELECT AVG(dosagem * quantidade) FROM medicamentos;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_diaria = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    // Média semanal
    sql = "SELECT AVG(dosagem) FROM (SELECT SUM(dosagem * quantidade) AS dosagem FROM medicamentos GROUP BY strftime('%W', hora_dosagem));";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_semanal = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    // Média mensal
    sql = "SELECT AVG(dosagem) FROM (SELECT SUM(dosagem * quantidade) AS dosagem FROM medicamentos GROUP BY strftime('%Y-%m', hora_dosagem));";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    double media_mensal = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    fprintf(file, "Estatísticas de Utilização:\n");
    fprintf(file, "Média de Consumo Diário,%.2f\n", media_diaria);
    fprintf(file, "Média de Consumo Semanal,%.2f\n", media_semanal);
    fprintf(file, "Média de Consumo Mensal,%.2f\n", media_mensal);

    // Medicamento mais utilizado
    sql = "SELECT nome, SUM(dosagem * quantidade) AS total_dosagem FROM medicamentos GROUP BY nome ORDER BY total_dosagem DESC LIMIT 1;";
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
    sql = "SELECT nome, SUM(dosagem * quantidade) AS total_dosagem FROM medicamentos GROUP BY nome ORDER BY total_dosagem ASC LIMIT 1;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_step(stmt);
    const unsigned char *medicamento_menos_utilizado = sqlite3_column_text(stmt, 0);
    int total_dosagem_menos_utilizado = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    fprintf(file, "Medicamento Mais Utilizado,%s,%d\n", medicamento_mais_utilizado, total_dosagem_mais_utilizado);
    fprintf(file, "Medicamento Menos Utilizado,%s,%d\n", medicamento_menos_utilizado, total_dosagem_menos_utilizado);
}

void informacoesMedicamentos(FILE *file) {
    char *sql = "SELECT nome, dosagem, quantidade, estoque_inicial, hora_dosagem, hora_dose_tomada FROM medicamentos;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    fprintf(file, "Informações sobre Medicamentos:\n");
    fprintf(file, "Nome,Dosagem (mg),Quantidade,Estoque Inicial,Hora da Dosagem,Hora da Dose Tomada\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int dosagem = sqlite3_column_int(stmt, 1);
        int quantidade = sqlite3_column_int(stmt, 2);
        int estoque_inicial = sqlite3_column_int(stmt, 3);
        const unsigned char *hora_dosagem = sqlite3_column_text(stmt, 4);
        const unsigned char *hora_dose_tomada = sqlite3_column_text(stmt, 5);
        fprintf(file, "%s,%d,%d,%d,%s,%s\n", nome, dosagem, quantidade, estoque_inicial, hora_dosagem, hora_dose_tomada);
    }
    sqlite3_finalize(stmt);
}

void gerarRelatorioDeUtilizacao() {
    FILE *file = fopen("relatorio.csv", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    // Estoque Atual de Medicamentos
    estoqueAtualMedicamentos(file);
    fprintf(file, "\n");

    // Resumo de Consumo
    resumoConsumo(file);
    fprintf(file, "\n");

    // Estatísticas de Utilização
    estatisticasUtilizacao(file);
    fprintf(file, "\n");

    // Informações sobre Medicamentos
    informacoesMedicamentos(file);

    fclose(file);
}
