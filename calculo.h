#ifndef CALCULO_H
#define CALCULO_H
/* ------------------------------------------------------------------ */
/* Struct auxiliar com os parâmetros da região complexa e da imagem.   */
/* ------------------------------------------------------------------ */
typedef struct ParametrosMandelbrot{
    int largura;
    int altura;
    int max_iteracoes;
    double re_min, re_max; /* região real:      [-2.0, 1.0]  */
    double im_min, im_max; /* região imaginária: [-1.5, 1.5] */
} ParametrosMandelbrot;


/*
 * Calcula quantas iterações são necessárias até o ponto (cr, ci) "escapar"
 * (|z| > 2), até o limite max_iteracoes. Retorna max_iteracoes se o ponto
 * nunca escapar (considerado dentro do conjunto).
 */
long mandelbrot_point(double cr, double ci, long max_iteracoes);

/*
 * Converte a posição de um pixel (px, py) na imagem largura x altura para
 * as coordenadas (cr, ci) correspondentes no plano complexo.
 */
void pixel_para_complexo(int px, int py, int largura, int altura,double *cr, double *ci);

/*
 * Normaliza um número de iterações (0..max_iteracoes) para uma intensidade
 * de pixel entre 0 e 255, proporcional ao número de iterações realizadas.
 */
unsigned char normaliza_intensidade(long iteracoes, long max_iteracoes);

/*
 * Aloca um buffer de imagem (largura * altura bytes), um byte por pixel.
 * Retorna NULL em caso de falha de alocação (o chamador deve checar).
 */
unsigned char *aloca_imagem(int largura, int altura);

/*
 * Preenche um buffer de imagem já alocado calculando o conjunto de
 * Mandelbrot para cada pixel, de forma sequencial (sem paralelismo).
 * Essa função serve de base para a versão Serial e pode ser usada como
 * referência de corretude para as versões paralelas (OpenMP/Pthreads),
 * desde que estas também chamem mandelbrot_point/normaliza_intensidade
 * para cada pixel, garantindo imagens idênticas.
 */
void calcula_mandelbrot_serial(unsigned char *imagem, int largura, int altura,long max_iteracoes);

/*
 * Escreve a imagem no formato exigido pelo enunciado: sem cabeçalho, um
 * valor de intensidade por pixel, separados por espaço, uma linha por
 * linha da imagem. Retorna 0 em sucesso e -1 em falha (arquivo não pôde
 * ser criado/escrito).
 */
int escreve_pgm(const char *caminho, const unsigned char *imagem,int largura, int altura);

#endif