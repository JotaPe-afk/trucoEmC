#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CARTAS_MAO 3
#define MAX_PONTOS 12

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
    int rodadaGanha;
} Jogador;


// baralho

Carta baralho[40];
Carta vira;
int manilha_valor = 0;

//rodada

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
void resetarJogador (Jogador jog[2]);
int escolherCarta(Jogador *jog, int jogadorNum);
void mostrar_mao(Jogador *jog, int jogadorNum);


int main() {
    srand((unsigned) time(NULL));
    iniciandoBaralho();
    
    int primeiro, segundo;
    int escPrimeiro, escSegundo;
    Carta cartaPrimeiro, cartaSegundo;

    Jogador jogadores[2];
    
    int vezJogar = 0;

    do {
    	
    	if(jogadores[0].pontos >= MAX_PONTOS){
    		jogadores[0].pes++;
    		jogadores[0].pontos = 0;
		}else if (jogadores[1].pontos >= MAX_PONTOS){
			jogadores[1].pes++;
    		jogadores[1].pontos = 0;
		}
    	
        reiniciarRodada(jogadores);
        resetarJogador(jogadores);
        
		printf("\n=== Rodada ===\n");
		printf("Vira: %s de %s\n", vira.nome, convertedor_de_naipe(vira.naipe));
		printf("Manilha: %s\n", valor_para_nome(manilha_valor));
		printf("Placar atual: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
		
		while(jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2){
			
        	printf("Quantidade de pontos: %d\n\n", pontosRodada);
        	
        	primeiro = vezJogar;
        	
        	
        	
		}	
			
        int vencedor = x;
        
        vezJogar = vencedor;

		printf("\nPressione Enter para continuar para a próxima rodada...");
        getchar();
       	

    } while (jogadores[0].pes < 1 && jogadores[1].pes < 1);
    
    if(jogadores[0].pes == 2){
    		system("cls");
    		printf("O Vencedor é o JOGADOR 1");
		}else if (jogadores[1].pontos >= MAX_PONTOS){
			system("cls");
    		printf("O Vencedor é o JOGADOR 1");
		}

    return 0;
}

// funcoes e procedimentos

void mostrar_mao(Jogador *jog, int jogadorNum) {
    printf("Mão do Jogador %d:\n", jogadorNum + 1);
    for (int i = 0; i < MAX_CARTAS_MAO; ++i) {
        Carta c = jog->mao.cartas[i];
        printf("  [%d] %s de %s\n", i + 1, c.nome, convertedor_de_naipe(c.naipe));
    }
}

int escolherCarta(Jogador *jog, int jogadorNum) {
    int escolha = -1;
    while (escolha < 1) {
        mostrar_mao(jog, jogadorNum);
        
        printf("Escolha a carta para jogar (1-%d): ", MAX_CARTAS_MAO);
        scanf("%d", &escolha);
        
        if (escolha != 1 && (escolha < 1 || escolha > MAX_CARTAS_MAO)) {
            while (getchar() != '\n'); 
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }
    
        
        return escolha-1;
    }
}

void distribuirMaoParaJogadores(Jogador jog[2]) {
    for (int p = 0; p < 2; ++p) {
        for (int i = 0; i < MAX_CARTAS_MAO; ++i) {
            jog[p].mao.cartas[i] = distribuirCartas();
        }
    }
}

void reiniciarRodada(Jogador jog[2]){
	system("cls");
	resetarBaralho();
    definirViraEManilha();
    pontosRodada = 1;
    
    for(int i=0; i<2; i++){
    	jog[i].rodadaGanha = 0;
	}

    distribuirMaoParaJogadores(jog);
}

void resetarJogador (Jogador jog[2]){
	for (int i = 0; i < 2; ++i) { 
		jog[i].pontos = 0; 
		jog[i].pes = 0; 
	}
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
        case OUROS: return "Ouros";
        case ESPADAS: return "Espadas";
        case COPAS: return "Copas";
        case PAUS: return "Paus";
        default: return "?";
    }
}

void iniciandoBaralho() {
    int index = 0;
    for (int x = 1; x <= 10; x++) {
        for (int y = 0; y < 4; y++) {
            baralho[index].valor = x;
            baralho[index].naipe = (Naipe)y;
            baralho[index].nome = valor_para_nome(x);
            baralho[index].ativo = false;
            index++;
        }
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
    if (c1.valor == c2.valor) return 0;
    return 0;
}
