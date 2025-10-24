/* truco_interativo.c
 * Compilar: gcc truco_interativo.c -o truco
 * Executar: ./truco
 *
 * Jogo interativo simplificado: humano vs CPU (ou humano vs humano se preferir).
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CARTAS_MAO 3
#define NUM_CARTAS 40
#define MAX_PONTOS 12

typedef enum { PAUS = 0, COPAS = 1, ESPADAS = 2, OUROS = 3 } Naipe;

typedef struct {
    const char *nome;
    int valor;      // 1..10 (4..3)
    Naipe naipe;
    bool ativo;     // usada/distribuída
} Carta;

typedef struct {
    Carta cartas[MAX_CARTAS_MAO];
} Mao;

typedef struct {
    Mao mao;
    int pontos;
    int pes;
} Jogador;

/* Baralho global */
Carta baralho[NUM_CARTAS];
int proxCarta = 0;
Carta vira;
int manilha_valor = 0;

/* protótipos */
const char *valor_para_nome(int valor);
const char *convertedor_de_naipe(Naipe n);
void iniciandoBaralho();
void resetarBaralho();
void embaralharBaralho();
Carta distribuirCarta();
void distribuirMaoParaJogadores(Jogador jog[2]);
void definirViraEManilha();
int compararCartas(const Carta *a, const Carta *b, int manilha, const Naipe suit_priority[], int len);

/* interatividade */
int escolher_carta_usuario(Jogador *j, int jogador_id);
int escolher_carta_cpu(Jogador *j);
void mostrar_mao(Jogador *j, int jogador_id);

/* resolver mão (melhor de 3), retorna vencedor (0 ou 1) e atualiza pontos */
int resolver_mano_interativo(Jogador jogadores[2], int quem_comeca, int pontosRodada, const Naipe suit_priority[], int priority_len);

