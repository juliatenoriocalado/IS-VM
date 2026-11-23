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

    FILE *disco_rigido = fopen("BACKING_STORE.bin", "rb"); //rb = leitura binária
    if (disco_rigido == NULL){
        printf("Erro ao abrir BACKING_STORE.bin\n");
        fclose(arquivo_enderecos);
        return 1;
    } 

    //criando as nossas estruturas de memoria
    int tabela_paginas[256];
    for (int i=0;i<256;i++){
        tabela_paginas[i] = -1; // -1 significa: "Essa página ainda não está na RAM"
    }

    // nossa memória ram fisica: 256 frames de 256 bytes cada = 65.536 bytes
    //signed char para ler valores entre -128 e 127 (igual ao correct.txt)
    signed char memoria_fisica[256 * 256]; // um 256 é frame e o outro é byte
    int proximo_frame_livre = 0;

    int endereco_logico;
    int contador = 0;
    int pagina_faltantes = 0;

    while (fscanf(arquivo_enderecos, "%d", &endereco_logico) == 1 && contador < 5){
        int numero_pagina = (endereco_logico >> 8) & 0xFF;
        int offset = endereco_logico & 0xFF;

        int frame = tabela_paginas[numero_pagina];

        if (frame == -1){
            pagina_faltantes++;
            frame = proximo_frame_livre;
            proximo_frame_livre++;

            fseek(disco_rigido, numero_pagina * 256, SEEK_SET); //vai no disco rigido na posicao exata da pagina
            fread(&memoria_fisica[frame * 256], sizeof(signed char), 256, disco_rigido); //copia os 256 bytes do disco direto para a gaveta do frame da nossa ram
            tabela_paginas[numero_pagina] = frame; //e anota a pagina onde mora o frame
        }

        int endereco_fisico = (frame << 8) | offset; //so pra traducao e leitura do dado
        signed char valor = memoria_fisica[frame * 256 + offset];

        printf("Virtual address: %d Physical address: %d Value: %d\n", endereco_logico, endereco_fisico, valor);
        contador++;
    }

    printf("\nTotal de page faults nos 5 testes: %d\n", pagina_faltantes);

    fclose(arquivo_enderecos);
    fclose(disco_rigido);
    return 0;
}