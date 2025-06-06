#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 104
#define BITS_PER_CHUNK 26
#define MAX_CHUNKS ((BUFFER_SIZE * 8) / BITS_PER_CHUNK)

#define MAX 64
#define MAX_LINE 32
#define BIT_DADOS 26
#define BIT_PARIDADE 5
#define TAMANHO_DA_MENSAGEM 31

// Obtem o tamanho do arquivo
long get_file_size(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);      // Vai até o fim do arquivo
    long size = ftell(f);       // Pega a posição atual = tamanho
    fclose(f);
    return size;
}

void uint32_to_bits31(uint32_t valor, unsigned char bits[TAMANHO_DA_MENSAGEM]) {
    for (int i = 0; i < TAMANHO_DA_MENSAGEM; i++) {
        bits[TAMANHO_DA_MENSAGEM -1 - i] = (valor >> i) & 1;
    }
}

uint32_t bits_to_uint32(const unsigned char bits[TAMANHO_DA_MENSAGEM]) {
    uint32_t valor = 0;
    for (int i = 0; i < TAMANHO_DA_MENSAGEM; i++) {
        valor |= ((uint32_t)(bits[i] & 1)) << (30 - i);
    }
    return valor;
}

int32_t decodifica (unsigned long int block) {
    unsigned long int decodded = 0;
    unsigned long int xor = 0;

    // realiza o xor dos bits
    for (int i = 0; i < TAMANHO_DA_MENSAGEM; i++) {
        if ((block >> i) & 1)
            xor^= (i+1);
    }
    
    // Se xor diferente de 0 inverte o bit que esta incorreto
    if (xor) 
        block^= 1 << (xor-1); 
    
    for (int i = 0, pot = 1, j = 0; i < TAMANHO_DA_MENSAGEM; i++) {
        if ((i+1) == pot) {
            pot*= 2;
        }
        else { // se nao eh uma potencia de 2 na posicao (bit de dado)
            unsigned int bit = (block >> i) & 1; // seleciona o i-esimo bit menos significativo
            decodded |= (bit << j); // coloca o bit na posicao correta
            j++;
        }
    }
    
    return decodded;
}

uint32_t encripta (uint32_t block) {
    unsigned int codded = 0;
    unsigned int xor = 0;

    for (int i = 0, pot = 1, j = 0; i < TAMANHO_DA_MENSAGEM; i++) {
        if ((i+1) == pot) {
            pot *= 2;
        } else { // se nao eh uma potencia de 2 na posicao (bit de dado)
            unsigned int bit = (block >> j) & 1; // seleciona o i-esimo bit menos significativo
            codded |= (bit << i); // coloca o bit na posicao correta
            if (bit)
                xor ^= (i+1); // se o bit de dado for 1, sera feito seu xor
            j++;
        }
    }

    // coloca os bits de validacao (valor do xor) nas posicoes corretas
    for (int pot = 1, i = 0; i < BIT_PARIDADE; pot *= 2, i++) {
        codded |= (((xor >> i) & 1) << (pot - 1));
    }
    return codded;    
}

// Le 26 bits do buffer a partir de bit_pos
// Retorna os bloco de 26 bits nos bits menos significativos
uint32_t read_26_bits(const unsigned char *buffer, int bit_pos, int buffer_size) {
    int byte_pos = bit_pos / 8; //indice a ser lido no vetor
    int bit_offset = bit_pos % 8; // inicio dos bits a serem lidos no elemento

    uint64_t chunk = 0;
    // Considera o caso do inicio do bloco estar no final do elemento (5 bytes sao varridos)
    for (int i = 0; i < 5; i++) {
        chunk <<= 8; // da espaco no bloco para um novo byte
        if (byte_pos + i < buffer_size) {
            chunk |= buffer[byte_pos + i];
        }
    }

    chunk >>= (40 - bit_offset - BITS_PER_CHUNK); // Dos 5 bytes lidos descarta os bits que estão depois do bloco de 26 bits
    return chunk & 0x3FFFFFF; // retorna os 26 bits menos significativos
}

// Le blocos de 26 bits de buffer
// Codifica os blocos de 26 bits em blocos de 31 bits no buffer de saida (31 bits na posicao menos significativa)
// Retorna o numero de blocos codificados e vetor de blocos codificados em buffer_saida
int hamming_buffer(unsigned char *buffer, int buffer_size, uint32_t **buffer_saida) {
    int total_bits = buffer_size * 8;
    int saida_size = (total_bits + (BITS_PER_CHUNK - 1)) / BITS_PER_CHUNK;  //divide o numero total de bits do buffer pelo tamanho do bloco (26) e arredonda o resultado para cima
    *buffer_saida = malloc(sizeof(uint32_t) * saida_size); // buffer que ira conter o blocos codificados de 31 bits
    if (!(*buffer_saida)) return 0; // erro de alocacao

    for (int i = 0; i < saida_size; i++) {
        int bit_pos = i * BITS_PER_CHUNK; // calcula o inicio do proximo bloco de 26 bits
        uint32_t val = read_26_bits(buffer, bit_pos, buffer_size); // le o bloco de 26 bits a partir do bit_pos, nos bits menos significativos de val
        uint32_t codded = encripta(val); // aplica hamming de 31 bits no bloco
        (*buffer_saida)[i] = codded;
    }

    return saida_size;
}

