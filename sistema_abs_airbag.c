#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>

// Intervalos de monitoramento em microsegundos (us)
#define TEMPO_AIRBAG_US 99000
#define TEMPO_ABS_US 99000

// Variáveis globais para WCET e WCRT
uint64_t wcet_abs = 0, wcet_airbag = 0;
uint64_t wcrt_abs = 0, wcrt_airbag = 0;

// Variáveis globais para sinalização de eventos
volatile int airbag_ativo = 0;
volatile int abs_ativo = 0;

// Funções de tempo em microsegundos
uint64_t obter_tempo_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
}

// Configuração do modo não bloqueante para entrada de teclado
void configurar_teclado() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);          // Salva configurações atuais
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);        // Desativa modo canônico e eco
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // Torna leitura não bloqueante
}

// Função para verificar entrada do teclado
void verificar_teclado() {
    char tecla = getchar();
    if (tecla == 'a') {
        airbag_ativo = 1;
    } else if (tecla == 's') {
        abs_ativo = 1;
    }
}

// Funções para threads
void *tempo_airbag(void *arg) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        tempo_inicio = obter_tempo_us();

        if (airbag_ativo) {
            printf("Airbag acionado!\n");
            airbag_ativo = 0; // Reseta sinal de ativação

            tempo_fim = obter_tempo_us();
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET para Airbag
            if (tempo_execucao > wcet_airbag) {
                wcet_airbag = tempo_execucao;
            }

            // Cálculo do WCRT para Airbag
            tempo_resposta = tempo_execucao + TEMPO_AIRBAG_US;
            if (tempo_resposta > wcrt_airbag) {
                wcrt_airbag = tempo_resposta;
            }

            // Verifica se o tempo de execução foi dentro do limite
            if (tempo_execucao <= TEMPO_AIRBAG_US) {
                printf("Airbag ativado dentro do prazo de 99ms.\n");
            } else {
                printf("Airbag ativado fora do prazo de 99ms.\n");
            }

            printf("WCET Airbag: %" PRIu64 " us, WCRT Airbag: %" PRIu64 " us\n", wcet_airbag, wcrt_airbag);
        }

        usleep(TEMPO_AIRBAG_US);
    }
    return NULL;
}

void *tempo_abs(void *arg) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        tempo_inicio = obter_tempo_us();

        if (abs_ativo) {
            printf("ABS acionado!\n");
            abs_ativo = 0; // Reseta sinal de ativação

            tempo_fim = obter_tempo_us();
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET para ABS
            if (tempo_execucao > wcet_abs) {
                wcet_abs = tempo_execucao;
            }

            // Cálculo do WCRT para ABS
            tempo_resposta = tempo_execucao + TEMPO_ABS_US;
            if (tempo_resposta > wcrt_abs) {
                wcrt_abs = tempo_resposta;
            }

            // Verifica se o tempo de execução foi dentro do limite
            if (tempo_execucao <= TEMPO_ABS_US) {
                printf("ABS ativado dentro do prazo de 99ms.\n");
            } else {
                printf("ABS ativado fora do prazo de 99ms.\n");
            }

            printf("WCET ABS: %" PRIu64 " us, WCRT ABS: %" PRIu64 " us\n", wcet_abs, wcrt_abs);
        }

        usleep(TEMPO_ABS_US);
    }
    return NULL;
}

void *monitorar_teclado(void *arg) {
    while (1) {
        verificar_teclado();
        usleep(1000); // Pequeno atraso para evitar alta carga de CPU
    }
    return NULL;
}

int main() {
    pthread_t thread_airbag, thread_abs, thread_teclado;
    struct sched_param param;

    // Configuração do teclado em modo não bloqueante
    configurar_teclado();

    // Configurações para threads em tempo real
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    // Criação das threads
    if (pthread_create(&thread_airbag, NULL, tempo_airbag, NULL) != 0) {
        perror("Falha ao criar a thread do Airbag");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_abs, NULL, tempo_abs, NULL) != 0) {
        perror("Falha ao criar a thread do ABS");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_teclado, NULL, monitorar_teclado, NULL) != 0) {
        perror("Falha ao criar a thread do teclado");
        exit(EXIT_FAILURE);
    }

    // Aguarda a execução das threads (infinito)
    pthread_join(thread_airbag, NULL);
    pthread_join(thread_abs, NULL);
    pthread_join(thread_teclado, NULL);

    return 0;
}
