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


void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


int validarEscolhaCartaCliente(Jogador* jogador) {
    int escolha;
    while (1) {
        printf("Escolha uma carta (1-3): ");
        if (scanf("%d", &escolha) == 1) {
            limpar_buffer();
            
            if (escolha >= 1 && escolha <= 3) {
                if (jogador->mao.cartas[escolha-1].ativo) {
                    return escolha - 1;
                } else {
                    printf("Carta ja foi jogada! Escolha outra.\n");
                }
            } else {
                printf("Escolha invalida! Digite 1, 2 ou 3.\n");
            }
        } else {
            printf("Entrada invalida! Digite um numero.\n");
            limpar_buffer();
        }
    }
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


int esperarRespostaTruco(SOCKET sock, int* pontosRodada, Jogador jogadores[2]) {
    char buffer[1024];
    while (1) {
        int bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Servidor desconectou durante o Truco.\n");
            return 1; 
        }
        buffer[bytesRecebidos] = '\0';

        if (strstr(buffer, "TRUCO_ACEITO:") == buffer) {
            int valor;
            sscanf(buffer, "TRUCO_ACEITO:%d", &valor);
            *pontosRodada = valor;
            printf("Truco aceito! Valor agora: %d\n", *pontosRodada);
            return 0; 
        } 
        else if (strstr(buffer, "TRUCO_AUMENTOU:") == buffer) {
            int valorTruco, challenger;
            sscanf(buffer, "TRUCO_AUMENTOU:%d:%d", &valorTruco, &challenger);

            printf("\nJogador %d aumentou para %d pontos! Responda: [1] Aceitar", challenger + 1, valorTruco);
            if (valorTruco < 12) {
                printf(" [2] Aumentar");
            }
            printf(" [3] Correr\n");

            int resp;
            if (scanf("%d", &resp) != 1) { 
                limpar_buffer(); 
                resp = 3; 
            }
            limpar_buffer();

            if (resp == 2 && valorTruco == 12) {
                resp = 1;
            }

            sprintf(buffer, "TRUCO_RESPOSTA:%d", resp);
            send(sock, buffer, strlen(buffer), 0);
            
            if (resp == 1) {
                *pontosRodada = valorTruco;
                printf("Truco aceito! Valor agora: %d\n", *pontosRodada);
               
            } else if (resp == 3) {
                printf("Voce correu do truco. Aguardando fim da mao...\n");
                
            }
            
        }
        else if (strstr(buffer, "TRUCO_CORREU_FIM:") == buffer) {
            int jogador, pontos;
            sscanf(buffer, "TRUCO_CORREU_FIM:%d:%d", &jogador, &pontos);
            jogadores[jogador].pontos = pontos;
            printf("Jogador %d correu do truco! Placar atualizado.\n", jogador + 1);
            printf("Mao finalizada devido a corrida. Aguardando nova mao...\n");
            return 1; 
        } else {
              
             printf("Resposta inesperada durante truco: %s. Retomando fluxo.\n", buffer);
             return 0;
        }
    }
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar Winsock\n");
        printf("Pressione Enter para sair...");
        limpar_buffer();
        getchar();
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar socket\n");
        WSACleanup();
        printf("Pressione Enter para sair...");
        limpar_buffer();
        getchar();
        return 1;
    }

    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor.sin_port = htons(51171);

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        printf("Erro ao conectar com o servidor. Certifique-se que o 'server.exe' esta rodando.\n");
        printf("Codigo de erro Winsock: %d\n", WSAGetLastError());
        printf("Pressione Enter para sair...");
        limpar_buffer();
        getchar();
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado ao servidor (Jogador 2)\n\n");

    Jogador jogadores[2];
    srand((unsigned)time(NULL));
    iniciandoBaralho(); 

    extern int pontosRodada; 

    char buffer[1024];
    char viraNome[20] = "";
    char viraNaipe[20] = "";
    char manilhaNome[20] = "";

    while (1) {
        int bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Servidor desconectou\n");
            break;
        }
        buffer[bytesRecebidos] = '\0';

        if (strstr(buffer, "NOVA_MAO:") == buffer) {
            sscanf(buffer, "NOVA_MAO:Vira:%[^:]:%[^:]:Manilha:%s", viraNome, viraNaipe, manilhaNome);

            bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRecebidos <= 0 || strstr(buffer, "MAO:") != buffer) {
                printf("Erro ao receber as cartas do servidor ou formato invalido.\n");
                break;
            }
            buffer[bytesRecebidos] = '\0';

            int v1, n1, v2, n2, v3, n3;
            if (sscanf(buffer, "MAO:%d:%d:%d:%d:%d:%d", &v1, &n1, &v2, &n2, &v3, &n3) == 6) {
                int valores[] = {v1, v2, v3};
                int naipes[] = {n1, n2, n3};
                for (int i = 0; i < MAX_CARTAS_MAO; i++) {
                    jogadores[1].mao.cartas[i].valor = valores[i];
                    jogadores[1].mao.cartas[i].naipe = (Naipe)naipes[i];
                    strcpy(jogadores[1].mao.cartas[i].nome, valor_para_nome(valores[i])); 
                    jogadores[1].mao.cartas[i].ativo = true;
                }
            } else {
                printf("Erro ao parsear as cartas do servidor: %s\n", buffer);
            }

            limparTela();
            printf("=== NOVA MAO INICIADA ===\n");
            printf("Vira: %s de %s\n", viraNome, viraNaipe);
            printf("Manilha: %s\n", manilhaNome);
            printf("Placar: J1=%d | J2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
            mostrar_mao(&jogadores[1], 1);

            send(sock, "PRONTO", 6, 0);
            jogadores[0].rodadaGanha = 0;
            jogadores[1].rodadaGanha = 0;
            
        }
        else if (strstr(buffer, "INFO_COMPLETA:") == buffer) {
            char cartaJ1Nome[20], cartaJ1Naipe[20];
            int pontos1, pontos2, valorRodada, rodada;
            sscanf(buffer, "INFO_COMPLETA:Rodada%d:J1:%[^:]:%[^:]:Vira:%[^:]:%[^:]:Manilha:%[^:]:Placar%d:%d:ValorRodada%d",
                   &rodada, cartaJ1Nome, cartaJ1Naipe, viraNome, viraNaipe, manilhaNome, &pontos1, &pontos2, &valorRodada);

            pontosRodada = valorRodada;
            jogadores[0].pontos = pontos1; 
            jogadores[1].pontos = pontos2; 
            
            mostrarInfoRodadaCompleta(viraNome, viraNaipe, manilhaNome, rodada, pontos1, pontos2, pontosRodada, cartaJ1Nome, cartaJ1Naipe);

            mostrar_mao(&jogadores[1], 1); 
        }
        else if (strstr(buffer, "SUA_VEZ_PRIMEIRO") != NULL || strstr(buffer, "CARTA_J1:") == buffer) {
            if (strstr(buffer, "SUA_VEZ_PRIMEIRO") != NULL) {
                 printf("Voce comeca esta rodada!\n");
            } else {
                 printf("\nSua vez de jogar!\n");
            }
           
            mostrar_mao(&jogadores[1], 1); 

            int opcao;
            printf("\nJogador 2: 1-Jogar carta  2-Pedir Truco\n");
            if (scanf("%d", &opcao) != 1) { 
                limpar_buffer(); 
                opcao = 1; 
            }
            limpar_buffer();

            if (opcao == 2) {
                int valores[] = {1, 3, 6, 9, 12};
                int maxIdx = 4;
                int currentIdx = 0;

                for (int i = 0; i <= maxIdx; i++) {
                    if (valores[i] == pontosRodada) {
                        currentIdx = i;
                        break;
                    }
                }

                if (currentIdx < maxIdx) {
                    int proximoValor = valores[currentIdx + 1];
                    sprintf(buffer, "TRUCO_PEDIDO:%d:%d", proximoValor, 1); 
                    send(sock, buffer, strlen(buffer), 0);
                    printf("Truco pedido para %d pontos enviado. Aguardando resposta...\n", proximoValor);
                    
                    int maoAcabou = esperarRespostaTruco(sock, &pontosRodada, jogadores);
                    
                    if (maoAcabou) {
                         continue; 
                    }
                     
                    printf("\nSua vez de jogar (apos truco)!\n");
                    mostrar_mao(&jogadores[1], 1);
                    
                } else {
                    printf("Truco ja esta no maximo. Jogar carta.\n");
                }
            }

            
            int carta2 = validarEscolhaCartaCliente(&jogadores[1]);

            Carta c2 = jogadores[1].mao.cartas[carta2];
            jogadores[1].mao.cartas[carta2].ativo = false;

            sprintf(buffer, "CARTA_J2:%d:%d", c2.valor, c2.naipe);
            send(sock, buffer, strlen(buffer), 0);

            printf("\nVoce jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));
            
            if (strstr(buffer, "SUA_VEZ_PRIMEIRO") == NULL) {
                printf("Aguardando Jogador 1...\n");
            }
        }
        else if (strstr(buffer, "AGUARDE_JOGADA_J1") == buffer) {
            printf("Aguardando Jogador 1 jogar...\n");
        }
        else if (strstr(buffer, "TRUCO_PEDIDO:") == buffer) {
            int valorTruco, challenger;
            sscanf(buffer, "TRUCO_PEDIDO:%d:%d", &valorTruco, &challenger);

            printf("\nJogador %d pediu Truco para %d pontos! Responda: [1] Aceitar", challenger + 1, valorTruco);
            if (valorTruco < 12) {
                printf(" [2] Aumentar");
            }
            printf(" [3] Correr\n");

            int resp;
            if (scanf("%d", &resp) != 1) { 
                limpar_buffer(); 
                resp = 3; 
            }
            limpar_buffer();

            if (resp == 2 && valorTruco == 12) {
                resp = 1;
            }

            sprintf(buffer, "TRUCO_RESPOSTA:%d", resp);
            send(sock, buffer, strlen(buffer), 0);

            if (resp == 1) {
                pontosRodada = valorTruco;
                printf("Truco aceito! Valor agora: %d\n", pontosRodada);
            } else if (resp == 3) {
                printf("Voce correu do truco. Aguardando fim da mao...\n");
            } else if (resp == 2) {
                printf("Voce aumentou o truco. Aguardando resposta do Jogador 1...\n");
                esperarRespostaTruco(sock, &pontosRodada, jogadores);
            }
        }
        else if (strstr(buffer, "TRUCO_ACEITO:") == buffer) {
            int valor;
            sscanf(buffer, "TRUCO_ACEITO:%d", &valor);
            pontosRodada = valor;
            printf("Truco aceito! Valor agora: %d\n", pontosRodada);
        }
        else if (strstr(buffer, "TRUCO_AUMENTOU:") == buffer) {
            int valorTruco, challenger;
            sscanf(buffer, "TRUCO_AUMENTOU:%d:%d", &valorTruco, &challenger);

            printf("\nJogador %d aumentou para %d pontos! Responda: [1] Aceitar", challenger + 1, valorTruco);
            if (valorTruco < 12) {
                printf(" [2] Aumentar");
            }
            printf(" [3] Correr\n");

            int resp;
            if (scanf("%d", &resp) != 1) { 
                limpar_buffer(); 
                resp = 3; 
            }
            limpar_buffer();

            if (resp == 2 && valorTruco == 12) {
                resp = 1;
            }

            sprintf(buffer, "TRUCO_RESPOSTA:%d", resp);
            send(sock, buffer, strlen(buffer), 0);
            
            if (resp == 1) {
                pontosRodada = valorTruco;
                printf("Truco aceito! Valor agora: %d\n", pontosRodada);
            } else if (resp == 3) {
                printf("Voce correu do truco. Aguardando fim da mao...\n");
            }
             else if (resp == 2) {
                printf("Voce aumentou o truco. Aguardando resposta do Jogador 1...\n");
                esperarRespostaTruco(sock, &pontosRodada, jogadores);
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
            int resultado, rodadasJ1, rodadasJ2, proximoIniciador;
            sscanf(buffer, "RESULTADO:%d:%d:%d:%d", &resultado, &rodadasJ1, &rodadasJ2, &proximoIniciador);

            printf("\n=== RESULTADO DA RODADA ===\n");
            if (resultado > 0) {
                printf(">>> Jogador 1 venceu a rodada! <<<\n");
            } else if (resultado < 0) {
                printf(">>> Voce venceu a rodada! <<<\n");
            } else {
                printf(">>> Empate! <<<\n");
            }
            printf("Rodadas ganhas: J1=%d | J2=%d\n", rodadasJ1, rodadasJ2);
            printf("Proxima rodada inicia com: %s\n\n", proximoIniciador == 0 ? "Jogador 1" : "Voce");
            
            if (rodadasJ1 < 2 && rodadasJ2 < 2) {
                bytesRecebidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
                if (bytesRecebidos <= 0) { printf("Servidor desconectou\n"); break; }
                buffer[bytesRecebidos] = '\0';
                
                if (strcmp(buffer, "PAUSA_CLIENTE") == 0) {
                     printf("\nPressione Enter para continuar para a proxima rodada...");
                     limpar_buffer();
                     getchar();
                     send(sock, "OK_PAUSA", 8, 0);
                }
            }
        }
        else if (strstr(buffer, "FIM:") == buffer) {
            int vencedor;
            sscanf(buffer, "FIM:%d", &vencedor);
            limparTela();
            if (vencedor == 1) {
                printf("PARABENS! Voce venceu o jogo!\n");
            } else {
                printf("Jogador 1 venceu o jogo!\n");
            }
            printf("Aguardando decisao do servidor sobre jogar novamente...\n");
        }
        else if (strstr(buffer, "JOGAR_NOVAMENTE") == buffer) {
            printf("Servidor escolheu jogar novamente. Iniciando novo jogo...\n");
            
          
            jogadores[0].rodadaGanha = 0;
            jogadores[1].rodadaGanha = 0;
        }
        else if (strstr(buffer, "FINALIZAR") == buffer) {
            printf("Servidor encerrou a partida. Fechando conexao.\n");
            break; 
        }
        else if (strstr(buffer, "PLACAR:") == buffer) {
            int pontos1, pontos2, proximoIniciador;
            sscanf(buffer, "PLACAR:%d:%d:%d", &pontos1, &pontos2, &proximoIniciador);
            printf("Placar atualizado: J1=%d J2=%d\n", pontos1, pontos2);
            jogadores[0].pontos = pontos1;
            jogadores[1].pontos = pontos2;
            printf("Proxima mao inicia com: %s\n", proximoIniciador == 0 ? "Jogador 1" : "Voce");
            
            
            printf("Aguardando novas cartas para a proxima mao...\n");
        } else {
            printf("Mensagem desconhecida: %s\n", buffer);
        }
    }

    printf("Conexao encerrada. Pressione Enter para sair...");
    limpar_buffer();
    getchar();

    closesocket(sock);
    WSACleanup();
    return 0;
}