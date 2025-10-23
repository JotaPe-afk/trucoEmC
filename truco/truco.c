#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CARTAS_MAO 3
#define MAX_PONTOS 12

// VARIAVEIS GLOBAIS
typedef enum {
    OUROS = 0, ESPADAS = 1, COPAS = 2, PAUS = 3
} Naipe;

typedef struct {
    const char *nome;
    int valor;
    Naipe naipe;
    bool ativo;
} Carta;

typedef struct {
    Carta cartas[MAX_CARTAS_MAO];	
} Mao;

typedef struct {
    Mao mao;
    int pontos;
    int pes;
} Jogador;

Carta baralho[40];

// FUNCOES E PROCEDIMENTOS
const char *valor_para_nome(int valor);
void iniciandoBaralho();
const char *convertedor_de_naipe(Naipe n);

int main() {
    iniciandoBaralho();

    for(int x = 0; x < 40; x++) {
        printf("Nome: %s | Valor: %d | Naipe: %s\n",
               baralho[x].nome,
               baralho[x].valor,
               convertedor_de_naipe(baralho[x].naipe)
		);
    }

    return 0;
}

const char *valor_para_nome(int valor) {
    switch(valor) {
        case 1: return "4";
        case 2: return "5";
        case 3: return "6";
        case 4: return "7";
        case 5: return "Q";
        case 6: return "J";
        case 7: return "K";
        case 8: return "A";
        case 9: return "2";
        case 10: return "3";
        default: return "?";
    }
}

const char *convertedor_de_naipe(Naipe n) {
    switch (n) {
        case 0:    return "Paus";  // paus
        case 1:   return "Copas";  // copas
        case 2: return "Espadas";  // espadas
        case 3:   return "Ouros";  // ouros
        default:      return "?";
    }
}

void iniciandoBaralho() {
    int indexBaralho = 0;

    for (int x = 1; x <= 10; x++) {
        for (int y = 0; y < 4; y++) {
            baralho[indexBaralho].valor = x;
            baralho[indexBaralho].naipe = (Naipe)y;
            baralho[indexBaralho].nome = valor_para_nome(x);
            baralho[indexBaralho].ativo = false;
            indexBaralho++;
        }
    }
}
