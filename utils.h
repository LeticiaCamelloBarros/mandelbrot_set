#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <time.h>

bool parse_positive_long(const char *str, const char *nome_param, long *resultado);
int long_para_int_seguro(long valor, const char *nome_param);
void inicializar_arquivo_tempos(const char *caminho);
void registrar_tempo(const char *caminho, const char *nome_implementacao, double segundos);
double diferenca_segundos(struct timespec inicio, struct timespec fim);

/* NOVA: valida que num_threads é positivo (cobre entradas negativas).
 * Encerra o programa com mensagem de erro coerente se falhar. */
void validar_num_threads(int num_threads);

#endif