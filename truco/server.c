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
        printf("Truco ja esta no maximo (%d)\n", valores[idx]);
        // Garante que o cliente seja notificado do estado final
        sprintf(buffer, "TRUCO_ACEITO:%d", valores[4]);
        send(cliente, buffer, strlen(buffer), 0);
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

                sprintf(buffer, "TRUCO_ACEITO:%d", *pontosRodada);
                send(cliente, buffer, strlen(buffer), 0);
                break;
            } else if (resposta == 3) {
                jogadores[challenger].pontos += *pontosRodada;
                printf("Jogador %d correu! Jogador %d ganha %d ponto(s)\n",
                       responder + 1, challenger + 1, *pontosRodada);

                sprintf(buffer, "TRUCO_CORREU_FIM:%d:%d", challenger, jogadores[challenger].pontos);
                send(cliente, buffer, strlen(buffer), 0);
                return;
            } else if (resposta == 2) {
                idx++;
                if (idx >= 4) {
                    printf("Truco ja esta no maximo\n");
                    *pontosRodada = valores[4];
                    // Se o cliente aumentar até o limite, o servidor aceita implicitamente e notifica
                    sprintf(buffer, "TRUCO_ACEITO:%d", *pontosRodada);
                    send(cliente, buffer, strlen(buffer), 0);
                    break;
                }

                proximoValor = valores[idx + 1];

                int temp = challenger;
                challenger = responder;
                responder = temp;

                // Aqui o Servidor (J1) deve responder ao aumento do Cliente (J2)
                printf("Jogador 2 aumentou para %d pontos!\n", proximoValor);

                int respJ1;
                printf("Jogador 1: [1] Aceitar [2] Aumentar [3] Correr\n");
                scanf("%d", &respJ1);
                getchar();

                if (respJ1 == 1) { // Aceitar
                    *pontosRodada = proximoValor;
                    printf("Jogador 1 aceitou o truco! Valor agora: %d\n", *pontosRodada);
                    sprintf(buffer, "TRUCO_ACEITO:%d", *pontosRodada);
                    send(cliente, buffer, strlen(buffer), 0);
                    break;
                } else if (respJ1 == 3) { // Correr
                    jogadores[challenger].pontos += proximoValor; // Challenger (J2) ganha
                    printf("Jogador 1 correu! Jogador 2 ganha %d ponto(s)\n", proximoValor);
                    sprintf(buffer, "TRUCO_CORREU_FIM:%d:%d", challenger, jogadores[challenger].pontos);
                    send(cliente, buffer, strlen(buffer), 0);
                    return;
                } else if (respJ1 == 2) { // Aumentar
                    idx++;
                    proximoValor = valores[idx + 1];
                    // Troca de papéis novamente
                    int temp2 = challenger;
                    challenger = responder;
                    responder = temp2;

                    printf("Jogador 1 aumentou para %d pontos!\n", proximoValor);
                    sprintf(buffer, "TRUCO_AUMENTOU:%d:%d", proximoValor, challenger);
                    send(cliente, buffer, strlen(buffer), 0);
                    // Continua o loop esperando a resposta do J2
                }
            }
        }
    }
}

int determinarIniciadorProximaRodada(Jogador jogadores[2], int iniciadorAtual, int resultadoRodada) {
    if (resultadoRodada > 0) return 0; // Jogador 1 venceu, começa próximo
    if (resultadoRodada < 0) return 1; // Jogador 2 venceu, começa próximo
    return iniciadorAtual; // Empate, mantém o mesmo
}

int validarEscolhaCarta(Jogador* jogador) {
    int escolha;
    while (1) {
        printf("Jogador 1, escolha (1-3): ");
        if (scanf("%d", &escolha) == 1) {
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
            while (getchar() != '\n');
        }
    }
}

