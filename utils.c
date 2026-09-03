#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"

int long_para_int_seguro(long valor, const char *nome_param) {
    if (valor > INT_MAX) {
        fprintf(stderr, "Erro: %s e grande demais (maximo permitido: %d)\n", nome_param, INT_MAX);
        exit(EXIT_FAILURE);
    }
    return (int) valor;
}

/* ------------------------------------------------------------------ */
/* Abre times.txt em modo "escrever" apenas para garantir que o         */
/* arquivo existe e está vazio no início desta execução (uma execução   */
/* não deve misturar tempos de execuções anteriores).                   */
/* ------------------------------------------------------------------ */
void inicializar_arquivo_tempos(const char *caminho) {
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo '%s'\n", caminho);
        exit(EXIT_FAILURE);
    }
    fclose(arquivo);
}

/* ------------------------------------------------------------------ */
/* Registra, em modo "append", o tempo gasto por uma implementação.     */
/* ------------------------------------------------------------------ */
void registrar_tempo(const char *caminho, const char *nome_implementacao, double segundos) {
    FILE *arquivo = fopen(caminho, "a");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s' para escrita\n", caminho);
        exit(EXIT_FAILURE);
    }
    fprintf(arquivo, "%s: %.6f segundos\n", nome_implementacao, segundos);
    fclose(arquivo);
}

/* ------------------------------------------------------------------ */
/* Diferença, em segundos, entre dois "struct timespec".                */
/* ------------------------------------------------------------------ */
double diferenca_segundos(struct timespec inicio, struct timespec fim) {
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

/* ------------------------------------------------------------------ */
/* Converte 'str' para um long positivo, escrevendo o resultado em      */
/* *resultado. Retorna true em caso de sucesso e false se a entrada     */
/* não for um número válido, estiver fora do intervalo, ou não for      */
/* positiva — quem chamar decide o que fazer (ex: encerrar o programa). */
/* ------------------------------------------------------------------ */
bool parse_positive_long(const char *str, const char *nome_param, long *resultado) {
    if (str == NULL || nome_param == NULL || resultado == NULL) {
        fprintf(stderr, "Erro interno: parametro nulo passado para parse_positive_long.\n");
        return false;
    }

    errno = 0;
    char *endptr = NULL;
    long valor = strtol(str, &endptr, 10);

    if (endptr == str) {
        fprintf(stderr, "Erro: %s ('%s') nao e um numero.\n", nome_param, str);
        return false;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Erro: %s ('%s') tem caracteres invalidos.\n", nome_param, str);
        return false;
    }
    if (errno == ERANGE || valor == LONG_MIN || valor == LONG_MAX) {
        fprintf(stderr, "Erro: %s ('%s') esta fora do intervalo.\n", nome_param, str);
        return false;
    }
    if (valor <= 0) {
        fprintf(stderr, "Erro: %s deve ser positivo.\n", nome_param);
        return false;
    }

    *resultado = valor;
    return true;
}

/* ------------------------------------------------------------------ */
/* Garante apenas que num_threads é positivo, cobrindo o caso de         */
/* entradas negativas (sem limite superior).                            */
/* ------------------------------------------------------------------ */
void validar_num_threads(int num_threads) {
    if (num_threads <= 0) {
        fprintf(stderr, "Erro: numero de threads deve ser positivo (recebido: %d)\n", num_threads);
        exit(EXIT_FAILURE);
    }
}