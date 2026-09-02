#ifndef THREADS_FUNC_H
#define THREADS_FUNC_H

#include "calculo.h"

/* OpenMP */
void definir_numero_threads(int num_threads);
void dividir_iteracoes_entre_threads(unsigned char *imagem, const ParametrosMandelbrot *p);

/* Pthreads - Estratégia 1: divisão intercalada/cíclica por linha.
 * Usada em pthreads1. Cada thread processa as linhas id, id+total,
 * id+2*total, ... Ótimo balanceamento de carga estático, já que
 * regiões de alta densidade de iteração (dentro do conjunto) ficam
 * distribuídas entre todas as threads, em vez de concentradas em uma. */
void mandelbrot_pthreads_ciclico(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads);

/* Pthreads - Estratégia 2: divisão em blocos contíguos de linhas.
 * Usada em pthreads2. A thread `id` calcula da linha
 * (id * altura) / total_threads até ((id + 1) * altura) / total_threads. */
void mandelbrot_pthreads_blocos(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads);

#endif