#include <stdio.h>
#include <winsock2.h>
#include "truco.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor.sin_port = htons(51171);

    connect(sock, (struct sockaddr*)&servidor, sizeof(servidor));
    printf("Conectado ao servidor (Jogador 2)\n");

    Jogador jogadores[2];
    srand((unsigned)time(NULL));
    iniciandoBaralho();
    resetarJogador(jogadores);

    char buffer[512];

    while (1) {
        // Receber vira do servidor
        recv(sock, buffer, sizeof(buffer), 0);
        if (strstr(buffer, "VIRA:") == buffer) {
            int valor_vira, naipe_vira;
            sscanf(buffer, "VIRA:%d:%d", &valor_vira, &naipe_vira);
            vira.valor = valor_vira;
            vira.naipe = (Naipe)naipe_vira;
            vira.nome = valor_para_nome(valor_vira);
            manilha_valor = (vira.valor % 10) + 1;
            
            printf("\n=== NOVA MÃO ===\n");
            printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
        }

        // Distribuir cartas para o jogador 2
        reiniciarRodada(jogadores);

        int rodada = 1;
        while (rodada <= 3) {
            // Receber carta do servidor
            recv(sock, buffer, sizeof(buffer), 0);
            
            if (strstr(buffer, "FIM:") == buffer) {
                int vencedor;
                sscanf(buffer, "FIM:%d", &vencedor);
                if (vencedor == 1) {
                    printf("PARABÉNS! Você venceu o jogo!\n");
                } else {
                    printf("Jogador 1 venceu o jogo!\n");
                }
                closesocket(sock);
                WSACleanup();
                return 0;
            }
            
            if (strstr(buffer, "PLACAR:") == buffer) {
                int pontos1, pontos2;
                sscanf(buffer, "PLACAR:%d:%d", &pontos1, &pontos2);
                printf("Placar atualizado: J1=%d J2=%d\n", pontos1, pontos2);
                break;
            }

            int carta1, valor1, naipe1;
            sscanf(buffer, "CARTA:%d:%d:%d", &valor1, &naipe1, &carta1);
            printf("Jogador 1 jogou a carta %d.\n", carta1);

            // Jogador 2 joga
            mostrar_mao(&jogadores[1], 1);
            printf("Escolha sua carta (1-3): ");
            int carta2;
            scanf("%d", &carta2);
            
            Carta c2 = jogadores[1].mao.cartas[carta2 - 1];
            jogadores[1].mao.cartas[carta2 - 1].ativo = false;
            
            // Enviar carta para servidor
            sprintf(buffer, "CARTA:%d:%d:%d", c2.valor, c2.naipe, carta2);
            send(sock, buffer, strlen(buffer), 0);

            rodada++;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}