#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "utils.h"
#include "calculo.h"
#include "threads_func.h"

/* Iniciais do e-mail: usadas para nomear os arquivos de saída, como     */
/* exige o enunciado (mandelbrot_<login>_serial.pgm, etc.).             */
#define LOGIN "lsncb"

/* ------------------------------------------------------------------ */
/* Converte um long já validado como positivo para int, checando se     */
/* ele cabe no intervalo de int. Encerra o programa com mensagem de     */
/* erro coerente caso não caiba (parâmetro absurdamente grande).        */
/* ------------------------------------------------------------------ */


int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Uso: %s [largura] [altura] [max_iteracoes] [num_threads]\n", argv[0]);
        return EXIT_FAILURE;
    }

    long largura_l, altura_l, max_iteracoes_l, num_threads_l;

    if (!parse_positive_long(argv[1], "largura", &largura_l))
        return EXIT_FAILURE;
    if (!parse_positive_long(argv[2], "altura", &altura_l))
        return EXIT_FAILURE;
    if (!parse_positive_long(argv[3], "max_iteracoes", &max_iteracoes_l))
        return EXIT_FAILURE;
    if (!parse_positive_long(argv[4], "num_threads", &num_threads_l))
        return EXIT_FAILURE;

    ParametrosMandelbrot p;
    p.largura       = long_para_int_seguro(largura_l, "largura");
    p.altura        = long_para_int_seguro(altura_l, "altura");
    p.max_iteracoes = long_para_int_seguro(max_iteracoes_l, "max_iteracoes");
    int num_threads = long_para_int_seguro(num_threads_l, "num_threads");
    validar_num_threads(num_threads);  // <-- nova checagem, antes de qualquer uso
    //para validar segmentation falt
    p.re_min = -2.0; p.re_max = 1.0;
    p.im_min = -1.5; p.im_max = 1.5;

    const char *arquivo_tempos = "times.txt";
    inicializar_arquivo_tempos(arquivo_tempos);

    struct timespec inicio, fim;
    char nome_arquivo[64];

    /* ---------------------------- Serial ---------------------------- */
    unsigned char *imagem_serial = aloca_imagem(p.largura, p.altura);
    if (imagem_serial == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria (serial)\n");
        return EXIT_FAILURE;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    calcula_mandelbrot_serial(imagem_serial, p.largura, p.altura, p.max_iteracoes);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    registrar_tempo(arquivo_tempos, "Serial", diferenca_segundos(inicio, fim));

    snprintf(nome_arquivo, sizeof(nome_arquivo), "mandelbrot_%s_serial.pgm", LOGIN);
    if (escreve_pgm(nome_arquivo, imagem_serial, p.largura, p.altura) != 0) {
        free(imagem_serial);
        return EXIT_FAILURE;
    }
    free(imagem_serial);

    /* ---------------------------- OpenMP ----------------------------- */
    unsigned char *imagem_openmp = aloca_imagem(p.largura, p.altura);
    if (imagem_openmp == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria (openmp)\n");
        return EXIT_FAILURE;
    }

    definir_numero_threads(num_threads);

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    dividir_iteracoes_entre_threads(imagem_openmp, &p);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    registrar_tempo(arquivo_tempos, "OpenMP", diferenca_segundos(inicio, fim));

    snprintf(nome_arquivo, sizeof(nome_arquivo), "mandelbrot_%s_openmp.pgm", LOGIN);
    if (escreve_pgm(nome_arquivo, imagem_openmp, p.largura, p.altura) != 0) {
        free(imagem_openmp);
        return EXIT_FAILURE;
    }
    free(imagem_openmp);

    /* -------------------------- Pthreads 1 ---------------------------- */
    /* Estratégia de divisão do trabalho: por LINHAS.                     */
    unsigned char *imagem_pthreads1 = aloca_imagem(p.largura, p.altura);
    if (imagem_pthreads1 == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria (pthreads1)\n");
        return EXIT_FAILURE;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_pthreads_por_linhas(imagem_pthreads1, &p, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    registrar_tempo(arquivo_tempos, "Pthreads1 (linhas)", diferenca_segundos(inicio, fim));

    snprintf(nome_arquivo, sizeof(nome_arquivo), "mandelbrot_%s_pthreads1.pgm", LOGIN);
    if (escreve_pgm(nome_arquivo, imagem_pthreads1, p.largura, p.altura) != 0) {
        free(imagem_pthreads1);
        return EXIT_FAILURE;
    }
    free(imagem_pthreads1);

    /* -------------------------- Pthreads 2 ---------------------------- */
    /* Estratégia de divisão do trabalho: por COLUNAS.                    */
    unsigned char *imagem_pthreads2 = aloca_imagem(p.largura, p.altura);
    if (imagem_pthreads2 == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria (pthreads2)\n");
        return EXIT_FAILURE;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrot_pthreads_por_colunas(imagem_pthreads2, &p, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    registrar_tempo(arquivo_tempos, "Pthreads2 (colunas)", diferenca_segundos(inicio, fim));

    snprintf(nome_arquivo, sizeof(nome_arquivo), "mandelbrot_%s_pthreads2.pgm", LOGIN);
    if (escreve_pgm(nome_arquivo, imagem_pthreads2, p.largura, p.altura) != 0) {
        free(imagem_pthreads2);
        return EXIT_FAILURE;
    }
    free(imagem_pthreads2);

    return EXIT_SUCCESS;
}