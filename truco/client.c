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
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP do servidor
    servidor.sin_port = htons(51171);

    connect(sock, (struct sockaddr*)&servidor, sizeof(servidor));
    printf("Conectado ao servidor (Jogador 1)\n");

    Jogador jogadores[2];
    iniciandoBaralho();
    resetarJogador(jogadores);
    reiniciarRodada(jogadores);

    char buffer[512];
    int cartaServidor, cartaCliente;

    while (1) {
        recv(sock, buffer, sizeof(buffer), 0);
        cartaServidor = atoi(buffer);
        printf("Jogador 1 jogou a carta %d.\n", cartaServidor);

        mostrar_mao(&jogadores[1], 1);
        printf("Escolha sua carta (1-3): ");
        scanf("%d", &cartaCliente);
        sprintf(buffer, "%d", cartaCliente);
        send(sock, buffer, strlen(buffer), 0);

        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s\n", buffer);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

