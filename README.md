# Hamming (31, 26) - Codificação e Decodificação

Este programa implementa codificação e decodificação de arquivos usando o código de Hamming (31,26), que adiciona 5 bits de paridade a blocos de 26 bits de dados, totalizando 31 bits por bloco.

## Funcionalidades

- Codifica arquivos binários em blocos Hamming 31.
- Decodifica arquivos Hamming 31 e recupera os dados originais, corrigindo erros de 1 bit.
- Armazena o tamanho original do arquivo no início do arquivo codificado para reconstrução exata.

## Uso

Compilar o programa:

```bash
gcc -o hamming hamming.c
```

Executar:

```bash
./hamming arquivo.ext
```

Durante a execução, o usuário deve escolher:

- `c` ou `C`: codificar arquivo
- `d` ou `D`: decodificar arquivo

## Saídas

- Arquivo codificado: `arquivo.hamming`
- Arquivo decodificado: `arquivo.dec`

## Limitações

- O tamanho máximo do arquivo original é de 64MB (26 bits de bloco).

## Estrutura do Código

- `codifica` e `decodifica`: aplicam Hamming (31,26).
- `hamming_buffer`: converte buffer binário em blocos codificados.
- `read_26_bits`: extrai 26 bits de um buffer binário.
- Armazena cada bloco codificado como linha de 31 caracteres ('0' ou '1').
