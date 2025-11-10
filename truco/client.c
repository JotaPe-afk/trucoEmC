#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "truco.h"

#pragma comment(lib, "ws2_32.lib")

void limparTela() {
    system("cls");
}

void mostrarInfoRodadaCompleta(char* viraNome, char* viraNaipe, char* manilhaNome, int rodada, 
                              int pontosJ1, int pontosJ2, int valorRodada, char* cartaJ1Nome, char* cartaJ1Naipe) {
    limparTela();
    printf("=========================================\n");
    printf("             RODADA %d\n", rodada);
    printf("=========================================\n");
    printf("Vira: %s de %s\n", viraNome, viraNaipe);
    printf("Manilha: %s\n", manilhaNome);
    printf("Placar: J1=%d | J2=%d\n", pontosJ1, pontosJ2);
    printf("Valor da rodada: %d\n", valorRodada);
    printf("=========================================\n");
    printf("Jogador 1 jogou: %s de %s\n\n", cartaJ1Nome, cartaJ1Naipe);
}

void processarTrucoCliente(SOCKET sock, int* pontosRodada, Jogador jogadores[2], int iniciador) {
    char buffer[512];
    int valores[] = {1, 3, 6, 9, 12};
    int idx = 0;
    
    for (int i = 0; i < 5; i++) {
        if (valores[i] == *pontosRodada) {
            idx = i;
            break;
        }
    }
    
    if (idx >= 4) {
        printf("Truco já está no máximo (%d)\n", valores[idx]);
        return;
    }
    
    int proximoValor = valores[idx + 1];
    int challenger = iniciador;
    int responder = 1 - challenger;
    
    sprintf(buffer, "TRUCO_PEDIDO:%d:%d", proximoValor, challenger);
    send(sock, buffer, strlen(buffer), 0);
    printf("Truco pedido para %d pontos enviado para Jogador %d\n", proximoValor, responder + 1);
    
    while (1) {
        int bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Servidor desconectou durante truco\n");
            return;
        }
        buffer[bytesRecebidos] = '\0';
        
        if (strstr(buffer, "TRUCO_RESPOSTA:") == buffer) {
            int resposta;
            sscanf(buffer, "TRUCO_RESPOSTA:%d", &resposta);
            
            if (resposta == 1) {
                *pontosRodada = proximoValor;
                printf("Jogador %d aceitou o truco! Valor agora: %d\n", responder + 1, *pontosRodada);
                break;
            } else if (resposta == 3) {
                jogadores[challenger].pontos += *pontosRodada;
                printf("Jogador %d correu! Jogador %d ganha %d ponto(s)\n", 
                       responder + 1, challenger + 1, *pontosRodada);
                return;
            } else if (resposta == 2) {
                idx++;
                if (idx >= 4) {
                    printf("Truco já está no máximo\n");
                    *pontosRodada = valores[4];
                    break;
                }
                proximoValor = valores[idx + 1];
                
                int temp = challenger;
                challenger = responder;
                responder = temp;
                
                printf("Jogador %d aumentou para %d pontos!\n", responder + 1, proximoValor);
                
                sprintf(buffer, "TRUCO_AUMENTOU:%d:%d", proximoValor, challenger);
                send(sock, buffer, strlen(buffer), 0);
            }
        }
        else if (strstr(buffer, "TRUCO_CORREU_FIM:") == buffer) {
            int jogador, pontos;
            sscanf(buffer, "TRUCO_CORREU_FIM:%d:%d", &jogador, &pontos);
            jogadores[jogador].pontos = pontos;
            printf("Jogador %d correu do truco! Placar atualizado.\n", jogador + 1);
            return;
        }
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor.sin_port = htons(51171);

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        printf("Erro ao conectar com o servidor\n");
        return 1;
    }
    
    printf("Conectado ao servidor (Jogador 2)\n\n");

    Jogador jogadores[2];
    srand((unsigned)time(NULL));
    iniciandoBaralho();
    resetarJogador(jogadores);

    char buffer[512];
    char viraNome[10] = "";
    char viraNaipe[10] = "";
    char manilhaNome[10] = "";
    int rodada = 1;

    while (1) {
        int bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Servidor desconectou\n");
            break;
        }
        buffer[bytesRecebidos] = '\0';
        
        printf("Mensagem recebida: %s\n", buffer);
        
        if (strstr(buffer, "NOVA_MAO:") == buffer) {
            char temp[100];
            sscanf(buffer, "NOVA_MAO:Vira:%[^:]:%[^:]:Manilha:%s", temp, viraNaipe, manilhaNome);
            
            char *token = strtok(temp, ":");
            if (token) strcpy(viraNome, token);
            
            limparTela();
            printf("=== NOVA MAO INICIADA ===\n");
            printf("Vira: %s de %s\n", viraNome, viraNaipe);
            printf("Manilha: %s\n\n", manilhaNome);
            
            reiniciarRodada(jogadores);
            rodada = 1;
        }
        else if (strstr(buffer, "INFO_COMPLETA:") == buffer) {
            char cartaJ1Nome[10], cartaJ1Naipe[10];
            int pontos1, pontos2, valorRodada;
            sscanf(buffer, "INFO_COMPLETA:Rodada%d:J1:%[^:]:%[^:]:Vira:%[^:]:%[^:]:Manilha:%[^:]:Placar%d:%d:ValorRodada%d", 
                   &rodada, cartaJ1Nome, cartaJ1Naipe, viraNome, viraNaipe, manilhaNome, &pontos1, &pontos2, &valorRodada);
            
            pontosRodada = valorRodada;
            mostrarInfoRodadaCompleta(viraNome, viraNaipe, manilhaNome, rodada, pontos1, pontos2, pontosRodada, cartaJ1Nome, cartaJ1Naipe);
        }
        else if (strstr(buffer, "SUA_VEZ") != NULL) {
            printf("Mensagem SUA_VEZ recebida: %s\n", buffer);
            
            // Verificar se houve corrida antes de jogar
            if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) {
                printf("Mao finalizada devido a corrida. Aguardando nova mao...\n");
                continue;
            }
            
            printf("Sua vez de jogar!\n");
            mostrar_mao(&jogadores[1], 1);
            
            int opcao;
            printf("\nJogador 2: 1-Jogar carta  2-Pedir Truco\n");
            scanf("%d", &opcao);
            getchar();

            if (opcao == 2) {
                processarTrucoCliente(sock, &pontosRodada, jogadores, 1);
                if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) {
                    break;
                }
                
                // Se alguém correu do truco, não continuar jogando
                if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) {
                    printf("Mao finalizada devido a corrida. Aguardando nova mao...\n");
                    continue;
                }
                
                printf("\nSua vez de jogar (apos truco)!\n");
                mostrar_mao(&jogadores[1], 1);
            }

            int carta2 = escolherCarta(&jogadores[1], 1);
            Carta c2 = jogadores[1].mao.cartas[carta2];
            
            sprintf(buffer, "CARTA:%d:%d:%d", c2.valor, c2.naipe, carta2 + 1);
            send(sock, buffer, strlen(buffer), 0);

            printf("\nVoce jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));
            printf("Aguardando resultado...\n");
        }
        else if (strstr(buffer, "TRUCO_PEDIDO:") == buffer) {
            int valorTruco, challenger;
            sscanf(buffer, "TRUCO_PEDIDO:%d:%d", &valorTruco, &challenger);
            
            printf("\nJogador %d pediu Truco para %d pontos! Responda: [1] Aceitar  [2] Aumentar  [3] Correr\n", 
                   challenger + 1, valorTruco);
            
            int resp;
            scanf("%d", &resp);
            getchar();
            
            sprintf(buffer, "TRUCO_RESPOSTA:%d", resp);
            send(sock, buffer, strlen(buffer), 0);
            
            if (resp == 1) {
                pontosRodada = valorTruco;
                printf("Truco aceito! Valor agora: %d\n", pontosRodada);
            } else if (resp == 3) {
                jogadores[challenger].pontos += pontosRodada;
                printf("Voce correu! Jogador %d ganha %d ponto(s)\n", challenger + 1, pontosRodada);
            }
        }
        else if (strstr(buffer, "TRUCO_CORREU_FIM:") == buffer) {
            int jogador, pontos;
            sscanf(buffer, "TRUCO_CORREU_FIM:%d:%d", &jogador, &pontos);
            jogadores[jogador].pontos = pontos;
            printf("Jogador %d correu do truco! Placar atualizado.\n", jogador + 1);
            printf("Mao finalizada devido a corrida. Aguardando nova mao...\n");
        }
        else if (strstr(buffer, "RESULTADO:") == buffer) {
            int resultado, rodadasJ1, rodadasJ2;
            sscanf(buffer, "RESULTADO:%d:%d:%d", &resultado, &rodadasJ1, &rodadasJ2);
            
            printf("\n=== RESULTADO DA RODADA ===\n");
            if (resultado > 0) {
                printf(">>> Jogador 1 venceu a rodada! <<<\n");
            } else if (resultado < 0) {
                printf(">>> Voce venceu a rodada! <<<\n");
            } else {
                printf(">>> Empate! <<<\n");
            }
            printf("Rodadas ganhas: J1=%d | J2=%d\n\n", rodadasJ1, rodadasJ2);
        }
        else if (strstr(buffer, "FIM:") == buffer) {
            int vencedor;
            sscanf(buffer, "FIM:%d", &vencedor);
            limparTela();
            if (vencedor == 1) {
                printf("?? PARABENS! Voce venceu o jogo! ??\n");
            } else {
                printf("?? Jogador 1 venceu o jogo! ??\n");
            }
            closesocket(sock);
            WSACleanup();
            return 0;
        }
        else if (strstr(buffer, "PLACAR:") == buffer) {
            int pontos1, pontos2;
            sscanf(buffer, "PLACAR:%d:%d", &pontos1, &pontos2);
            printf("Placar atualizado: J1=%d J2=%d\n", pontos1, pontos2);
            jogadores[0].pontos = pontos1;
            jogadores[1].pontos = pontos2;
        }
    }

    printf("Conexao encerrada. Pressione Enter para sair...");
    getchar();

    closesocket(sock);
    WSACleanup();
    return 0;
}
