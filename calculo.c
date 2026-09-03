#include <stdio.h>
#include <stdlib.h>
#include "calculo.h"

/* Região fixa do plano complexo exigida pelo enunciado */
#define MANDELBROT_RE_MIN -2.0
#define MANDELBROT_RE_MAX  1.0
#define MANDELBROT_IM_MIN -1.5
#define MANDELBROT_IM_MAX  1.5

/* ------------------------------------------------------------------ */
/* Aplica repetidamente z_novo = z_atual^2 + c, partindo de z0 = 0,     */
/* até que |z| ultrapasse 2 (ponto "explode", fora do conjunto) ou até  */
/* atingir max_iteracoes (ponto considerado dentro do conjunto).       */
/*                                                                      */
/* Comparamos o quadrado do módulo (z_re^2 + z_im^2) com 4.0 em vez de  */
/* comparar o módulo com 2.0 diretamente, para evitar uma chamada a     */
/* sqrt() a cada iteração — resultado idêntico, porém mais rápido.      */
/* ------------------------------------------------------------------ */
long mandelbrot_point(double cr, double ci, long max_iteracoes) {
    double z_re = 0.0, z_im = 0.0;
    long iter = 0;

    while (z_re * z_re + z_im * z_im <= 4.0 && iter < max_iteracoes) {
        double z_re_novo = z_re * z_re - z_im * z_im + cr;
        z_im = 2.0 * z_re * z_im + ci;
        z_re = z_re_novo;
        iter++;
    }
    return iter;
}

/* ------------------------------------------------------------------ */
/* Converte a posição de um pixel (px, py) para o ponto correspondente*/
/* do plano complexo, mapeando linearmente:                           */
/*   px em [0, largura)  -> cr em [RE_MIN, RE_MAX]                    */
/*   py em [0, altura)   -> ci em [IM_MIN, IM_MAX]                    */
/*                                                                    */
/* Essa correspondência precisa ser usada por TODAS as implementações */
/* (serial, OpenMP, Pthreads) para garantir que os quatro arquivos de */
/* saída sejam idênticos entre si, como exige o enunciado.            */
/* ------------------------------------------------------------------ */
void pixel_para_complexo(int px, int py, int largura, int altura,
                          double *cr, double *ci) {
    double delta_re = (MANDELBROT_RE_MAX - MANDELBROT_RE_MIN) / largura;
    double delta_im = (MANDELBROT_IM_MAX - MANDELBROT_IM_MIN) / altura;

    *cr = MANDELBROT_RE_MIN + px * delta_re;
    *ci = MANDELBROT_IM_MIN + py * delta_im;
}

/* ------------------------------------------------------------------ */
/* Normaliza o número de iterações para uma intensidade de 0 a 255,     */
/* proporcional ao número de iterações realizadas, como pede o          */
/* enunciado. Pontos que "explodem" rápido (poucas iterações) recebem   */
/* intensidade baixa; pontos dentro do conjunto (max_iteracoes          */
/* iterações) recebem intensidade máxima (255).                        */
/* ------------------------------------------------------------------ */
unsigned char normaliza_intensidade(long iteracoes, long max_iteracoes) {
    if (max_iteracoes <= 0) {
        return 0;
    }
    /* Divisao inteira (trunca a parte decimal), para bater com o
     * criterio de normalizacao usado no gabarito/testes do professor. */
    long intensidade = (iteracoes * 255) / max_iteracoes;

    if (intensidade < 0)   intensidade = 0;
    if (intensidade > 255) intensidade = 255;

    return (unsigned char) intensidade;
}

/* ------------------------------------------------------------------ */
/* Aloca o buffer de imagem: largura * altura bytes, um por pixel.      */
/* Retorna NULL em caso de falha — quem chamar é responsável por        */
/* checar o retorno e encerrar com mensagem de erro coerente, como      */
/* exige o enunciado (falha de alocação de memória).                    */
/* ------------------------------------------------------------------ */
unsigned char *aloca_imagem(int largura, int altura) {
    if (largura <= 0 || altura <= 0) {
        return NULL;
    }
    return (unsigned char *) malloc((size_t) largura * (size_t) altura * sizeof(unsigned char));
}

/* ------------------------------------------------------------------ */
/* Implementação SERIAL: percorre todos os pixels da imagem, sem        */
/* nenhum paralelismo, calculando o número de iterações de cada ponto   */
/* e já normalizando a intensidade. Serve como referência de            */
/* corretude: as versões OpenMP/Pthreads devem produzir byte a byte a   */
/* mesma saída que esta função.                                         */
/* ------------------------------------------------------------------ */
void calcula_mandelbrot_serial(unsigned char *imagem, int largura, int altura,
                                long max_iteracoes) {
    for (int py = 0; py < altura; py++) {
        for (int px = 0; px < largura; px++) {
            double cr, ci;
            pixel_para_complexo(px, py, largura, altura, &cr, &ci);

            long iteracoes = mandelbrot_point(cr, ci, max_iteracoes);
            imagem[py * largura + px] = normaliza_intensidade(iteracoes, max_iteracoes);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Escreve a imagem no formato exigido pelo enunciado: SEM cabeçalho    */
/* de formato, um valor de intensidade por pixel, separados por         */
/* espaço, com uma linha do arquivo para cada linha da imagem.          */
/* Retorna 0 em sucesso e -1 em falha (arquivo não pôde ser criado ou    */
/* houve erro de escrita) — quem chamar deve tratar esse retorno.       */
/* ------------------------------------------------------------------ */
int escreve_pgm(const char *caminho, const unsigned char *imagem, int largura, int altura) {
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida '%s'\n", caminho);
        return -1;
    }
    /* buffer para uma linha inteira: cada pixel usa no máx. "255 " (4 chars) */
    char *linha_buf = malloc((size_t)largura * 4 + 2);
    if (linha_buf == NULL) {
        fprintf(stderr, "Erro: falha ao alocar buffer de escrita\n");
        fclose(arquivo);
        return -1;
    }
    for (int py = 0; py < altura; py++) {
        int offset = 0;
        for (int px = 0; px < largura; px++) {
            int sep = (px < largura - 1) ? ' ' : '\n';
            offset += snprintf(linha_buf + offset, 6, "%d%c",
                                imagem[py * largura + px], sep);
        }
        if (fwrite(linha_buf, 1, offset, arquivo) != (size_t)offset) {
            fprintf(stderr, "Erro: falha ao escrever no arquivo '%s'\n", caminho);
            free(linha_buf);
            fclose(arquivo);
            return -1;
        }
    }
    free(linha_buf);
    if (fclose(arquivo) != 0) {
        fprintf(stderr, "Erro: falha ao fechar o arquivo '%s'\n", caminho);
        return -1;
    }
    return 0;
}