int main() {
    // ... (Inicialização do socket)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar Winsock\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(51171);

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro ao fazer bind\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (listen(sock, 1) == SOCKET_ERROR) {
        printf("Erro ao escutar\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Aguardando o Jogador 2 conectar...\n");
    int clientSize = sizeof(client);
    cliente_global = accept(sock, (struct sockaddr*)&client, &clientSize);
    if (cliente_global == INVALID_SOCKET) {
        printf("Erro ao aceitar conexao\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Jogador 2 conectado!\n");


    Jogador jogadores[2];
    srand((unsigned)time(NULL));

    char buffer[1024];
    int quemIniciaMao = 0;

    while (1) {
        iniciandoBaralho();
        resetarJogador(jogadores);
        reiniciarRodada(jogadores);
        distribuirMaoParaJogadores(jogadores); // <-- DISTRIBUIÇÃO CORRETA
        pontosRodada = 1;

        sprintf(buffer, "NOVA_MAO:Vira:%s:%s:Manilha:%s",
                vira.nome,
                convertedor_de_naipe(vira.naipe),
                valor_para_nome(manilha_valor));
        send(cliente_global, buffer, strlen(buffer), 0);

        // --- NOVO: Enviar as cartas do Jogador 2 para o cliente ---
        sprintf(buffer, "MAO:%d:%d:%d:%d:%d:%d",
                jogadores[1].mao.cartas[0].valor, jogadores[1].mao.cartas[0].naipe,
                jogadores[1].mao.cartas[1].valor, jogadores[1].mao.cartas[1].naipe,
                jogadores[1].mao.cartas[2].valor, jogadores[1].mao.cartas[2].naipe);
        send(cliente_global, buffer, strlen(buffer), 0);
        // -----------------------------------------------------------

        int bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecebidos <= 0) {
            printf("Cliente desconectou\n");
            break;
        }
        buffer[bytesRecebidos] = '\0';

        if (strcmp(buffer, "PRONTO") != 0) {
            printf("Erro de sincronizacao com cliente\n");
            break;
        }

        system("cls");
        printf("\n=== NOVA MAO ===\n");
        printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
        printf("Quem inicia: %s\n", quemIniciaMao == 0 ? "Jogador 1 (Voce)" : "Jogador 2");

        int rodada = 1;
        int iniciadorRodada = quemIniciaMao;

        while (rodada <= 3 && jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
            system("cls");
            printf("\n=== RODADA %d ===\n", rodada);
            printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
            printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
            printf("Valor da rodada: %d\n", pontosRodada);
            printf("Rodadas ganhas: J1=%d J2=%d\n\n", jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);

            Carta c1, c2;
            int resultado = 0;

            if (iniciadorRodada == 0) {
                // JOGADOR 1 (SERVIDOR) COMEÇA
                mostrar_mao(&jogadores[0], 0);

                int opcao;
                printf("\nJogador 1: 1-Jogar carta  2-Pedir Truco\n");
                scanf("%d", &opcao);
                getchar();

                if (opcao == 2) {
                    processarTrucoServidor(cliente_global, &pontosRodada, jogadores, 0);
                    if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) break;
                    if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) break; // Corrida

                    system("cls");
                    printf("\n=== RODADA %d ===\n", rodada);
                    printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
                    printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
                    printf("Valor da rodada: %d\n", pontosRodada);
                    printf("Rodadas ganhas: J1=%d J2=%d\n\n", jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);
                    mostrar_mao(&jogadores[0], 0);

                    printf("\nJogador 1: Jogar carta\n");
                    int carta1 = validarEscolhaCarta(&jogadores[0]);
                    c1 = jogadores[0].mao.cartas[carta1];
                    jogadores[0].mao.cartas[carta1].ativo = false;
                } else {
                    int carta1 = validarEscolhaCarta(&jogadores[0]);
                    c1 = jogadores[0].mao.cartas[carta1];
                    jogadores[0].mao.cartas[carta1].ativo = false;
                }

                enviarInfoCompleta(cliente_global, c1, rodada, jogadores);

                sprintf(buffer, "CARTA_J1:%d:%d", c1.valor, c1.naipe);
                send(cliente_global, buffer, strlen(buffer), 0);
                printf("\nVoce jogou: %s de %s\n", c1.nome, convertedor_de_naipe(c1.naipe));
                printf("Aguardando Jogador 2...\n");

                bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
                if (bytesRecebidos <= 0) {
                    printf("Cliente desconectou\n");
                    break;
                }
                buffer[bytesRecebidos] = '\0';

                // TRATAMENTO DA RESPOSTA DO CLIENTE (CARTA OU TRUCO)
                if (strstr(buffer, "TRUCO_PEDIDO:") == buffer) {
                     // Cliente pediu truco antes de jogar a carta
                    processarTrucoServidor(cliente_global, &pontosRodada, jogadores, 1); // J2 é o iniciador
                    if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) break;
                    if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) break; // Corrida

                    // Se o truco foi aceito, J2 ainda deve jogar a carta.
                    // O cliente deve enviar a CARTA_J2 logo após a aceitação do Truco.
                    bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRecebidos <= 0) { printf("Cliente desconectou\n"); break; }
                    buffer[bytesRecebidos] = '\0';
                }

                if (strstr(buffer, "CARTA_J2:") == buffer) {
                    int valor2, naipe2;
                    sscanf(buffer, "CARTA_J2:%d:%d", &valor2, &naipe2);
                    c2.valor = valor2;
                    c2.naipe = (Naipe)naipe2;
                    strcpy(c2.nome, valor_para_nome(c2.valor)); // <-- CORRIGIDO

                    printf("Jogador 2 jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));
                }

                resultado = compararCartas(c1, c2);
            }
            else {
                // JOGADOR 2 (CLIENTE) COMEÇA
                printf("Aguardando Jogador 2 jogar...\n");

                sprintf(buffer, "SUA_VEZ_PRIMEIRO");
                send(cliente_global, buffer, strlen(buffer), 0);

                bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
                if (bytesRecebidos <= 0) {
                    printf("Cliente desconectou\n");
                    break;
                }
                buffer[bytesRecebidos] = '\0';

                // TRATAMENTO DA RESPOSTA DO CLIENTE (CARTA OU TRUCO)
                if (strstr(buffer, "TRUCO_PEDIDO:") == buffer) {
                    // Cliente pediu truco.
                    processarTrucoServidor(cliente_global, &pontosRodada, jogadores, 1); // J2 é o iniciador
                    if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) break;
                    if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) break; // Corrida

                    // Se o truco foi aceito, J2 ainda deve jogar a carta.
                    // O cliente deve enviar a CARTA_J2 logo após a aceitação do Truco.
                    bytesRecebidos = recv(cliente_global, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRecebidos <= 0) { printf("Cliente desconectou\n"); break; }
                    buffer[bytesRecebidos] = '\0';
                }

                if (strstr(buffer, "CARTA_J2:") == buffer) {
                    int valor2, naipe2;
                    sscanf(buffer, "CARTA_J2:%d:%d", &valor2, &naipe2);
                    c2.valor = valor2;
                    c2.naipe = (Naipe)naipe2;
                    strcpy(c2.nome, valor_para_nome(c2.valor)); // <-- CORRIGIDO

                    printf("Jogador 2 jogou: %s de %s\n", c2.nome, convertedor_de_naipe(c2.naipe));
                }

                // JOGADOR 1 (SERVIDOR) JOGA EM SEGUIDA
                mostrar_mao(&jogadores[0], 0);

                int opcao;
                printf("\nJogador 1: 1-Jogar carta  2-Pedir Truco\n");
                scanf("%d", &opcao);
                getchar();

                if (opcao == 2) {
                    processarTrucoServidor(cliente_global, &pontosRodada, jogadores, 0);
                    if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) break;
                    if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) break; // Corrida

                    system("cls");
                    printf("\n=== RODADA %d ===\n", rodada);
                    printf("Vira: %s de %s | Manilha: %s\n", vira.nome, convertedor_de_naipe(vira.naipe), valor_para_nome(manilha_valor));
                    printf("Placar: Jogador1=%d | Jogador2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
                    printf("Valor da rodada: %d\n", pontosRodada);
                    printf("Rodadas ganhas: J1=%d J2=%d\n\n", jogadores[0].rodadaGanha, jogadores[1].rodadaGanha);
                    mostrar_mao(&jogadores[0], 0);

                    printf("\nJogador 1: Jogar carta\n");
                    int carta1 = validarEscolhaCarta(&jogadores[0]);
                    c1 = jogadores[0].mao.cartas[carta1];
                    jogadores[0].mao.cartas[carta1].ativo = false;
                } else {
                    int carta1 = validarEscolhaCarta(&jogadores[0]);
                    c1 = jogadores[0].mao.cartas[carta1];
                    jogadores[0].mao.cartas[carta1].ativo = false;
                }

                enviarInfoCompleta(cliente_global, c1, rodada, jogadores);

                sprintf(buffer, "CARTA_J1:%d:%d", c1.valor, c1.naipe);
                send(cliente_global, buffer, strlen(buffer), 0);
                printf("\nVoce jogou: %s de %s\n", c1.nome, convertedor_de_naipe(c1.naipe));

                resultado = compararCartas(c2, c1);
                resultado = -resultado;
            }

            if (resultado > 0) {
                jogadores[0].rodadaGanha++;
                printf("\n>>> Voce venceu esta rodada! <<<\n");
            } else if (resultado < 0) {
                jogadores[1].rodadaGanha++;
                printf("\n>>> Jogador 2 venceu esta rodada! <<<\n");
            } else {
                printf("\n>>> Empate! <<<\n");
            }

            iniciadorRodada = determinarIniciadorProximaRodada(jogadores, iniciadorRodada, resultado);

            sprintf(buffer, "RESULTADO:%d:%d:%d:%d", resultado, jogadores[0].rodadaGanha, jogadores[1].rodadaGanha, iniciadorRodada);
            send(cliente_global, buffer, strlen(buffer), 0);

            rodada++;

            if (rodada <= 3 && jogadores[0].rodadaGanha < 2 && jogadores[1].rodadaGanha < 2) {
                printf("\nPressione Enter para continuar para a proxima rodada...");
                getchar();
            }
        }

        if (jogadores[0].pontos > 0 || jogadores[1].pontos > 0) {
            // Mao terminou por truco/corrida. O placar já foi atualizado pelo Truco.
        } else if (jogadores[0].rodadaGanha > jogadores[1].rodadaGanha) {
            jogadores[0].pontos += pontosRodada;
            printf("\n*** Voce venceu a mao! +%d ponto(s) ***\n", pontosRodada);
        } else if (jogadores[1].rodadaGanha > jogadores[0].rodadaGanha) {
            jogadores[1].pontos += pontosRodada;
            printf("\n*** Jogador 2 venceu a mao! +%d ponto(s) ***\n", pontosRodada);
        } else {
            printf("\n*** Mao empatada! ***\n");
        }

        quemIniciaMao = 1 - quemIniciaMao;

        if (jogadores[0].pontos >= 12 || jogadores[1].pontos >= 12) {
            if (jogadores[0].pontos >= 12) {
                printf("\n🎉 PARABENS! Voce venceu o jogo! 🎉\n");
                sprintf(buffer, "FIM:0");
            } else {
                printf("\n🎉 Jogador 2 venceu o jogo! 🎉\n");
                sprintf(buffer, "FIM:1");
            }
            send(cliente_global, buffer, strlen(buffer), 0);
            break;
        }

        sprintf(buffer, "PLACAR:%d:%d:%d", jogadores[0].pontos, jogadores[1].pontos, quemIniciaMao);
        send(cliente_global, buffer, strlen(buffer), 0);

        printf("\nPlacar atual: J1=%d J2=%d\n", jogadores[0].pontos, jogadores[1].pontos);
        printf("Proxima mao inicia com: %s\n", quemIniciaMao == 0 ? "Jogador 1" : "Jogador 2");
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