#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "truco.h"

#define ALTURA_ARTE 6
#define LARGURA_ARTE 11

Carta baralho[40];
Carta vira;
int manilha_valor = 0;
int pontosRodada = 1;

const char *artes_naipe[4][ALTURA_ARTE] = {
    {
   		" _____",
        "|  ^  |",
        "| / \\ |",
        "| \\ / |",
        "|  v  |",
        "|_____|"
    },
    {
    	" _____",
        "|  .  |",
        "| /.\\ |",
        "|(_._)|",
        "|  |  |",
        "|_____|"
    },
    {
         " _____",
        "| _ _ |",
        "|( v )|",
        "| \\ / |",
        "|  v  |",
        "|_____|"
    },
    {
		" _____",
        "|  _  |",
        "| ( ) |",
        "|(_'_)|",
        "|  |  |",
        "|_____|"
    }
};

void gerar_arte_carta(Carta c, char arte[ALTURA_ARTE][LARGURA_ARTE]) {
    int naipeIndex = c.naipe;

    for (int i = 0; i < ALTURA_ARTE; i++) {
        strcpy(arte[i], artes_naipe[naipeIndex][i]);
    }

    if (c.nome != NULL && c.nome[0] != '\0') {
        arte[1][1] = c.nome[0];
    } else {
        arte[1][1] = '?';
    }

    if (c.nome != NULL && c.nome[0] != '\0') {
        arte[4][5] = c.nome[0];
    } else {
        arte[4][5] = '?';
    }
}

void mostrar_mao(Jogador *jog, int jogadorNum) {
    printf("\nMao do Jogador %d:\n", jogadorNum + 1);

    char artes[MAX_CARTAS_MAO][ALTURA_ARTE][LARGURA_ARTE];

    for (int i = 0; i < MAX_CARTAS_MAO; i++) {
        if (jog->mao.cartas[i].ativo)
            gerar_arte_carta(jog->mao.cartas[i], artes[i]);
        else {
            for (int l = 0; l < ALTURA_ARTE; l++)
                strcpy(artes[i][l], "          ");
        }
    }

    for (int i = 0; i < MAX_CARTAS_MAO; i++)
        printf("   [%d]     ", i + 1);
    printf("\n");

    for (int linha = 0; linha < ALTURA_ARTE; linha++) {
        for (int i = 0; i < MAX_CARTAS_MAO; i++) {
            printf("%-10s ", artes[i][linha]);
        }
        printf("\n");
    }
}

int escolherCarta(Jogador *jog, int jogadorNum) {
    int escolha;
    while (1) {
        mostrar_mao(jog, jogadorNum);
        printf("Jogador %d, escolha (1-%d): ", jogadorNum + 1, MAX_CARTAS_MAO);
        if (scanf("%d", &escolha) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        if (escolha < 1 || escolha > MAX_CARTAS_MAO) continue;
        int idx = escolha - 1;
        if (!jog->mao.cartas[idx].ativo) continue;

        return idx;
    }
}

void distribuirMaoParaJogadores(Jogador jog[2]) {
    for (int p = 0; p < 2; ++p)
        for (int i = 0; i < MAX_CARTAS_MAO; ++i)
            jog[p].mao.cartas[i] = distribuirCartas();
}

void reiniciarRodada(Jogador jog[2]) {
    system("cls");
    resetarBaralho();
    definirViraEManilha();
    pontosRodada = 1;
    for (int i = 0; i < 2; i++) jog[i].rodadaGanha = 0;
}

void resetarJogador(Jogador jog[2]) {
    for (int i = 0; i < 2; ++i) {
        jog[i].pontos = 0;
        jog[i].pes = 0;
        jog[i].rodadaGanha = 0;
    }
}

const char *valor_para_nome(int valor) {
    switch (valor) {
        case 1: return "4"; case 2: return "5"; case 3: return "6"; case 4: return "7";
        case 5: return "Q"; case 6: return "J"; case 7: return "K"; case 8: return "A";
        case 9: return "2"; case 10: return "3"; default: return "?";
    }
}

const char *convertedor_de_naipe(Naipe n) {
    switch (n) {
        case OUROS: return "Ouros"; case ESPADAS: return "Espadas";
        case COPAS: return "Copas"; case PAUS: return "Paus"; default: return "?";
    }
}

void iniciandoBaralho() {
    int index = 0;
    for (int x = 1; x <= 10; x++)
        for (int y = 0; y < 4; y++) {
            baralho[index].valor = x;
            baralho[index].naipe = (Naipe)y;
            strcpy(baralho[index].nome, valor_para_nome(x));
            baralho[index].ativo = false;
            index++;
        }
}

void resetarBaralho() {
    for (int i = 0; i < 40; i++)
        baralho[i].ativo = false;
}

Carta distribuirCartas() {
    int numeroCarta;
    do {
        numeroCarta = rand() % 40;
    } while (baralho[numeroCarta].ativo);

    baralho[numeroCarta].ativo = true;
    return baralho[numeroCarta];
}

void definirViraEManilha() {
    vira = distribuirCartas();
    manilha_valor = (vira.valor % 10) + 1;
    strcpy(vira.nome, valor_para_nome(vira.valor));
}

int compararCartas(Carta c1, Carta c2) {
    int pesosNaipe[] = {1, 2, 3, 4};
    bool c1Manilha = (c1.valor == manilha_valor);
    bool c2Manilha = (c2.valor == manilha_valor);

    if (c1Manilha && !c2Manilha) return 1;
    if (!c1Manilha && c2Manilha) return -1;
    if (c1Manilha && c2Manilha) {
        if (pesosNaipe[c1.naipe] > pesosNaipe[c2.naipe]) return 1;
        if (pesosNaipe[c1.naipe] < pesosNaipe[c2.naipe]) return -1;
        return 0;
    }
    if (c1.valor > c2.valor) return 1;
    if (c1.valor < c2.valor) return -1;
    return 0;
}