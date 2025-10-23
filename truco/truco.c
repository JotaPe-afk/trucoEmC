#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CARTAS_MAO 3
#define MAX_PONTOS 12
#define NUM_CARTAS 40

typedef enum {
    PAUS = 0, COPAS = 1, ESPADAS = 2, OUROS = 3
} Naipe;

typedef struct {
    const char *nome;
    int valor;   
    Naipe naipe;
    bool ativo;  
	int valorAlternativo; 
} Carta;

typedef struct {
    Carta cartas[MAX_CARTAS_MAO];
} Mao;

typedef struct {
    Mao mao;
    int pontos;
    int pes;
} Jogador;

Carta baralho[NUM_CARTAS];
int proxCarta = 0; 


const char *valor_para_nome(int valor);
void iniciandoBaralho();
const char *convertedor_de_naipe(Naipe n);
void resetarBaralho();
void embaralharBaralho();
Carta distribuirCartas();

int main() {
	
	Jogador jogadores[2];
	
    srand((unsigned) time(NULL));
    iniciandoBaralho();
    embaralharBaralho();

  //  do {
    	for (int x = 0; x < 2; x++) {
        	for (int y = 0; y < MAX_CARTAS_MAO; y++) {
            	jogadores[x].mao.cartas[y] = distribuirCartas();
        	}
    	}

   

//	} while (jogadores[0].pes < 1 && jogadores[1].pes < 1);


	for (int i = 0; i < 2; i++) {
    printf("Jogador %d:\n", i + 1);
    for (int j = 0; j < MAX_CARTAS_MAO; j++) {
        Carta c = jogadores[i].mao.cartas[j];
        printf("  %s de %s (valor %d)\n",
               c.nome,
               convertedor_de_naipe(c.naipe),
               c.valor);
    }
}


    return 0;
}

const char *valor_para_nome(int valor) {
    switch (valor) {
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
        case PAUS:    return "Paus";
        case COPAS:   return "Copas";
        case ESPADAS: return "Espadas";
        case OUROS:   return "Ouros";
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
    proxCarta = 0;
}

void resetarBaralho() {
    for (int i = 0; i < NUM_CARTAS; i++) {
        baralho[i].ativo = false;
    }
    proxCarta = 0;
}

void embaralharBaralho() {
    for (int i = NUM_CARTAS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Carta tmp = baralho[i];
        baralho[i] = baralho[j];
        baralho[j] = tmp;
    }

    resetarBaralho();
}

/* distribui a próxima carta do baralho.
   Marca a carta como ativa e incrementa proxCarta.
   Se vazio, retorna Carta com nome == NULL */
   
Carta distribuirCartas() {
    if (proxCarta >= NUM_CARTAS) {
        Carta vazio = { NULL, 0, PAUS, true };
        return vazio;
    }
    Carta c = baralho[proxCarta];
    baralho[proxCarta].ativo = true;
    proxCarta++;
    return c;
}
