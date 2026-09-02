#include <pthread.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // para sysconf
#define FATOR_MAX_THREADS 4  /* margem de segurança acima do nº de núcleos */
#include "calculo.h"
#include "threads_func.h"
/*as duas estratégias abaixo percorrem a imagem por LINHA,  */
/* nunca por coluna. O buffer é armazenado de forma "row-major"    */
/* ==================================================================== */
/* PARTE 1: OpenMP (Seção 4.5.3)                                        */
/* ==================================================================== */

void definir_numero_threads(int num_threads) {
    if (num_threads <= 0) {
        fprintf(stderr, "Erro: numero de threads invalido (%d)\n", num_threads);
        exit(EXIT_FAILURE);
    }
    omp_set_num_threads(num_threads);
}

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

/* ==================================================================== */
/* PARTE 2: Pthreads - paralelismo de dados por LINHA (Seção 4.2.2)     */
/*                                                                        */
/*      */
/* (imagem[py * largura + px]), ou seja, pixels da MESMA linha ficam     */
/* contíguos na memória. Uma implementação que percorre por coluna       */
/* (px fixo, py variando) acessa endereços espaçados de `largura` bytes  */
/* a cada iteração — isso gera cache miss em quase todo acesso e pode    */
/* degradar o desempenho o suficiente para estourar o tempo limite de    */
/* execução em imagens grandes. Por isso essa abordagem foi eliminada.   */
/* ==================================================================== */

/* -------------------- pthreads1: Estratégia 1 (Intercalada/Cíclica) -------------------- */

typedef struct {
    int id_thread;
    int total_threads;
    unsigned char *imagem;
    const ParametrosMandelbrot *params;
} ArgCiclico;

static void *rotina_calcula_ciclico(void *arg) {
    ArgCiclico *a = (ArgCiclico *) arg;
    const ParametrosMandelbrot *p = a->params;

    /* Cada thread processa a linha id_thread, depois pula
     * total_threads linhas para a próxima: id, id+T, id+2T, ... */
    for (int py = a->id_thread; py < p->altura; py += a->total_threads) {
        for (int px = 0; px < p->largura; px++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            a->imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
    return NULL;
}

void mandelbrot_pthreads_ciclico(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads) {
    pthread_t *threads = malloc((size_t) num_threads * sizeof(pthread_t));
    ArgCiclico *args = malloc((size_t) num_threads * sizeof(ArgCiclico));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para threads\n");
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        args[i].id_thread     = i;
        args[i].total_threads = num_threads;
        args[i].imagem        = imagem;
        args[i].params        = p;

        if (pthread_create(&threads[i], NULL, rotina_calcula_ciclico, &args[i]) != 0) {
            fprintf(stderr, "Erro: falha ao criar a thread %d\n", i);
            free(threads);
            free(args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}

/* -------------------- pthreads2: Estratégia 2 (Blocos Contíguos) -------------------- */

typedef struct {
    int py_inicio;
    int py_fim;
    unsigned char *imagem;
    const ParametrosMandelbrot *params;
} ArgBloco;

static void *rotina_calcula_bloco(void *arg) {
    ArgBloco *a = (ArgBloco *) arg;
    const ParametrosMandelbrot *p = a->params;

    for (int py = a->py_inicio; py < a->py_fim; py++) {
        for (int px = 0; px < p->largura; px++) {
            double cr, ci;
            pixel_para_complexo(px, py, p->largura, p->altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, p->max_iteracoes);
            a->imagem[py * p->largura + px] = normaliza_intensidade(iteracoes, p->max_iteracoes);
        }
    }
    return NULL;
}
/*agora o percorrer linha virou esse percorrer por bloco 
por causa do modelo row-major de uma matriz em c */
void mandelbrot_pthreads_blocos(unsigned char *imagem, const ParametrosMandelbrot *p, int num_threads) {
    pthread_t *threads = malloc((size_t) num_threads * sizeof(pthread_t));
    ArgBloco *args = malloc((size_t) num_threads * sizeof(ArgBloco));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para threads\n");
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        /* Fatia [ (i*altura)/N , ((i+1)*altura)/N ) — cobre todas as
         * linhas exatamente uma vez, mesmo quando altura não é
         * múltiplo de num_threads (a divisão inteira absorve o resto
         * de forma automática e uniforme entre as fatias). */
        args[i].py_inicio = (int) (((long) i * p->altura) / num_threads);
        args[i].py_fim    = (int) (((long) (i + 1) * p->altura) / num_threads);
        args[i].imagem    = imagem;
        args[i].params    = p;

        if (pthread_create(&threads[i], NULL, rotina_calcula_bloco, &args[i]) != 0) {
            fprintf(stderr, "Erro: falha ao criar a thread %d\n", i);
            free(threads);
            free(args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}