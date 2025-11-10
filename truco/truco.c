#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "truco.h"

#define ALTURA_ARTE 6
#define LARGURA_ARTE 10

Carta baralho[40];
Carta vira;
int manilha_valor = 0;
int pontosRodada = 1;

// Artes fixas (uma por naipe)
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

    char simbolo = c.nome[0];
    arte[1][1] = simbolo;
}

void mostrar_mao(Jogador *jog, int jogadorNum) {
    printf("\nMao do Jogador %d:\n", jogadorNum + 1);

    char artes[MAX_CARTAS_MAO][ALTURA_ARTE][LARGURA_ARTE];

    for (int i = 0; i < MAX_CARTAS_MAO; i++) {
        if (jog->mao.cartas[i].ativo)
            gerar_arte_carta(jog->mao.cartas[i], artes[i]);
        else {
            for (int l = 0; l < ALTURA_ARTE; l++)
                strcpy(artes[i][l], "         ");
        }
    }

    for (int i = 0; i < MAX_CARTAS_MAO; i++)
        printf("   [%d]     ", i + 1);
    printf("\n");

    for (int linha = 0; linha < ALTURA_ARTE; linha++) {
        for (int i = 0; i < MAX_CARTAS_MAO; i++) {
            printf("%-9s  ", artes[i][linha]);
        }
        printf("\n");
    }
}

int escolherCarta(Jogador *jog, int jogadorNum) {
    int escolha, c;
    while (1) {
        mostrar_mao(jog, jogadorNum);
        printf("Jogador %d, escolha (1-%d): ", jogadorNum + 1, MAX_CARTAS_MAO);
        if (scanf("%d", &escolha) != 1) { 
            while ((c = getchar()) != '\n' && c != EOF); 
            continue; 
        }
        while ((c = getchar()) != '\n' && c != EOF);
        if (escolha < 1 || escolha > MAX_CARTAS_MAO) continue;
        int idx = escolha - 1;
        if (!jog->mao.cartas[idx].ativo) continue;
        jog->mao.cartas[idx].ativo = false;
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
    distribuirMaoParaJogadores(jog);
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
            baralho[index].nome = valor_para_nome(x);
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
    vira.nome = valor_para_nome(vira.valor);
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

int negociarTruco(int pontosAtuais, int iniciador, Jogador jogadores[2]) {
    int valores[] = {1, 3, 6, 9, 12};
    int idx = 0;
    for (int i = 0; i < 5; ++i) 
        if (valores[i] == pontosAtuais) { 
            idx = i; 
            break; 
        }
    
    int challenger = iniciador, responder = 1 - challenger;

    while (1) {
        if (idx >= 4) {
            int resp;
            printf("\nRodada já no máximo (%d). Jogador %d: [1] Aceitar  [3] Correr\n", valores[idx], responder + 1);
            if (scanf("%d", &resp) != 1) { 
                while (getchar() != '\n'); 
                continue; 
            }
            while (getchar() != '\n');
            
            if (resp == 1) return valores[idx];
            if (resp == 3) {
                jogadores[challenger].pontos += valores[idx];
                printf("Jogador %d correu. Jogador %d ganha %d ponto(s).\n", responder + 1, challenger + 1, valores[idx]);
                return -1;
            }
            continue;
        }

        int proximoValor = valores[idx + 1];
        printf("\nJogador %d pediu Truco para %d pontos! Jogador %d, responda: [1] Aceitar  [2] Aumentar  [3] Correr\n",
               challenger + 1, proximoValor, responder + 1);
        
        int resp;
        if (scanf("%d", &resp) != 1) { 
            while (getchar() != '\n'); 
            continue; 
        }
        while (getchar() != '\n');
        
        if (resp == 1) return proximoValor;
        else if (resp == 3) {
            jogadores[challenger].pontos += proximoValor;
            printf("Jogador %d correu. Jogador %d ganha %d ponto(s).\n", responder + 1, challenger + 1, proximoValor);
            return -1;
        } else if (resp == 2) {
            idx++;
            int novoChallenger = responder;
            responder = challenger;
            challenger = novoChallenger;
            continue;
        }
    }
}