int main(int argc, char const *argv[]) {
    char in, option;

    if (argc < 2) {
        printf ("Uso: ./hamming <nome_do_arquivo>\n");
        return 1;
    }

    char nome[100];

    
    // Copia o nome original para `base`
    strncpy(nome, argv[1], 100);
    nome[sizeof(nome) - 1] = '\0'; // segurança

    // remove extensao
    char *ponto = strrchr(nome, '.');
    if (ponto != NULL) {
        *ponto = '\0'; // corta a string no ponto
    }
    

    printf ("digite <c> para codicar arquivo\n");
    printf ("digite <d> para decodificar arquivo\n");
    scanf ("%c", &in);
    switch (in)
    {
    case 'c':
        option = 1;
        break;
    case 'd':
        option = 0;
        break;
    case 'C':
        option = 1;
        break;
    case 'D':
        option = 0;
        break;
    default:
        printf ("opcao invalida\n");
        return 1;
    }

    if (option) {
        char nome_saida[120];
        snprintf(nome_saida, sizeof(nome_saida), "%s.hamming", nome);

        int fd_read = open(argv[1], O_RDONLY);
        if (fd_read < 0) {
            printf ("arquivo nao encontrado\n");
            return 1;
        }

        int fd_write = open(nome_saida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_write < 0) {
            printf ("falha ao criar arquivo de saida\n");
            return 1;
        }
        
        long original_size = get_file_size(argv[1]);
        if (original_size < 0 || original_size > 0x3FFFFFF) return 1; // arquivo nao existe ou tamanho em bytes nao cabe em 26 bits
        
        // Codifica o tamanho do arquivo original
        uint32_t size_block = encripta((uint32_t)original_size);
        for (int bit = 30; bit >= 0; bit--) {
            char c = ((size_block >> bit) & 1) ? '1' : '0';
            write(fd_write, &c, 1);
        }
        write(fd_write, "\n", 1);

        unsigned char buffer[BUFFER_SIZE];
        int buffer_size = read(fd_read, buffer, BUFFER_SIZE);
        while (buffer_size > 0) {
            uint32_t *saida; // vetor com valores codificados em blocos de 31 bits
            int tamanho_saida = hamming_buffer(buffer, buffer_size, &saida);

            // escreve vetor codificado
            int total_bits = tamanho_saida * 31;
            unsigned char bits[total_bits + tamanho_saida]; // +1 por '\n' a cada bloco

            int bit_index = 0;

            for (int i = 0; i < tamanho_saida; i++) {
                for (int bit = 30; bit >= 0; bit--) {
                    bits[bit_index++] = ((saida[i] >> bit) & 1) ? '1' : '0';
                }
                bits[bit_index++] = '\n';
            }

            write(fd_write, bits, bit_index);


            free(saida);
            buffer_size = read(fd_read, buffer, BUFFER_SIZE);
        }

        close(fd_read);
        close(fd_write);

    }
    else {
        char nome_saida[120];
        snprintf(nome_saida, sizeof(nome_saida), "%s.dec", nome);
        
        FILE *fd_read = fopen(argv[1], "r");
        FILE *fd_write = fopen(nome_saida, "wb");
        if (!fd_read) {
            printf ("erro ao abrir arquivo\n");
            return 1;
        }
        if (!fd_write) {
            printf ("erro ao criar arquivo de saida\n");
            return 1;
        }

        char linha[40];
        unsigned char bits[TAMANHO_DA_MENSAGEM];

        // Lê o primeiro bloco contendo o tamanho original do arquivo
        fgets(linha, sizeof(linha), fd_read);
        for (int i = 0; i < TAMANHO_DA_MENSAGEM; i++) {
            bits[i] = (linha[i] == '1') ? 1 : 0;
        }
        uint32_t original_size_decoded = decodifica(bits_to_uint32(bits));

        uint64_t bit_buffer = 0; // acumula bits que nao formaram bytes completos
        int bit_count = 0; // conta os bits de bit_buffer
        int bytes_written = 0;

        // Le linha por linha codificada em hamming
        while (fgets(linha, sizeof(linha), fd_read) && bytes_written < original_size_decoded) {
            for (int i = 0; i < TAMANHO_DA_MENSAGEM; i++) {
                bits[i] = (linha[i] == '1') ? 1 : 0;
            }

            uint32_t codificado = bits_to_uint32(bits);
            uint32_t decodificado = decodifica(codificado) & 0x3FFFFFF;

            bit_buffer = (bit_buffer << BIT_DADOS) | decodificado; // abre espaco para ler proximo bloco de bits de dados 
            bit_count += BIT_DADOS;

            // Enquanto ha bytes inteiros eles sao escritos 
            while (bit_count >= 8 && bytes_written < original_size_decoded) {
                int shift = bit_count - 8; // calcula shift necessario para bit_buffer
                unsigned char byte = (bit_buffer >> shift) & 0xFF;
                fputc(byte, fd_write);
                bytes_written++;
                bit_count -= 8;
                bit_buffer &= ((uint64_t)1 << shift) - 1; // desloca os bits que ja foram escritos abrindo espaco para os proximos
            }
        }

        // Escreve bits restantes que nao completaram 1 byte inteiro
        if (bit_count > 0 && bytes_written < original_size_decoded) {
            // 0xFF garante que apenas o ultimo byte e considerado
            unsigned char byte = (bit_buffer << (8 - bit_count)) & 0xFF; // descola os bits restantes para a parte mais significativa do byte
            fputc(byte, fd_write);
        }

        fclose(fd_read);
        fclose(fd_write);
    }

    
    return 0;
}