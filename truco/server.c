#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "truco.h"

#pragma comment(lib, "ws2_32.lib")

SOCKET cliente_global;

void enviarInfoCompleta(SOCKET cliente, Carta cartaJogador1, int rodada, Jogador jogadores[2]) {
    char buffer[512];
    
    sprintf(buffer, "INFO_COMPLETA:Rodada%d:J1:%s:%s:Vira:%s:%s:Manilha:%s:Placar%d:%d:ValorRodada%d", 
            rodada, 
            cartaJogador1.nome, 
            convertedor_de_naipe(cartaJogador1.naipe),
            vira.nome,
            convertedor_de_naipe(vira.naipe),
            valor_para_nome(manilha_valor),
            jogadores[0].pontos, 
            jogadores[1].pontos,
            pontosRodada);
    send(cliente, buffer, strlen(buffer), 0);
}

void processarTrucoServidor(SOCKET cliente, int* pontosRodada, Jogador jogadores[2], int iniciador) {
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
    send(cliente, buffer, strlen(buffer), 0);
    printf("Truco pedido para %d pontos enviado para Jogador %d\n", proximoValor, responder + 1);
    
    while (1) {
        int bytesRecebidos = recv(cliente, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Cliente desconectou durante truco\n");
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
                
                // Enviar mensagem de corrida para o cliente
                sprintf(buffer, "TRUCO_CORREU_FIM:%d:%d", challenger, jogadores[challenger].pontos);
                send(cliente, buffer, strlen(buffer), 0);
                
                return; // Sai da função
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
                send(cliente, buffer, strlen(buffer), 0);
            }
        }
    }
}

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
    cliente_global = accept(sock, (struct sockaddr*)&client, &clientSize);
    printf("Jogador 2 conectado!\n");

    Jogador jogadores[2];
    srand((unsigned)time(NULL));
    iniciandoBaralho();
    resetarJogador(jogadores);

    char buffer[512];

    while (1) {
        reiniciarRodada(jogadores);
        pontosRodada = 1;
        
        sprintf(buffer, "NOVA_MAO:Vira:%s:%s:Manilha:%s", 
                vira.nome, 
                convertedor_de_naipe(vira.naipe),
                valor_para_nome(manilha_valor));
        send(cliente_global, buffer, strlen(buffer), 0);

        system("cls");
        printf("\n=== NOVA MAO ===\n");
        printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));

        int rodada = 1;
        while (rodada <= 3 && jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
            system("cls");
            printf("\n=== RODADA %d ===\n", rodada);
            printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
            printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
            printf("Valor da rodada: %d\n", pontosRodada);
            printf("Rodadas ganhas: J1=%d J2=%d\n\n", jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);

            mostrar_mao(&jogadores[0], 0);
            
            int opcao;
            printf("\nJogador 1: 1-Jogar carta  2-Pedir Truco\n");
            scanf("%d", &opcao);
            getchar();

            if (opcao == 2) {
                processarTrucoServidor(cliente_global, &pontosRodada, jogadores, 0);
                if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) {
                    break;
                }
                
                // Se alguém correu do truco, finalizar a rodada
                if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) {
                    printf("Mao finalizada devido a corrida. Pressione Enter para nova mao...\n");
                    getchar();
                    break;
                }
                
                // Atualizar tela após truco (se não houve corrida)
                system("cls");
                printf("\n=== RODADA %d ===\n", rodada);
                printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
                printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
                printf("Valor da rodada: %d\n", pontosRodada);
                printf("Rodadas ganhas: J1=%d J2=%d\n\n", jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);
                mostrar_mao(&jogadores[0], 0);
            }

            int carta1 = escolherCarta(&jogadores[0], 0);
            Carta c1 = jogadores[0].mao.cartas[carta1];
            
            enviarInfoCompleta(cliente_global, c1, rodada, jogadores);

            sprintf(buffer, "CARTA:%d:%d:%d", c1.valor, c1.naipe, carta1 + 1);
            send(cliente_global, buffer, strlen(buffer), 0);

            printf("\nVoce jogou: %s de %s\n", c1.nome, convertedor_de_naipe(c1.naipe));
            printf("Aguardando Jogador 2...\n");

            sprintf(buffer, "SUA_VEZ");
            send(cliente_global, buffer, strlen(buffer), 0);
            
            int bytesRecebidos;
            int processado = 0;
            int corridaOcorreu = 0;
            
            while (!processado) {
                bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesRecebidos <= 0) {
                    printf("Cliente desconectou\n");
                    break;
                }
                
                buffer[bytesRecebidos] = '\0';

                if (strstr(buffer, "TRUCO_PEDIDO:") == buffer) {
                    int valorTruco, challenger;
                    sscanf(buffer, "TRUCO_PEDIDO:%d:%d", &valorTruco, &challenger);
                    
                    printf("\nJogador %d pediu Truco para %d pontos! Responda: [1] Aceitar  [2] Aumentar  [3] Correr\n", 
                           challenger + 1, valorTruco);
                    
                    int resp;
                    scanf("%d", &resp);
                    getchar();
                    
                    sprintf(buffer, "TRUCO_RESPOSTA:%d", resp);
                    send(cliente_global, buffer, strlen(buffer), 0);
                    
                    if (resp == 1) {
                        pontosRodada = valorTruco;
                        printf("Truco aceito! Valor agora: %d\n", pontosRodada);
                    } else if (resp == 3) {
                        jogadores[challenger].pontos += pontosRodada;
                        printf("Voce correu! Jogador %d ganha %d ponto(s)\n", challenger + 1, pontosRodada);
                        
                        // Enviar mensagem de corrida para o cliente
                        sprintf(buffer, "TRUCO_CORREU_FIM:%d:%d", challenger, jogadores[challenger].pontos);
                        send(cliente_global, buffer, strlen(buffer), 0);
                        
                        corridaOcorreu = 1;
                        processado = 1;
                    }
                }
                else if (strstr(buffer, "TRUCO_CORREU_FIM:") == buffer) {
                    int jogador, pontos;
                    sscanf(buffer, "TRUCO_CORREU_FIM:%d:%d", &jogador, &pontos);
                    jogadores[jogador].pontos = pontos;
                    printf("Jogador %d correu do truco! Placar atualizado.\n", jogador + 1);
                    
                    corridaOcorreu = 1;
                    processado = 1;
                }
                else if (strstr(buffer, "CARTA:") == buffer) {
                    int carta2, valor2, naipe2;
                    if (sscanf(buffer, "CARTA:%d:%d:%d", &valor2, &naipe2, &carta2) == 3) {
                        Carta c2;
                        c2.valor = valor2;
                        c2.naipe = (Naipe)naipe2;
                        c2.nome = valor_para_nome(valor2);
                        
                        if (carta2 >= 1 && carta2 <= MAX_CARTAS_MAO) {
                            jogadores[1].mao.cartas[carta2 - 1].ativo = false;
                        }

                        printf("Jogador 2 jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));

                        int resultado = compararCartas(c1, c2);
                        
                        if (resultado > 0) {
                            jogadores[0].rodadaGanha++;
                            printf("\n>>> Voce venceu esta rodada! <<<\n");
                        } else if (resultado < 0) {
                            jogadores[1].rodadaGanha++;
                            printf("\n>>> Jogador 2 venceu esta rodada! <<<\n");
                        } else {
                            printf("\n>>> Empate! <<<\n");
                        }
                        
                        sprintf(buffer, "RESULTADO:%d:%d:%d", resultado, jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);
                        send(cliente_global, buffer, strlen(buffer), 0);
                        
                        processado = 1;
                    }
                }
            }

            // Se houve corrida, finalizar a rodada
            if (corridaOcorreu) {
                printf("Mao finalizada devido a corrida. Pressione Enter para nova mao...\n");
                getchar();
                break;
            }

            rodada++;

            if (rodada <= 3 && jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
                printf("\nPressione Enter para continuar para a proxima rodada...");
                getchar();
            }
        }

        if (jogadores[0].rodadaGanha > jogadores[1].rodadaGanha) {
            jogadores[0].pontos += pontosRodada;
            printf("\n*** Voce venceu a mao! +%d ponto(s) ***\n", pontosRodada);
        } else if (jogadores[1].rodadaGanha > jogadores[0].rodadaGanha) {
            jogadores[1].pontos += pontosRodada;
            printf("\n*** Jogador 2 venceu a mao! +%d ponto(s) ***\n", pontosRodada);
        } else {
            printf("\n*** Mao empatada! ***\n");
        }

        if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) {
            if (jogadores[0].pontos >= 12) {
                printf("\n?? PARABENS! Voce venceu o jogo! ??\n");
                sprintf(buffer, "FIM:0");
            } else {
                printf("\n?? Jogador 2 venceu o jogo! ??\n");
                sprintf(buffer, "FIM:1");
            }
            send(cliente_global, buffer, strlen(buffer), 0);
            break;
        }

        sprintf(buffer, "PLACAR:%d:%d", jogadores[0].pontos, jogadores[1].pontos);
        send(cliente_global, buffer, strlen(buffer), 0);
        
        printf("\nPlacar atual: J1=%d J2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
        printf("\nPressione Enter para proxima mao...");
        getchar();
    }

    printf("\nFim do jogo. Pressione Enter para sair...");
    getchar();

    closesocket(cliente_global);
    closesocket(sock);
    WSACleanup();
    return 0;
}
