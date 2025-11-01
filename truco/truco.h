#ifndef TRUCO_H
#define TRUCO_H

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

extern Carta baralho[40];
extern Carta vira;
extern int manilha_valor;
extern int pontosRodada;

void iniciandoBaralho();
void resetarBaralho(); // ADICIONAR ESTE PROTÓTIPO
void resetarJogador(Jogador jog[2]);
void reiniciarRodada(Jogador jog[2]);
void distribuirMaoParaJogadores(Jogador jog[2]);
int compararCartas(Carta c1, Carta c2);
const char *convertedor_de_naipe(Naipe n);
const char *valor_para_nome(int valor);
int negociarTruco(int pontosAtuais, int iniciador, Jogador jogadores[2]);
void mostrar_mao(Jogador *jog, int jogadorNum);
Carta distribuirCartas();
void definirViraEManilha();

#endif