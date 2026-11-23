#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    if (argc <2){
        printf("Uso: ./vm <arquivo_enderecos>\n");
        return 1;
    }

    FILE *arquivo_enderecos = fopen(argv[1], "r");
    if (arquivo_enderecos == NULL){
        printf("Erro ao abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    int endereco_logico;
    int contador = 0;

    while (fscanf(arquivo_enderecos, "%d", &endereco_logico) == 1 && contador < 5){
        int numero_pagina = (endereco_logico >> 8) & 0xFF;
        int offset = endereco_logico & 0xFF;

        printf("Endereco lido: %d -> pagina: %d | offset (posicao dentro da pagina): %d\n", endereco_logico, numero_pagina, offset);
        contador++;
    }

    fclose(arquivo_enderecos);
    return 0;
}