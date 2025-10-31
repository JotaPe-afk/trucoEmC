#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CARTAS_MAO 3
#define MAX_PONTOS 12

typedef enum { OUROS = 0, ESPADAS = 1, COPAS = 2, PAUS = 3 } Naipe;

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
    int rodadaGanha;
} Jogador;


Carta baralho[40];
Carta vira;
int manilha_valor = 0;
int pontosRodada;

const char *valor_para_nome(int valor);
const char *convertedor_de_naipe(Naipe n);
void iniciandoBaralho();
void resetarBaralho();
Carta distribuirCartas();
void definirViraEManilha();
int compararCartas(Carta c1, Carta c2);
void distribuirMaoParaJogadores(Jogador jog[2]);
void reiniciarRodada(Jogador jog[2]);
void resetarJogador(Jogador jog[2]);
int escolherCarta(Jogador *jog, int jogadorNum);
void mostrar_mao(Jogador *jog, int jogadorNum);
int negociarTruco(int pontosAtuais, int iniciador, Jogador jogadores[2]);

#define ALTURA_ARTE 6
#define LARGURA_ARTE 10

// Artes fixas (uma por naipe)
const char *artes_naipe[4][ALTURA_ARTE] = {
    { 
        " _____",
        "|  .  |",
        "| /.\\ |", //Espadas
        "|(_._)|",
        "|  |  |",
        "|_____|"
    },
    { 
        " _____",
        "|  ^  |",
        "| / \\ |", //Ouros
        "| \\ / |",
        "|  v  |",
        "|_____|"
    },
    { 
        " _____",
        "|  _  |",
        "| ( ) |", //Paus
        "|(_'_)|",
        "|  |  |",
        "|_____|"
    },
    { 
        " _____",
        "| _ _ |",
        "|( v )|", //
        "| \\ / |",
        "|  v  |",
        "|_____|"
    }
};

void gerar_arte_carta(Carta c, char arte[ALTURA_ARTE][LARGURA_ARTE]) {
    int naipeIndex = c.naipe;

    // Copia arte base
    for (int i = 0; i < ALTURA_ARTE; i++) {
        strcpy(arte[i], artes_naipe[naipeIndex][i]);
    }

    // Pega o símbolo da carta (A, K, Q, J, 7 etc.)
    char simbolo = c.nome[0];

    // Substitui nos cantos
    arte[1][1] = simbolo;  // canto superior esquerdo
    
}


/* int main() {
    srand((unsigned)time(NULL));
    iniciandoBaralho();
    Jogador jogadores[2];
    int primeiro, segundo, escPrimeiro, escSegundo;
    Carta cartaPrimeiro, cartaSegundo;
    int vezJogar = 0, compCarta, vencedor;
    resetarJogador(jogadores);

    do {
        if (jogadores[0].pontos >= MAX_PONTOS) { jogadores[0].pes++; jogadores[0].pontos = 0; }
        else if (jogadores[1].pontos >= MAX_PONTOS) { jogadores[1].pes++; jogadores[1].pontos = 0; }

        reiniciarRodada(jogadores);
        for (int i = 0; i < 2; ++i) jogadores[i].rodadaGanha = 0;
        primeiro = vezJogar;

        while (jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
            system("cls");
            printf("\n=== Rodada ===\n");
            printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
            printf("Placar: Jogador 1=%d | Jogador 2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
            printf("Valor da rodada: %d\n", pontosRodada);

            int opcao;
            printf("\nJogador %d: 1-Jogar carta  2-Pedir Truco\n", primeiro + 1);
            scanf("%d", &opcao);
            getchar();

            if (opcao == 2) {
                int r = negociarTruco(pontosRodada, primeiro, jogadores);
                if (r == -1) break;
                pontosRodada = r;
            }

            escPrimeiro = escolherCarta(&jogadores[primeiro], primeiro);
            cartaPrimeiro = jogadores[primeiro].mao.cartas[escPrimeiro];
            
			segundo = 1 - primeiro;	
			
            printf("\nJogador %d: 1-Jogar carta  2-Pedir Truco\n", primeiro + 1);
            scanf("%d", &opcao);
            getchar();

            if (opcao == 2) {
                int r = negociarTruco(pontosRodada, primeiro, jogadores);
                if (r == -1) break;
                pontosRodada = r;
            }	
				
            
            escSegundo = escolherCarta(&jogadores[segundo], segundo);
            cartaSegundo = jogadores[segundo].mao.cartas[escSegundo];

            compCarta = compararCartas(cartaPrimeiro, cartaSegundo);

            if (compCarta == -1) { primeiro = segundo; jogadores[segundo].rodadaGanha++; printf("Jogador %d ganhou\n", segundo + 1); }
            else if (compCarta == 0) { jogadores[primeiro].rodadaGanha++; jogadores[segundo].rodadaGanha++; printf("Empate\n"); }
            else { jogadores[primeiro].rodadaGanha++; printf("Jogador %d ganhou\n", primeiro + 1); }

            printf("Pressione Enter para continuar...");
            getchar();
        }

        if (jogadores[0].rodadaGanha == 2) { jogadores[0].pontos += pontosRodada; vencedor = 0; }
        else if (jogadores[1].rodadaGanha == 2) { jogadores[1].pontos += pontosRodada; vencedor = 1; }
        else vencedor = vezJogar;

        vezJogar = vencedor;

        printf("\nPressione Enter para próxima rodada...");
        getchar();

    } while (jogadores[0].pes < 1 && jogadores[1].pes < 1);

    system("cls");
    if (jogadores[0].pes == 1) printf("O Vencedor é o JOGADOR 1\n");
    else printf("O Vencedor é o JOGADOR 2\n");

    return 0;
} */

