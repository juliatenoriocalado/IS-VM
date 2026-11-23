# Gerenciador de Memória Virtual

Implementação em C de um simulador de Gerenciador de Memória Virtual baseado no exercício *Designing a Virtual Memory Manager* (Silberschatz et al., *Operating System Concepts*, 10ª edição). O simulador opera com memória física reduzida a **128 frames**, suporte às políticas de substituição **FIFO** e **LRU** (para a Tabela de Páginas e para a TLB) e busca paralela na TLB utilizando **16 threads** (`pthread`).

## Requisitos do Ambiente

- Sistema operacional Linux, Unix ou macOS (ou WSL no Windows)
- Compilador `gcc` com suporte a POSIX Threads (`-pthread`)
- Utilitário `make`
- Arquivo binário `BACKING_STORE.bin` no mesmo diretório do executável

## Compilação e Limpeza

Para compilar o código-fonte e gerar o executável `vm`:

    make

Para remover o executável e os arquivos de saída gerados (`correct.txt` e `evidencias.log`):

    make clean

## Como Executar

O programa recebe 3 argumentos obrigatórios via linha de comando:

    ./vm <arquivo_de_enderecos> <algoritmo_pagina> <algoritmo_tlb>

Onde:
- `<arquivo_de_enderecos>`: caminho para o arquivo texto contendo os endereços lógicos (ex: `addresses.txt`).
- `<algoritmo_pagina>`: política de substituição de página (`fifo` ou `lru`).
- `<algoritmo_tlb>`: política de substituição da TLB (`fifo` ou `lru`).

### Exemplo de Execução

    ./vm addresses.txt lru fifo

## Saída Gerada

1. **Arquivo `correct.txt`**: contém a tradução linha a linha no formato `Virtual address: X TLB: Y Physical address: Z Value: W`, seguida das estatísticas finais de execução.
2. **Saída Padrão (Terminal)**: exibe o resumo das métricas (total de endereços traduzidos, quantidade e taxa de *Page Faults*, quantidade e taxa de *TLB Hits*).

## Tratamento de Erros

Antes e durante a execução, o programa valida e encerra com mensagem de erro coerente caso:
- A quantidade de argumentos seja diferente de 3 parâmetros além do executável (`argc != 4`).
- Os algoritmos informados sejam diferentes de `fifo` ou `lru`.
- O arquivo de endereços ou o `BACKING_STORE.bin` não existam ou não possam ser abertos.
- Alguma linha do arquivo de endereços contenha caracteres não numéricos, formatação inválida ou valores fora do intervalo de 16 bits (`0` a `65535`).