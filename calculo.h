#ifndef CALCULO_H
#define CALCULO_H

typedef struct {
    int largura;
    int altura;
    long max_iteracoes;
} ParametrosMandelbrot;

long mandelbrot_point(double cr, double ci, long max_iteracoes);
void pixel_para_complexo(int px, int py, int largura, int altura, double *cr, double *ci);
unsigned char normaliza_intensidade(long iteracoes, long max_iteracoes);
unsigned char *aloca_imagem(int largura, int altura);
void calcula_mandelbrot_serial(unsigned char *imagem, int largura, int altura, long max_iteracoes);
int escreve_pgm(const char *caminho, const unsigned char *imagem, int largura, int altura);

#endif