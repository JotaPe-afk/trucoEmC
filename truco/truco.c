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
    int valor;    // 1..10 (representa 4,5,6,7,Q,J,K,A,2,3)
    Naipe naipe;
    bool ativo;   // true se já foi distribuída
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
int proxCarta = 0; // índice da próxima carta no baralho embaralhado

/* protótipos */
const char *valor_para_nome(int valor);
void iniciandoBaralho();
const char *convertedor_de_naipe(Naipe n);
void resetarBaralho();
void embaralharBaralho();
Carta distribuirCartas();

int main() {
    srand((unsigned) time(NULL));
    iniciandoBaralho();
    embaralharBaralho();

    printf("Baralho embaralhado (lista completa):\n");
    for (int x = 0; x < NUM_CARTAS; x++) {
        printf("%2d: %s de %s (valor interno %d) - ativo=%s\n",
               x + 1,
               baralho[x].nome,
               convertedor_de_naipe(baralho[x].naipe),
               baralho[x].valor,
               baralho[x].ativo ? "SIM" : "NAO");
    }

    printf("\nDistribuindo 5 cartas como exemplo:\n");
    for (int i = 0; i < 5; i++) {
        Carta c = distribuirCartas();
        if (c.nome == NULL) {
            printf("Nenhuma carta disponível para distribuir.\n");
            break;
        }
        printf("-> Distribuída: %s de %s (valor %d)\n",
               c.nome, convertedor_de_naipe(c.naipe), c.valor);
    }

    return 0;
}

/* converte valor interno (1..10) para string do nome da carta */
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

/* converte enum Naipe para string legível */
const char *convertedor_de_naipe(Naipe n) {
    switch (n) {
        case PAUS:    return "Paus";
        case COPAS:   return "Copas";
        case ESPADAS: return "Espadas";
        case OUROS:   return "Ouros";
        default:      return "?";
    }
}

/* inicializa baralho em ordem (sem embaralhar ainda) */
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

/* reseta flags e ponteiro de próxima carta */
void resetarBaralho() {
    for (int i = 0; i < NUM_CARTAS; i++) {
        baralho[i].ativo = false;
    }
    proxCarta = 0;
}

/* embaralha o baralho usando Fisher-Yates */
void embaralharBaralho() {
    for (int i = NUM_CARTAS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        /* troca baralho[i] <-> baralho[j] */
        Carta tmp = baralho[i];
        baralho[i] = baralho[j];
        baralho[j] = tmp;
    }
    /* após embaralhar, nenhuma carta está marcada como distribuída */
    resetarBaralho();
}

/* distribui a próxima carta do baralho embaralhado.
   Marca a carta como ativa e incrementa proxCarta.
   Se esgotado, retorna Carta com nome == NULL */
Carta distribuirCartas() {
    if (proxCarta >= NUM_CARTAS) {
        /* baralho esgotado */
        Carta vazio = { NULL, 0, PAUS, true };
        return vazio;
    }
    Carta c = baralho[proxCarta];
    baralho[proxCarta].ativo = true;
    proxCarta++;
    return c;
}
