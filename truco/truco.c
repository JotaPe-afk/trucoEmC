#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_CARTAS 40
#define CARTAS_MAO 3

typedef struct {
    char nome[10];
    int valor;      // Valor base da carta (1 a 10)
    char naipe;     // O, E, C, P (Ouros, Espadas, Copas, Paus)
} Carta;

char *nomesCartas[] = {"4", "5", "6", "7", "Q", "J", "K", "A", "2", "3"};

void gerarBaralho(Carta baralho[]){
    char naipes[] = {'O', 'E', 'C', 'P'};
    int pos = 0;
    for (int n = 0; n < 4; n++) {
        for (int v = 1; v <= 10; v++) {
            Carta c;
            c.valor = v;
            c.naipe = naipes[n];
            sprintf(c.nome, "%s%c", nomesCartas[v - 1], naipes[n]);
            baralho[pos++] = c;
        }
    }
}

void embaralhar(Carta baralho[]) {
    for (int i = 0; i < NUM_CARTAS; i++) {
        int r = rand() % NUM_CARTAS;
        Carta temp = baralho[i];
        baralho[i] = baralho[r];
        baralho[r] = temp;
    }
}

int proximoValor(int valor) {
    return valor == 10 ? 1 : valor + 1;
}

int valorTruco(Carta c, int manilhas[4][2]) {
    // Se for manilha, retorna valor alto
    for (int i = 0; i < 4; i++) {
        if (c.valor == manilhas[i][0] && c.naipe == manilhas[i][1]) {
            return 15 + i; // cada manilha tem forca diferente
        }
    }

    // Mapeamento de forca das cartas (4 ate 3)
    int ordemForca[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // 1=4, 10=3
    return ordemForca[c.valor - 1];
}

void definirManilhas(Carta vira, int manilhas[4][2]) {
    int prox = proximoValor(vira.valor);
    char naipes[] = {'O', 'E', 'C', 'P'};
    for (int i = 0; i < 4; i++) {
        manilhas[i][0] = prox;
        manilhas[i][1] = naipes[i];
    }
}

void mostrarMao(Carta mao[], int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        if (mao[i].valor != -1)
            printf("[%d] %s  ", i + 1, mao[i].nome);
        else
            printf("[%d] (---)  ", i + 1);
    }
    printf("\n");
}

int compararCartas(Carta c1, Carta c2, int manilhas[4][2]) {
    int v1 = valorTruco(c1, manilhas);
    int v2 = valorTruco(c2, manilhas);
    if (v1 > v2) return 1;
    else if (v2 > v1) return -1;
    else return 0;
}

int main() {
    srand(time(NULL));

    Carta baralho[NUM_CARTAS];
    gerarBaralho(baralho);
    embaralhar(baralho);

    Carta maoJogador[CARTAS_MAO];
    Carta maoBot[CARTAS_MAO];

    for (int i = 0; i < CARTAS_MAO; i++) {
        maoJogador[i] = baralho[i];
        maoBot[i] = baralho[i + 3];
    }

    Carta vira = baralho[6];
    int manilhas[4][2];
    definirManilhas(vira, manilhas);

    printf("==== JOGO DE TRUCO ====\n");
    printf("Vira: %s\n", vira.nome);
    printf("Manilhas da rodada:\n");
    for (int i = 0; i < 4; i++) {
        printf("- %s%c\n", nomesCartas[manilhas[i][0] - 1], manilhas[i][1]);
    }
    printf("========================\n");

    int pontosJogador = 0, pontosBot = 0;

    for (int rodada = 1; rodada <= 3; rodada++) {
        printf("\nRodada %d\n", rodada);
        printf("Sua mao:\n");
        mostrarMao(maoJogador, CARTAS_MAO);

        int escolha;
        printf("Escolha sua carta (1-3): ");
        scanf("%d", &escolha);
        while (escolha < 1 || escolha > CARTAS_MAO || maoJogador[escolha - 1].valor == -1) {
            printf("Carta invalida! Escolha novamente: ");
            scanf("%d", &escolha);
        }

        Carta cartaJogador = maoJogador[escolha - 1];
        maoJogador[escolha - 1].valor = -1; // remove carta

        // Bot joga carta aleatoria
        int botEscolha;
        do {
            botEscolha = rand() % CARTAS_MAO;
        } while (maoBot[botEscolha].valor == -1);

        Carta cartaBot = maoBot[botEscolha];
        maoBot[botEscolha].valor = -1;

        printf("Voce jogou: %s\n", cartaJogador.nome);
        printf("Bot jogou: %s\n", cartaBot.nome);

        int resultado = compararCartas(cartaJogador, cartaBot, manilhas);

        if (resultado == 1) {
            printf("Voce venceu a rodada!\n");
            pontosJogador++;
        } else if (resultado == -1) {
            printf("Bot venceu a rodada!\n");
            pontosBot++;
        } else {
            printf("Empate!\n");
        }

        if (pontosJogador == 2 || pontosBot == 2) break;
    }

    printf("\n==== RESULTADO FINAL ====\n");
    if (pontosJogador > pontosBot) printf("Voce venceu a mao!\n");
    else if (pontosBot > pontosJogador) printf("Bot venceu a mao!\n");
    else printf("Empate na mao!\n");

    return 0;
}
