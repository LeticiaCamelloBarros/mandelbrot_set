#include <pthread.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // para sysconf
#define FATOR_MAX_THREADS 4  /* margem de segurança acima do nº de núcleos */
#include "calculo.h"
#include "threads_func.h"

/* ==================================================================== */
/* PARTE 1: OpenMP (Seção 4.5.3)                                        */
/* ==================================================================== */

/* ------------------------------------------------------------------ */
/* Definir manualmente o número de threads usadas pelas regiões         */
/* paralelas OpenMP seguintes (Seção 4.5.3).                            */
/* ------------------------------------------------------------------ */
void definir_numero_threads(int num_threads) {
    if (num_threads <= 0) {
        fprintf(stderr, "Erro: numero de threads invalido (%d)\n", num_threads);
        exit(EXIT_FAILURE);
    }
    omp_set_num_threads(num_threads);
}

/* ------------------------------------------------------------------ */
/* Implementação OpenMP: divide as LINHAS da imagem entre as threads    */
/* com "#pragma omp parallel for". Cada thread calcula um intervalo     */
/* contíguo de linhas, escrevendo diretamente no buffer plano.          */
/* Percorrer por linhas favorece a localidade de cache, já que o        */
/* buffer é contíguo na memória (ver discussão da Seção 4.2).           */
/* ------------------------------------------------------------------ */
void dividir_iteracoes_entre_threads(unsigned char *imagem, const ParametrosMandelbrot *p) {
    int py;

    #pragma omp parallel for schedule(static)
    for (py = 0; py < p->altura; py++) {
        for (int px = 0; px < p->largura; px++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Implementação OpenMP: mesma ideia, mas paralelizando pelo eixo das   */
/* COLUNAS em vez das linhas. Mantida apenas para fins de comparação    */
/* de desempenho no relatório — o enunciado exige só UMA implementação  */
/* OpenMP na entrega final.                                             */
/* ------------------------------------------------------------------ */
void percorrer_colunas_mandelbrot(unsigned char *imagem, const ParametrosMandelbrot *p) {
    int px;

    #pragma omp parallel for schedule(static)
    for (px = 0; px < p->largura; px++) {
        for (int py = 0; py < p->altura; py++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
}

/* ==================================================================== */
/* PARTE 2: Pthreads - paralelismo de dados (Seção 4.2.2)               */
/* ==================================================================== */

/* -------------------- ESTRATÉGIA 1: divisão por LINHAS -------------------- */

static void *rotina_calcula_linhas(void *arg) {
    ArgumentosThread *a = (ArgumentosThread *) arg;
    const ParametrosMandelbrot *p = a->params;

    for (int py = a->indice_inicio; py < a->indice_fim; py++) {
        for (int px = 0; px < p->largura; px++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            a->imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
    return NULL;
}

/*
 * Cria 'num_threads', divide as ALTURA linhas da imagem em blocos
 * contíguos aproximadamente iguais (balanceamento, Seção 4.2.1) e
 * aguarda todas terminarem antes de retornar.
 */
void mandelbrot_pthreads_por_linhas(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ArgumentosThread *args = malloc(num_threads * sizeof(ArgumentosThread));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para threads\n");
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    int linhas_por_thread = p->altura / num_threads;
    int linhas_restantes  = p->altura % num_threads;
    int inicio = 0;

    for (int i = 0; i < num_threads; i++) {
        int tamanho_bloco = linhas_por_thread + (i < linhas_restantes ? 1 : 0);

        args[i].indice_inicio = inicio;
        args[i].indice_fim    = inicio + tamanho_bloco;
        args[i].imagem        = imagem;
        args[i].params        = p;

        if (pthread_create(&threads[i], NULL, rotina_calcula_linhas, &args[i]) != 0) {
            fprintf(stderr, "Erro: falha ao criar a thread %d\n", i);
            exit(EXIT_FAILURE);
        }
        inicio += tamanho_bloco;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}

/* -------------------- ESTRATÉGIA 2: divisão por COLUNAS -------------------- */

static void *rotina_calcula_colunas(void *arg) {
    ArgumentosThread *a = (ArgumentosThread *) arg;
    const ParametrosMandelbrot *p = a->params;

    for (int px = a->indice_inicio; px < a->indice_fim; px++) {
        for (int py = 0; py < p->altura; py++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            a->imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
    return NULL;
}

/*
 * Mesma lógica da função anterior, mas particionando a LARGURA
 * (colunas) da imagem entre as threads em vez das linhas — a segunda
 * estratégia de divisão do trabalho exigida pelo enunciado.
 */
void mandelbrot_pthreads_por_colunas(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ArgumentosThread *args = malloc(num_threads * sizeof(ArgumentosThread));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para threads\n");
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    int colunas_por_thread = p->largura / num_threads;
    int colunas_restantes  = p->largura % num_threads;
    int inicio = 0;

    for (int i = 0; i < num_threads; i++) {
        int tamanho_bloco = colunas_por_thread + (i < colunas_restantes ? 1 : 0);

        args[i].indice_inicio = inicio;
        args[i].indice_fim    = inicio + tamanho_bloco;
        args[i].imagem        = imagem;
        args[i].params        = p;

        if (pthread_create(&threads[i], NULL, rotina_calcula_colunas, &args[i]) != 0) {
            fprintf(stderr, "Erro: falha ao criar a thread %d\n", i);
            exit(EXIT_FAILURE);
        }
        inicio += tamanho_bloco;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}

