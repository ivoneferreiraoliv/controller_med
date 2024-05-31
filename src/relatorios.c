#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <hpdf.h>
#include <setjmp.h>
#include <iconv.h>
#include "../include/relatorios.h"
#include "../include/database.h"

// Handler para erros do libharu
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    printf("ERROR: error_no=%04X, detail_no=%u\n", (unsigned int)error_no, (unsigned int)detail_no);
    longjmp(*(jmp_buf *)user_data, 1);
}

// Função para escrever o estoque atual de medicamentos no PDF
void estoqueAtualMedicamentos(HPDF_Doc pdf, HPDF_Page page, float *ypos, char *buf) {
    sqlite3_stmt *stmt;
    char *sql = "SELECT nome, SUM(quantidade) AS quantidade_atual FROM medicamentos GROUP BY nome;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int quantidade_atual = sqlite3_column_int(stmt, 1);
        snprintf(buf, 512, "%s, %d", nome, quantidade_atual);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 50, *ypos, buf);
        HPDF_Page_EndText(page);
        *ypos -= 20;
    }
    sqlite3_finalize(stmt);
}

// Função para escrever o resumo de consumo no PDF
void resumoConsumo(HPDF_Doc pdf, HPDF_Page page, float *ypos, char *buf) {
    sqlite3_stmt *stmt;
    char *sql = "SELECT nome, SUM(dosagem * quantidade) AS total_dosagem FROM medicamentos GROUP BY nome;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        int total_dosagem = sqlite3_column_int(stmt, 1);
        snprintf(buf, 512, "%s, %d", nome, total_dosagem);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 50, *ypos, buf);
        HPDF_Page_EndText(page);
        *ypos -= 20;
    }
    sqlite3_finalize(stmt);
}

// Função para escrever as informações sobre medicamentos no PDF
void informacoesMedicamentos(HPDF_Doc pdf, HPDF_Page page, float *ypos, char *buf) {
    sqlite3_stmt *stmt;

    // Inicia um bloco de texto
    HPDF_Page_BeginText(page);

     // Define a posição do cursor para o início do título
    HPDF_Page_MoveTextPos(page, 50, *ypos);

    // Exibe o título da tabela
    HPDF_Page_TextOut(page, 50, *ypos, "Dados de medicamentos armazenados:");

    // Move para a próxima linha para os títulos das colunas
    *ypos -= 20;


    // Define a posição do cursor para o início da linha de títulos
    HPDF_Page_MoveTextPos(page, 50, *ypos);

    // Exibe os títulos das colunas
    HPDF_Page_TextOut(page, 50, *ypos, "Nome");
    HPDF_Page_TextOut(page, 150, *ypos, "Dosagem");
    HPDF_Page_TextOut(page, 250, *ypos, "Quantidade");
    HPDF_Page_TextOut(page, 350, *ypos, "Estoque Inicial");
    HPDF_Page_TextOut(page, 450, *ypos, "Hora Dosagem");
    HPDF_Page_TextOut(page, 550, *ypos, "Hora Dose Tomada");

    // Define a posição inicial para os dados
    *ypos -= 20;
    

    // Prepara a consulta SQL
    char *sql = "SELECT nome, dosagem, quantidade, estoque_inicial, hora_dosagem, hora_dose_tomada FROM medicamentos;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Processa cada linha do resultado
    // Processa cada linha do resultado
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        const unsigned char *dosagem = sqlite3_column_text(stmt, 1);
        int quantidade = sqlite3_column_int(stmt, 2);
        int estoque_inicial = sqlite3_column_int(stmt, 3);
        const unsigned char *hora_dosagem = sqlite3_column_text(stmt, 4);
        const unsigned char *hora_dose_tomada = sqlite3_column_text(stmt, 5);

        char quantidade_str[12]; // Para armazenar a quantidade como string
        sprintf(quantidade_str, "%d", quantidade); // Converte a quantidade para string

        char estoque_inicial_str[12]; // Para armazenar o estoque inicial como string
        sprintf(estoque_inicial_str, "%d", estoque_inicial); // Converte o estoque inicial para string

        // Imprime os dados na página
        HPDF_Page_TextOut(page, 50, *ypos, (char *)nome);
        HPDF_Page_TextOut(page, 150, *ypos, (char *)dosagem);
        HPDF_Page_TextOut(page, 250, *ypos, quantidade_str);
        HPDF_Page_TextOut(page, 350, *ypos, estoque_inicial_str);
        HPDF_Page_TextOut(page, 450, *ypos, (char *)hora_dosagem);
        HPDF_Page_TextOut(page, 550, *ypos, (char *)hora_dose_tomada);

        // Move para a próxima linha
        *ypos -= 20;
    }

    // Finalize o bloco de texto
    HPDF_Page_EndText(page);

    sqlite3_finalize(stmt);
}

char *convert_utf8_to_iso8859_1(char *input) {
    iconv_t cd = iconv_open("ISO-8859-1", "UTF-8");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return NULL;
    }

    size_t input_len = strlen(input);
    size_t output_len = input_len * 2; // Pode precisar de mais espaço por causa de caracteres multibyte
    char *output = malloc(output_len + 1); // Espaço extra para o caractere nulo
    if (output == NULL) {
        perror("malloc");
        iconv_close(cd);
        return NULL;
    }

    char *output_ptr = output;
    size_t result = iconv(cd, &input, &input_len, &output_ptr, &output_len);
    if (result == (size_t)-1) {
        perror("iconv");
        free(output);
        iconv_close(cd);
        return NULL;
    }

    *output_ptr = '\0'; // Adiciona o caractere nulo no final

    iconv_close(cd);

    return output;
}

// Função principal para gerar o relatório de utilização
void gerarRelatorioDeUtilizacao() {
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font font;
    jmp_buf env;

    pdf = HPDF_New(error_handler, &env);
    if (!pdf) {
        printf("Erro ao criar o objeto PDF.\n");
        return;
    }

    if (setjmp(env)) {
        HPDF_Free(pdf);
        return;
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE, "Relatório de Utilização");

    page = HPDF_AddPage(pdf);
    HPDF_Page_SetWidth(page, 550);
    HPDF_Page_SetHeight(page, 800);

    HPDF_UseUTFEncodings(pdf);
    font = HPDF_GetFont(pdf, "Helvetica", "WinAnsiEncoding");
    HPDF_Page_SetFontAndSize(page, font, 12);

    float ypos = 750;
    char buf[512];

    char *converted_text;
    // Estoque Atual de Medicamentos
    HPDF_Page_BeginText(page);
converted_text = convert_utf8_to_iso8859_1("Estoque Atual de Medicamentos:");
    if (converted_text != NULL) {
        HPDF_Page_TextOut(page, 50, ypos, converted_text);
        free(converted_text);
    }
    HPDF_Page_EndText(page);
    ypos -= 20;
    estoqueAtualMedicamentos(pdf, page, &ypos, buf);



    // Informações sobre Medicamentos
    ypos -= 50; // Espaço extra entre seções
   
    informacoesMedicamentos(pdf, page, &ypos, buf);

    HPDF_SaveToFile(pdf, "relatorio.pdf");
    HPDF_Free(pdf);
}