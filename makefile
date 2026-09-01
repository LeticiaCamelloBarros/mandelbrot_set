# ==============================================================================
# Variáveis de Compilação
# ==============================================================================
CC      = gcc
# -O3 ativa otimizações agressivas de desempenho para código de uso intenso de CPU
# -Wall -Wextra exibem avisos importantes para evitar bugs
# -pthread e -fopenmp habilitam o suporte a Pthreads e OpenMP
CFLAGS  = -Wall -Wextra -O3 -pthread -fopenmp
# -lm vincula a biblioteca matemática padrão (math.h)
LDFLAGS = -pthread -fopenmp -lm

# ==============================================================================
# Fontes, Objetos e Executável
# ==============================================================================
TARGET  = mandelbrot

# Lista todos os seus arquivos de código-fonte (.c)
SRCS    = main.c calculo.c threads_func.c utils.c

# Gera automaticamente a lista de arquivos objeto (.o) correspondentes
OBJS    = $(SRCS:.c=.o)

# Lista todos os cabeçalhos (.h) para garantir que mudanças neles forcem a recompilação
DEPS    = calculo.h threads_func.h utils.h

# ==============================================================================
# Regras de Compilação
# ==============================================================================
# Alvo padrão (compila o executável)
all: $(TARGET)

# Regra para ligar todos os arquivos objeto (.o) e gerar o executável final
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regra genérica para compilar arquivos .c em arquivos objeto .o.
# Ela monitora tanto o arquivo .c quanto as definições nos cabeçalhos (.h)
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# Regra de Limpeza (Essencial para os Testes Automatizados)
# ==============================================================================
# Remove o binário, os arquivos objeto (.o), os tempos salvos e as imagens geradas (.pgm)
clean:
	rm -f $(TARGET) $(OBJS) times.txt *.pgm

# Define 'all' e 'clean' como alvos lógicos (evita conflitos com arquivos locais)