void mostrar_mao(Jogador *jog, int jogadorNum) {
    printf("\nMao do Jogador %d:\n", jogadorNum + 1);

    // Gera arte de cada carta
    char artes[MAX_CARTAS_MAO][ALTURA_ARTE][LARGURA_ARTE];

    for (int i = 0; i < MAX_CARTAS_MAO; i++) {
        if (jog->mao.cartas[i].ativo)
            gerar_arte_carta(jog->mao.cartas[i], artes[i]);
        else {
            // Carta usada → mostra carta vazia
            for (int l = 0; l < ALTURA_ARTE; l++)
                strcpy(artes[i][l], "         ");
        }
    }

    // Mostrar índices acima das cartas
    for (int i = 0; i < MAX_CARTAS_MAO; i++)
        printf("   [%d]     ", i + 1);
    printf("\n");

    // Mostrar todas as linhas das cartas lado a lado
    for (int linha = 0; linha < ALTURA_ARTE; linha++) {
    for (int i = 0; i < MAX_CARTAS_MAO; i++) {
        printf("%-9s  ", artes[i][linha]); // <- garante largura fixa
    }
    printf("\n");
}
}


int escolherCarta(Jogador *jog, int jogadorNum) {
    int escolha, c;
    while (1) {
        mostrar_mao(jog, jogadorNum);
        printf("Jogador %d, escolha (1-%d): ", jogadorNum + 1, MAX_CARTAS_MAO);
        if (scanf("%d", &escolha) != 1) { while ((c = getchar()) != '\n' && c != EOF); continue; }
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
    do { numeroCarta = rand() % 40; } while (baralho[numeroCarta].ativo);
    baralho[numeroCarta].ativo = true;
    return baralho[numeroCarta];
}

void definirViraEManilha() {
    vira = distribuirCartas();
    manilha_valor = vira.valor + 1;
    if (manilha_valor > 10) manilha_valor = 1;
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
    for (int i = 0; i < 5; ++i) if (valores[i] == pontosAtuais) { idx = i; break; }
    int challenger = iniciador, responder = 1 - challenger;

    while (1) {
        if (idx >= 4) {
            int resp;
            printf("\nRodada já no máximo (%d). Jogador %d: [1] Aceitar  [3] Correr\n", valores[idx], responder + 1);
            if (scanf("%d", &resp) != 1) { while (getchar() != '\n'); continue; }
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
        if (scanf("%d", &resp) != 1) { while (getchar() != '\n'); continue; }
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
