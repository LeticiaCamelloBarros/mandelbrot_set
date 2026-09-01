#ifndef THREADS_FUNC_H
#define THREADS_FUNC_H

#include "calculo.h"

/* ------------------------------------------------------------------ */
/* Única struct de argumentos passada às threads (Pthreads).            */
/* Usa 'unsigned char *imagem' para ser compatível com o buffer plano   */
/* definido em calculo.h (mesmo tipo usado por aloca_imagem,            */
/* calcula_mandelbrot_serial e escreve_pgm).                            */
/* ------------------------------------------------------------------ */
typedef struct {
    int indice_inicio;        /* primeira linha (ou coluna) que esta thread calcula */
    int indice_fim;            /* última linha (ou coluna), exclusiva                */
    unsigned char *imagem;     /* buffer plano: imagem[linha * largura + coluna]     */
    const ParametrosMandelbrot *params;
} ArgumentosThread;

/* ---------------------- OpenMP (Seção 4.5.3) ---------------------- */

void definir_numero_threads(int num_threads);

void dividir_iteracoes_entre_threads(unsigned char *imagem, const ParametrosMandelbrot *p);

void percorrer_colunas_mandelbrot(unsigned char *imagem, const ParametrosMandelbrot *p);
/* ---------------- Pthreads - paralelismo de dados (Seção 4.2.2) ---------------- */

void mandelbrot_pthreads_por_linhas(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads);

void mandelbrot_pthreads_por_colunas(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads);

void validar_num_threads(int num_threads) ;

#endif