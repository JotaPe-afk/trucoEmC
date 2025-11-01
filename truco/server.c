#include <stdio.h>
#include <winsock2.h>
#include "truco.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(51171);

    bind(sock, (struct sockaddr*)&server, sizeof(server));
    listen(sock, 1);

    printf("Aguardando o Jogador 2 conectar...\n");
    int clientSize = sizeof(client);
    SOCKET cliente = accept(sock, (struct sockaddr*)&client, &clientSize);
    printf("Jogador 2 conectado!\n");

    Jogador jogadores[2];
    srand((unsigned)time(NULL));
    iniciandoBaralho();
    resetarJogador(jogadores);

    char buffer[512];
    int vez = 0;

    while (1) {
        // Reiniciar rodada e sincronizar com cliente
        reiniciarRodada(jogadores);
        
        // Enviar vira para cliente
        sprintf(buffer, "VIRA:%d:%d", vira.valor, vira.naipe);
        send(cliente, buffer, strlen(buffer), 0);

        int rodada = 1;
        while (rodada <= 3 && jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
            printf("\n=== RODADA %d ===\n", rodada);
            printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
            printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);

            // Jogador 1 (servidor) joga
            mostrar_mao(&jogadores[0], 0);
            printf("Escolha sua carta (1-3): ");
            int carta1;
            scanf("%d", &carta1);
            
            Carta c1 = jogadores[0].mao.cartas[carta1 - 1];
            jogadores[0].mao.cartas[carta1 - 1].ativo = false;
            
            // Enviar carta do servidor para cliente
            sprintf(buffer, "CARTA:%d:%d:%d", c1.valor, c1.naipe, carta1);
            send(cliente, buffer, strlen(buffer), 0);

            // Receber carta do cliente
            printf("Aguardando Jogador 2...\n");
            recv(cliente, buffer, sizeof(buffer), 0);
            
            int carta2, valor2, naipe2;
            sscanf(buffer, "CARTA:%d:%d:%d", &valor2, &naipe2, &carta2);
            
            Carta c2 = jogadores[1].mao.cartas[carta2 - 1];
            jogadores[1].mao.cartas[carta2 - 1].ativo = false;

            printf("Jogador 2 jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));

            // Comparar cartas
            int resultado = compararCartas(c1, c2);
            if (resultado > 0) {
                printf("Você venceu esta rodada!\n");
                jogadores[0].rodadaGanha++;
            } else if (resultado < 0) {
                printf("Jogador 2 venceu esta rodada!\n");
                jogadores[1].rodadaGanha++;
            } else {
                printf("Empate!\n");
            }

            rodada++;
        }

        // Determinar vencedor da mão
        int pontosGanhos = 1; // Base, pode aumentar com truco
        if (jogadores[0].rodadaGanha > jogadores[1].rodadaGanha) {
            jogadores[0].pontos += pontosGanhos;
            printf("Você venceu a mão! +%d ponto(s)\n", pontosGanhos);
        } else if (jogadores[1].rodadaGanha > jogadores[0].rodadaGanha) {
            jogadores[1].pontos += pontosGanhos;
            printf("Jogador 2 venceu a mão! +%d ponto(s)\n", pontosGanhos);
        } else {
            printf("Mão empatada!\n");
        }

        // Verificar fim de jogo
        if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) {
            if (jogadores[0].pontos >= 12) {
                printf("PARABÉNS! Você venceu o jogo!\n");
                sprintf(buffer, "FIM:0");
            } else {
                printf("Jogador 2 venceu o jogo!\n");
                sprintf(buffer, "FIM:1");
            }
            send(cliente, buffer, strlen(buffer), 0);
            break;
        }

        // Enviar placar atualizado
        sprintf(buffer, "PLACAR:%d:%d", jogadores[0].pontos, jogadores[1].pontos);
        send(cliente, buffer, strlen(buffer), 0);
        
        printf("Placar atual: J1=%d J2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
        printf("Pressione Enter para próxima mão...");
        getchar(); getchar(); // Limpar buffer
    }

    closesocket(cliente);
    closesocket(sock);
    WSACleanup();
    return 0;
}