#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Declaracao das funcoes das threads
void * normal(void * id);
void * expert(void * id);

// Mecanismos de Sincronizacao POSIX
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_normal = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_expert = PTHREAD_COND_INITIALIZER;

// Variaveis de Estado Globais
int normais_usando = 0;
int experts_usando = 0;
int normais_querem = 0;
int experts_querem = 0;
// Quantidade de Computadores
int total_computadores = 0;

int main(int argc, char *argv[]) {
    // Validacao dos argumentos de linha de comando (agora sao 4 argumentos no total contando com o nome do programa)
    if (argc != 4) {
        printf("Uso correto: %s <total_programadores> <total_experts> <total_computadores>\n", argv[0]);
        return 1;
    }

    int total_prog = atoi(argv[1]);
    int qtd_experts = atoi(argv[2]);
    total_computadores = atoi(argv[3]);

    if (total_prog < 3) {
        printf("Erro: O time precisa ter pelo menos 3 programador!\n");
        return 1;
    }
    if (qtd_experts < 0 || qtd_experts > total_prog) {
        printf("Erro: O numero de experts deve ser entre 0 e o total de programadores (%d).\n", total_prog);
        return 1;
    }
    if (total_computadores < 1) {
        printf("Erro: Deve haver pelo menos 1 computador na maratona!\n");
        return 1;
    }

    int qtd_normais = total_prog - qtd_experts;

    printf(" Iniciando Maratona \n");
    printf("Total: %d programadores (%d Normais, %d Experts). Computadores: %d\n", total_prog, qtd_normais, qtd_experts, total_computadores);
    printf("Pressione Ctrl+C para encerrar a simulacao.\n\n");

    int erro;
    int *id;

    // Alocacao dos arrays de threads
    pthread_t tNormal[qtd_normais];
    pthread_t tExpert[qtd_experts];

    // Criando as threads dos programadores NORMAIS
    if (qtd_normais > 0) {
        for (int i = 0; i < qtd_normais; i++) {
            id = (int *) malloc(sizeof(int));
            *id = i + 1; // ID comeca em 1
            erro = pthread_create(&tNormal[i], NULL, normal, (void *) (id));

            if (erro) {
                printf("Erro na criacao da thread Normal %d\n", i);
                exit(1);
            }
        }
    }

    // Criando as threads dos programadores EXPERTS
    if (qtd_experts > 0) {
        for (int i = 0; i < qtd_experts; i++) {
            id = (int *) malloc(sizeof(int));
            *id = i + 1; 
            erro = pthread_create(&tExpert[i], NULL, expert, (void *) (id));

            if (erro) {
                printf("Erro na criacao da thread Expert %d\n", i);
                exit(1);
            }
        }
    }

    // Aguarda a execucao indefinidamente
    if (qtd_normais > 0) {
        pthread_join(tNormal[0], NULL);
    } else if (qtd_experts > 0) {
        pthread_join(tExpert[0], NULL);
    }
    
    return 0;
}

// FUNCAO DOS PROGRAMADORES NORMAIS (Rating < 1600)
void * normal(void * a) {
    int i = *((int *) a);
    
    while(1) {
        // Fora da Regiao Critica: Pensando na solucao
        sleep(rand() % 5 + 4); 
        
        // Entrando na Fila
        pthread_mutex_lock(&mutex);
        normais_querem++;
        
        // Espera se os computadores estao ocupados OU se tem algum Expert na fila
        while ((normais_usando + experts_usando) >= total_computadores || experts_querem > 0) {
            pthread_cond_wait(&cond_normal, &mutex);
        }

        // Regiao Critica: Pegando o Computador
        normais_querem--;
        normais_usando++;
        printf("[NORMAL %d] Assumiu o PC! (PCs ocupados: %d/%d)\n", i, normais_usando + experts_usando, total_computadores);
        pthread_mutex_unlock(&mutex);
        
        // Simula o tempo codando e submetendo o problema
        sleep(3); 
        
        // Liberando o Computador
        pthread_mutex_lock(&mutex);
        normais_usando--;
        printf("[NORMAL %d] Liberou o PC.\n", i);
        
        // Notifica a proxima thread: Sempre verifica os Experts primeiro!
        if (experts_querem > 0) {
            pthread_cond_signal(&cond_expert);
        } else {
            pthread_cond_signal(&cond_normal);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

// FUNCAO DOS PROGRAMADORES EXPERTS (Rating >= 1600)
void * expert(void * a) {
    int i = *((int *) a);
    
    while(1) {
        // Fora da Regiao Critica: Pensando na solucao
        sleep(rand() % 3 + 2); 
        
        // Entrando na Fila
        pthread_mutex_lock(&mutex);
        experts_querem++;
    
        // Por ter prioridade maxima, so espera se os computadores estiverem em uso no limite maximo
        while ((normais_usando + experts_usando) >= total_computadores) {
            pthread_cond_wait(&cond_expert, &mutex);
        }

        // Regiao Critica: Pegando o Computador
        experts_querem--;
        experts_usando++;
        printf(">>> [EXPERT %d] Assumiu o PC! (PCs ocupados: %d/%d) <<<\n", i, normais_usando + experts_usando, total_computadores);
        pthread_mutex_unlock(&mutex);
        
        // Simula o tempo codando
        sleep(2); 
        
        // Liberando o Computador
        pthread_mutex_lock(&mutex);
        experts_usando--;
        printf("<<< [EXPERT %d] Liberou o PC.\n", i);
        
        // Notifica a proxima thread
        if (experts_querem > 0) {
            pthread_cond_signal(&cond_expert);
        } else {
            pthread_cond_signal(&cond_normal);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}