#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int tlb_pagina[16];
int tlb_frame[16];

typedef struct {
    int indice_tlb;          
    int pagina_procurada;    
    int *frame_encontrado;  
    int *indice_encontrado; 
    pthread_mutex_t *cadeado; 
    // trava pra evitar conflito entre as threads
} DadosDasThreads;

void* buscar_na_tlb_thread(void* arg) {
    DadosDasThreads* dados = (DadosDasThreads*)arg;
    int idx = dados->indice_tlb;

    //cada thread  vai olhar APENAS a sua gaveta especifica da TLB
    if (tlb_pagina[idx] == dados->pagina_procurada) {
        pthread_mutex_lock(dados->cadeado); // tranca o cadeado
        *(dados->frame_encontrado) = tlb_frame[idx];
        *(dados->indice_encontrado) = idx;
        pthread_mutex_unlock(dados->cadeado); // destranca o cadeado
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    if (argc != 4){
        printf("Uso: ./vm <arquivo_enderecos> <fifo|lru> <fifo|lru>\n");
        return 1;
    }

    //descobrir qual algotirmo foi digitado no terminal
    int algo_pagina = -1; //0 = fifo ou 1 pra lru
    if (strcmp(argv[2], "fifo") == 0){
        algo_pagina = 0;
    }else if (strcmp(argv[2], "lru") == 0){
        algo_pagina = 1;
    }else{
        printf("Erro: Algoritmo invalido. Use 'fifo' ou 'lru'.\n");
        return 1;
    }

    int algo_tlb = -1;
    if (strcmp(argv[3], "fifo") == 0) algo_tlb = 0;
    else if (strcmp(argv[3], "lru") == 0) algo_tlb = 1;
    else{
        printf("Erro: Algoritmo de TLB invalido. Use 'fifo' ou 'lru'.\n");
        return 1;
    }

    FILE *arquivo_enderecos = fopen(argv[1], "r");
    if (arquivo_enderecos == NULL){
        printf("Erro ao abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    FILE *disco_rigido = fopen("BACKING_STORE.bin", "rb"); //rb = leitura binária
    if (disco_rigido == NULL){
        printf("Erro ao abrir BACKING_STORE.bin\n");
        fclose(arquivo_enderecos);
        return 1;
    }

    FILE *arquivo_saida = fopen("correct.txt", "w");
    if (arquivo_saida == NULL){
        fprintf(stderr, "Erro: Nao foi possivel criar o arquivo corret.txt\n");
        fclose(arquivo_enderecos);
        fclose(disco_rigido);
        return 1;
    }

    int tabela_paginas[256];
    for (int i=0; i<256; i++) tabela_paginas[i] = -1;
    
    signed char memoria_fisica[128*256];
    int proximo_frame_livre = 0;
    int memoria_cheia = 0;

    int frame_para_pagina[128];
    int ultimo_uso_frame[128];
    for (int i=0; i<128; i++){
        frame_para_pagina[i] = -1;
        ultimo_uso_frame[i] = 0;
    }

    int tlb_ultimo_uso[16];
    int tlb_ponteiro_fifo = 0;
    for (int i=0; i < 16; i++){
        tlb_pagina[i] = -1;
        tlb_frame[i] = -1;
        tlb_ultimo_uso[i] = 0;
    }

    int endereco_logico;
    int contador = 0;
    int pagina_faltantes = 0;
    int tlb_hits = 0;
    int tempo_atual = 0;

    char linha[256];
    char lixo;

    while (fgets(linha, sizeof(linha), arquivo_enderecos) != NULL){
        //ignora as linhas em branco no fianl do arquivo 
        if (linha[0] == '\n' || linha[0] == '\r' || linha[0] == '\0') continue;
        //se a linha nao tiver exatamemte um numero inteiro ela cancela com erro
        if (sscanf(linha, " %d %c", &endereco_logico, &lixo) != 1 || endereco_logico < 0 || endereco_logico > 65535){
            fprintf(stderr, "Erro: Arquivo de enderecos com formatacao invalida na linha: %s", linha);
            fclose(arquivo_enderecos);
            fclose(disco_rigido);
            fclose(arquivo_saida);
            return 1;
        }

        tempo_atual++;
        int numero_pagina = (endereco_logico >> 8) & 0xFF;
        int offset = endereco_logico & 0xFF;

        int frame = -1;
        int indice_tlb_usado = -1;

        pthread_t threads[16]; //dispara as threads na tbl em paralelo
        DadosDasThreads dados_threads[16];
        pthread_mutex_t cadeado;
        pthread_mutex_init(&cadeado, NULL);

        for (int i=0;i<16;i++){
            dados_threads[i].indice_tlb = i;
            dados_threads[i].pagina_procurada = numero_pagina;
            dados_threads[i].frame_encontrado = &frame;
            dados_threads[i].indice_encontrado = &indice_tlb_usado;
            dados_threads[i].cadeado = &cadeado;
            pthread_create(&threads[i], NULL, buscar_na_tlb_thread, &dados_threads[i]);
        }

        //espera por todas as 16 threads terminarem a busca
        for (int i=0; i<16; i++){
            pthread_join(threads[i], NULL);
        }

        pthread_mutex_destroy(&cadeado);

        if (frame != -1){
            tlb_hits++;
            tlb_ultimo_uso[indice_tlb_usado] = tempo_atual;
        }else{
            //tlb misses vai na tabela de paginas
            frame = tabela_paginas[numero_pagina];

            if (frame == -1){
                pagina_faltantes++;

                if (memoria_cheia == 0){
                    frame = proximo_frame_livre;
                    proximo_frame_livre++;
                    if (proximo_frame_livre == 128){
                        memoria_cheia = 1;
                        proximo_frame_livre = 0;
                    }
                }else{
                    if (algo_pagina == 0){
                        frame = proximo_frame_livre;
                        proximo_frame_livre = (proximo_frame_livre + 1) % 128;
                    }else{
                        int menor_tempo = ultimo_uso_frame[0];
                        frame = 0;
                        for (int i=1; i<128; i++){
                            if (ultimo_uso_frame[i] < menor_tempo){
                                menor_tempo = ultimo_uso_frame[i];
                                frame = i;
                            }
                        }
                    }
        
                    int pagina_antiga = frame_para_pagina[frame];
                    tabela_paginas[pagina_antiga] = -1;
                    
                    for (int i=0; i<16; i++){
                        if (tlb_pagina[i] == pagina_antiga){
                            tlb_pagina[i] = -1;
                        }
                    }
                }

                //busca no disco e grava o frame escolhido
                fseek(disco_rigido, numero_pagina * 256, SEEK_SET); //vai no disco rigido na posicao exata da pagina
                fread(&memoria_fisica[frame * 256], sizeof(signed char), 256, disco_rigido); //copia os 256 bytes do disco direto para a gaveta do frame da nossa ram //e anota a pagina onde mora o frame
                tabela_paginas[numero_pagina] = frame;
                frame_para_pagina[frame] = numero_pagina;
            }

            for (int i=0; i<16; i++){
                if (tlb_pagina[i] == -1){
                    indice_tlb_usado = i;
                    break;
                }
            }

            if (indice_tlb_usado == -1){
                if (algo_tlb == 0){
                    indice_tlb_usado = tlb_ponteiro_fifo;
                    tlb_ponteiro_fifo = (tlb_ponteiro_fifo + 1) % 16;
                }else{
                    int menor_tempo_tlb = tlb_ultimo_uso[0];
                    indice_tlb_usado = 0;
                    for (int i=1; i<16; i++){
                        if (tlb_ultimo_uso[i] < menor_tempo_tlb){
                            menor_tempo_tlb = tlb_ultimo_uso[i];
                            indice_tlb_usado = i;
                        }
                    }
                }
            }

            tlb_pagina[indice_tlb_usado] = numero_pagina;
            tlb_frame[indice_tlb_usado] = frame;
            tlb_ultimo_uso[indice_tlb_usado] = tempo_atual;
        }

        ultimo_uso_frame[frame] = tempo_atual;

        int endereco_fisico = (frame << 8) | offset;
        signed char valor = memoria_fisica[frame * 256 + offset];

        fprintf(arquivo_saida, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n", endereco_logico, indice_tlb_usado, endereco_fisico, valor);
        contador++;

    }

    printf("Number of Translated Addresses = %d\n", contador);
    printf("Page Faults = %d\n", pagina_faltantes);
    printf("Page Fault Rate = %.3f\n", (float)pagina_faltantes / contador);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (float)tlb_hits / contador);
  
    fclose(arquivo_enderecos);
    fclose(disco_rigido);
    return 0;
}