int main() {
    srand((unsigned int) time(NULL));
    const Naipe suit_priority[4] = { PAUS, COPAS, ESPADAS, OUROS }; // ordem de força (exemplo)

    Jogador jogadores[2];
    for (int i = 0; i < 2; ++i) { jogadores[i].pontos = 0; jogadores[i].pes = 0; }

    int jogadorHumano = 0; // 0 = jogador 1 é humano; se quiser humano ser jogador 2, mude para 1
    int vez_inicial = 0; // quem começa (0 ou 1)
    int pontosRodada = 1;

    /* loop de partidas até alguém alcançar MAX_PONTOS (exemplo) */
    while (jogadores[0].pontos < MAX_PONTOS && jogadores[1].pontos < MAX_PONTOS) {
        iniciandoBaralho();
        embaralharBaralho();
        resetarBaralho(); // marca livre (embaralharBaralho chama resetar, mas repetimos para segurança)

        distribuirMaoParaJogadores(jogadores);
        definirViraEManilha();

        printf("\n========= NOVA MÃO =========\n");
        printf("Vira: %s de %s\n", vira.nome, convertedor_de_naipe(vira.naipe));
        printf("Manilha: %s (valor interno %d)\n\n", valor_para_nome(manilha_valor), manilha_valor);

        printf("Placar atual: Jogador1=%d | Jogador2=%d\n\n", jogadores[0].pontos, jogadores[1].pontos);

        int vencedor = resolver_mano_interativo(jogadores, vez_inicial, pontosRodada, suit_priority, 4);

        printf("\nVencedor da mão: Jogador %d\n", vencedor + 1);

        /* definir quem começa na próxima mão: vencedor começa */
        vez_inicial = vencedor;

        /* pausa para o usuário ver */
        printf("\nPressione Enter para continuar para a próxima mão...");
        getchar();
    }

    printf("\nJogo acabou! Placar final: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
    return 0;
}

/* --- Implementações básicas do baralho --- */

const char *valor_para_nome(int valor) {
    switch (valor) {
        case 1: return "4"; case 2: return "5"; case 3: return "6"; case 4: return "7";
        case 5: return "Q"; case 6: return "J"; case 7: return "K"; case 8: return "A";
        case 9: return "2"; case 10: return "3"; default: return "?";
    }
}

const char *convertedor_de_naipe(Naipe n) {
    switch (n) { case PAUS: return "Paus"; case COPAS: return "Copas"; case ESPADAS: return "Espadas"; case OUROS: return "Ouros"; default: return "?"; }
}

void iniciandoBaralho() {
    int idx = 0;
    for (int v = 1; v <= 10; v++) {
        for (int s = 0; s < 4; s++) {
            baralho[idx].valor = v;
            baralho[idx].naipe = (Naipe)s;
            baralho[idx].nome = valor_para_nome(v);
            baralho[idx].ativo = false;
            idx++;
        }
    }
    proxCarta = 0;
}

/* marca todas as cartas como não usadas */
void resetarBaralho() {
    for (int i = 0; i < NUM_CARTAS; i++) baralho[i].ativo = false;
    proxCarta = 0;
}

/* embaralha o array de cartas (Fisher-Yates) */
void embaralharBaralho() {
    for (int i = NUM_CARTAS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Carta tmp = baralho[i];
        baralho[i] = baralho[j];
        baralho[j] = tmp;
    }
    // não marcar ativo aqui para permitir resetar depois se quiser
}

/* sorteia uma carta não usada */
Carta distribuirCarta() {
    int numero;
    do {
        numero = rand() % NUM_CARTAS;
    } while (baralho[numero].ativo);
    baralho[numero].ativo = true;
    return baralho[numero];
}

void distribuirMaoParaJogadores(Jogador jog[2]) {
    for (int p = 0; p < 2; ++p) {
        for (int i = 0; i < MAX_CARTAS_MAO; ++i) {
            jog[p].mao.cartas[i] = distribuirCarta();
        }
    }
}

void definirViraEManilha() {
    vira = distribuirCarta();
    manilha_valor = vira.valor + 1;
    if (manilha_valor > 10) manilha_valor = 1;
}

/* --- comparação de cartas --- */
/* priority: array de naipes em ordem de força (maior primeiro) */
int index_of_naipe(Naipe n, const Naipe priority[], int len) {
    for (int i = 0; i < len; ++i) if (priority[i] == n) return i;
    return len;
}

int compararCartas(const Carta *a, const Carta *b, int manilha, const Naipe suit_priority[], int len) {
    bool aMan = (a->valor == manilha);
    bool bMan = (b->valor == manilha);
    if (aMan && !bMan) return 1;
    if (!aMan && bMan) return -1;
    if (aMan && bMan) {
        int ia = index_of_naipe(a->naipe, suit_priority, len);
        int ib = index_of_naipe(b->naipe, suit_priority, len);
        if (ia < ib) return 1;
        if (ib < ia) return -1;
        return 0;
    }
    if (a->valor > b->valor) return 1;
    if (b->valor > a->valor) return -1;
    return 0;
}

/* --- interatividade: escolher carta --- */
void mostrar_mao(Jogador *j, int jogador_id) {
    printf("Mão do Jogador %d:\n", jogador_id + 1);
    for (int i = 0; i < MAX_CARTAS_MAO; ++i) {
        Carta c = j->mao.cartas[i];
        printf("  [%d] %s de %s (valor %d)%s\n", i + 1, c.nome, convertedor_de_naipe(c.naipe), c.valor, c.ativo ? "" : "");
    }
}

int escolher_carta_usuario(Jogador *j, int jogador_id) {
    int escolha = -1;
    while (1) {
        mostrar_mao(j, jogador_id);
        printf("Escolha a carta para jogar (1-%d): ", MAX_CARTAS_MAO);
        if (scanf("%d", &escolha) != 1) {
            while (getchar() != '\n'); // limpar buffer
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }
        if (escolha < 1 || escolha > MAX_CARTAS_MAO) {
            printf("Escolha fora do intervalo. Tente novamente.\n");
            continue;
        }
        int idx = escolha - 1;
        // aceitar sempre (no modelo atual não há cartas "removidas" da mão, apenas consumimos por índice)
        return idx;
    }
}

/* versão simples de CPU: escolhe a primeira carta disponível (pode melhorar) */
int escolher_carta_cpu(Jogador *j) {
    // estratégia simples: jogar primeiro índice
    for (int i = 0; i < MAX_CARTAS_MAO; ++i) {
        return i;
    }
    return 0;
}

/* --- resolver mão interativamente --- */
int resolver_mano_interativo(Jogador jogadores[2], int quem_comeca, int pontosRodada, const Naipe suit_priority[], int priority_len) {
    int manilha = manilha_valor;
    int winner_of_trick[3];
    for (int t = 0; t < 3; ++t) winner_of_trick[t] = -1;

    int primeiro = quem_comeca;
    int pos_jogado[2] = {0, 0}; // quantas cartas já jogadas (índice)

    for (int t = 0; t < 3; ++t) {
        printf("\n--- Truque %d ---\n", t + 1);
        int idx1, idx2;
        Carta carta1, carta2;

        if (primeiro == 0) {
            // jogador 0 joga primeiro
            if (/* humano é jogador 0 */ 1) { // aqui assumimos jogador 0 é humano; adapte conforme queira
                idx1 = escolher_carta_usuario(&jogadores[0], 0);
            } else {
                idx1 = escolher_carta_cpu(&jogadores[0]);
            }
            carta1 = jogadores[0].mao.cartas[idx1];

            // jogador 1 (CPU) joga
            idx2 = escolher_carta_cpu(&jogadores[1]);
            carta2 = jogadores[1].mao.cartas[idx2];
        } else {
            // jogador 1 joga primeiro
            idx2 = escolher_carta_cpu(&jogadores[1]);
            carta2 = jogadores[1].mao.cartas[idx2];

            idx1 = escolher_carta_usuario(&jogadores[0], 0);
            carta1 = jogadores[0].mao.cartas[idx1];
        }

        printf("Jogador %d jogou: %s de %s\n", primeiro + 1, (primeiro == 0 ? carta1.nome : carta2.nome), convertedor_de_naipe(primeiro == 0 ? carta1.naipe : carta2.naipe));
        printf("Jogador %d jogou: %s de %s\n", (1 - primeiro) + 1, (primeiro == 0 ? carta2.nome : carta1.nome), convertedor_de_naipe(primeiro == 0 ? carta2.naipe : carta1.naipe));

        int cmp = compararCartas(&carta1, &carta2, manilha, suit_priority, priority_len);
        if (cmp > 0) {
            // quem jogou carta1 vence
            if (primeiro == 0) {
                winner_of_trick[t] = 0; primeiro = 0;
                printf("Resultado: Jogador 1 vence o truque.\n");
            } else {
                winner_of_trick[t] = 1; primeiro = 1;
                printf("Resultado: Jogador 2 vence o truque.\n");
            }
        } else if (cmp < 0) {
            // quem jogou carta2 vence
            if (primeiro == 0) { winner_of_trick[t] = 1; primeiro = 1; printf("Resultado: Jogador 2 vence o truque.\n"); }
            else { winner_of_trick[t] = 0; primeiro = 0; printf("Resultado: Jogador 1 vence o truque.\n"); }
        } else {
            winner_of_trick[t] = -1;
            printf("Resultado: Empate no truque.\n");
            // quem começou permanece
        }

        // marcar cartas como "removidas" visualmente trocando nome (opcional)
        jogadores[0].mao.cartas[idx1].nome = "--";
        jogadores[1].mao.cartas[idx2].nome = "--";

        // verificar se alguém já tem 2 vitórias
        int cnt0 = 0, cnt1 = 0;
        for (int k = 0; k <= t; ++k) {
            if (winner_of_trick[k] == 0) cnt0++;
            else if (winner_of_trick[k] == 1) cnt1++;
        }
        if (cnt0 >= 2) { jogadores[0].pontos += pontosRodada; return 0; }
        if (cnt1 >= 2) { jogadores[1].pontos += pontosRodada; return 1; }
    }

    // após 3 truques, decidir vencedor por desempate
    int wins0 = 0, wins1 = 0;
    for (int i = 0; i < 3; ++i) {
        if (winner_of_trick[i] == 0) wins0++;
        else if (winner_of_trick[i] == 1) wins1++;
    }
    int vencedor = -1;
    if (wins0 > wins1) vencedor = 0;
    else if (wins1 > wins0) vencedor = 1;
    else if (wins0 == 0 && wins1 == 0) vencedor = quem_comeca; // 3 empates -> quem começou vence
    else {
        // 1 vitória cada e 1 empate -> quem venceu o último truque não empatado
        for (int i = 2; i >= 0; --i) {
            if (winner_of_trick[i] >= 0) { vencedor = winner_of_trick[i]; break; }
        }
    }

    if (vencedor >= 0) jogadores[vencedor].pontos += pontosRodada;
    return vencedor;
}
