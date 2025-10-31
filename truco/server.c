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
    iniciandoBaralho();
    resetarJogador(jogadores);
    reiniciarRodada(jogadores);

    char buffer[512];
    int vez = 0;
    int carta1, carta2;

    while (1) {
        printf("\n=== NOVA RODADA ===\n");
        printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
        printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);

        mostrar_mao(&jogadores[0], 0);
        printf("Escolha sua carta (1-3): ");
        scanf("%d", &carta1);
        sprintf(buffer, "%d", carta1);
        send(cliente, buffer, strlen(buffer), 0);

        printf("Aguardando Jogador 2...\n");
        recv(cliente, buffer, sizeof(buffer), 0);
        carta2 = atoi(buffer);

        Carta c1 = jogadores[0].mao.cartas[carta1 - 1];
        Carta c2 = jogadores[1].mao.cartas[carta2 - 1];

        int resultado = compararCartas(c1, c2);
        if (resultado > 0) {
            printf("Você venceu esta rodada!\n");
            jogadores[0].pontos += 1;
        } else if (resultado < 0) {
            printf("Jogador 2 venceu esta rodada!\n");
            jogadores[1].pontos += 1;
        } else {
            printf("Empate!\n");
        }

        sprintf(buffer, "Placar: J1=%d J2=%d", jogadores[0].pontos, jogadores[1].pontos);
        send(cliente, buffer, strlen(buffer), 0);

        printf("%s\n", buffer);
    }

    closesocket(cliente);
    closesocket(sock);
    WSACleanup();
    return 0;
}

