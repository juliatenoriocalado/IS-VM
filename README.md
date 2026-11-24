# Gerenciador de Memória Virtual
 
Simulador em C de um gerenciador de memória virtual, baseado no exercício *Designing a Virtual Memory Manager* (Silberschatz et al., *Operating System Concepts*, 10ª edição, p. 51), com as modificações pedidas na atividade:
 
- memória física com **128 frames**;
- substituição de página por **FIFO** ou **LRU**;
- substituição da TLB por **FIFO** ou **LRU**;
- busca na TLB com **uma thread por entrada** (16 threads, `pthread`).
## Requisitos
 
- Linux, Unix ou macOS (no Windows, use WSL)
- `gcc` com suporte a pthreads
- `make`
- O arquivo `BACKING_STORE.bin` no mesmo diretório do executável (fornecido pelo professor)
- O arquivo de endereços lógicos, por exemplo `addresses.txt` (fornecido pelo professor)
## Compilação e limpeza
 
```
make          # gera o executável vm
make clean    # remove vm e correct.txt
```
 
O `make clean` não remove `evidencias.log`.
 
## Execução
 
```
./vm <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>
```
 
- `<arquivo_enderecos>`: arquivo com um endereço lógico por linha.
- `<algoritmo_pagina>`: `fifo` ou `lru`.
- `<algoritmo_tlb>`: `fifo` ou `lru`.
Exemplo:
 
```
./vm addresses.txt lru fifo
```
 
## Saída
 
- **`correct.txt`**: uma linha por endereço traduzido, no formato
  `Virtual address: X TLB: Y Physical address: Z Value: W`,
  seguida das estatísticas (total de endereços, page faults, taxa de page faults, TLB hits e taxa de TLB hits).
- **Terminal**: mostra as mesmas estatísticas ao final da execução.
## Tratamento de erros
 
O programa valida os parâmetros antes de processar. Ele exibe uma mensagem e encerra com código 1 quando:
 
- o número de argumentos é diferente de 3 (além do nome do executável);
- o algoritmo de página ou de TLB não é `fifo` nem `lru`;
- o arquivo de endereços não existe ou não pode ser aberto;
- o `BACKING_STORE.bin` não existe ou não pode ser aberto;
- o arquivo de endereços está vazio;
- alguma linha não é um inteiro entre 0 e 65535.
## Evidências (`evidencias.log`)
 
O arquivo reúne as saídas dos testes, organizadas em blocos:
 
| Bloco | Descrição | Commit do código |
|---|---|---|
| FLUXO 1 | 256 frames, sem TLB | `7d25d35` |
| FLUXO 2 | 128 frames, FIFO, sem TLB | `442f41b` |
| FLUXO 3 | 128 frames, LRU, sem TLB | `b94e3b4` |
| FLUXO 4 | TLB sem thread, FIFO e LRU | `c32a397` |
| FINAL | 128 frames, TLB com thread, as 4 combinações de algoritmos | versão final |
| TESTE DE ERRO | Validação de argumentos | versão final |
 
Para reproduzir uma etapa intermediária, copie o código daquele commit e compile:
 
```
git show <commit>:vm.c > vm.c
gcc -Wall -pthread -o vm vm.c
```
 
## Estrutura do repositório
 
- `vm.c`: código-fonte
- `Makefile`: compilação e limpeza
- `evidencias.log`: saídas dos testes
- `README.md`: este arquivo
O histórico de commits registra o desenvolvimento em etapas: leitura e fatiamento dos endereços, tabela de páginas com 256 frames, FIFO e LRU com 128 frames, TLB sem thread e TLB com thread.
 
