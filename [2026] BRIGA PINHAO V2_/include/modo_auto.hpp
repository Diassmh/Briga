//===============================================================================================//
//======================================// MODO AUTO //==========================================//
//===============================================================================================//

#ifndef MODO_AUTO_HPP
#define MODO_AUTO_HPP

#pragma region INCLUDES
#include "defines.hpp"
#include "move.hpp"

#pragma endregion

void LedFicaVerde(int numeroDaEtapa)
{
    // Altera apenas a cor do LED da etapa correspondente
    if (numeroDaEtapa >= 0 && numeroDaEtapa < 5)
    {
        leds[numeroDaEtapa] = CRGB::Green;
        FastLED.show(); // Envia o comando atualizado
    }
}

void TodosLedsAmarelos()
{
    // A FastLED possui a função fill_solid que preenche todo o array de uma vez
    fill_solid(leds, 5, CRGB::Yellow);
    FastLED.show(); // Envia o comando para acender a fita
}

#pragma region Menu Guia
void mostrarMenu()
{
    SerialBT.println("\n========== MENU BRIGA ==========");
    SerialBT.println("--- ATAQUE ---");
    SerialBT.println(" f : Frentao Progressivo");
    SerialBT.println(" F : Frentao BOLADO (Anti-Pescador)");
    SerialBT.println(" b : Costas");
    SerialBT.println(" c : Curva Leve");
    SerialBT.println(" C : Curva Pesada");
    SerialBT.println(" e : Desempate ");
    SerialBT.println(" v : Movimentacao em V");
    SerialBT.println(" d : Diagonal Rapida");
    SerialBT.println(" g : Giro 90°");

    SerialBT.println("\n--- DEFESA ---");
    SerialBT.println(" z : Basico");
    SerialBT.println(" A : Aquele La");

    SerialBT.println("\n--- CONFIGURACAO ---");
    SerialBT.println(" O     : Busca Ofensiva antiga");
    SerialBT.println(" I     : Busca Corna (padrao)");
    SerialBT.println(" < / > : Define Lado (Esq/Dir)");
    SerialBT.println(" =     : Define Lado Frente (Costas Apenas)");
    SerialBT.println(" l / r : Bandeira (Left/Right)");
    SerialBT.println(" S     : Debug JSumos laterais (Reset p/ sair)");
    SerialBT.println(" R     : Debug IRs frontais (Reset p/ sair)");
    SerialBT.println(" L     : Debug sensores de linha (Reset p/ sair)");
    SerialBT.println(" h     : Mostrar Menu");
    SerialBT.println("======================================\n");
}
#pragma endregion

#pragma region Function Check

void checarStatus()
{
    SerialBT.println("\n--- STATUS ATUAL ---");
    SerialBT.print("Estrategia: ");
    SerialBT.println(estrategia);

    SerialBT.print("Lado: ");
    SerialBT.println(lado == direita ? "Direita" : (lado == esquerda ? "Esquerda" : "Frente"));

    SerialBT.print("Bandeira: ");
    SerialBT.println(lado_Bandeira == B_Esquerda ? "Esquerda" : "Direita"); //

    SerialBT.print("Busca: ");
    if (ModoBusca == Ofensiva)
        SerialBT.println("Ofensiva antiga");
    else
        SerialBT.println("Corna");

    SerialBT.println("--------------------\n");
    LedFicaVerde(4);
    num_led = 5;
}
#pragma endregion

#pragma region Inicio Setup

void setupAuto()
{
    num_led = 2;
    LedFicaVerde(0);
    LedFicaVerde(1);
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(led, HIGH);
        delay(250);
        digitalWrite(led, LOW);
        delay(250);
    }

    SerialBT.begin("Briga_Briga_Based"); // Inicia Bluetooth do Celular (SerialBT):

    bool primeiraVez = true;
    while (true)
    {
        uint8_t brilhoPulsante = beatsin8(30, 40, 120);
        fill_solid(leds, num_led, CHSV(96, 255, brilhoPulsante));
        FastLED.show();

        if (SerialBT.connected() && primeiraVez)
        {
            SerialBT.println("CONECTADO");
            mostrarMenu();
            digitalWrite(led, HIGH);
            primeiraVez = false;
            LedFicaVerde(2);
            LedFicaVerde(3);
            num_led = 4;
        }

        // ============================|| ESCOLHA DE CONFIGURAÇÕES ||================================

        if (SerialBT.available())
        {
            BT = SerialBT.read();

            if (BT != 13 && BT != 10)
            {
                SerialBT.println(BT);

                if (BT == '<')
                {
                    lado = esquerda;
                }
                else if (BT == '>')
                {
                    lado = direita;
                }
                else if (BT == '=')
                {
                    lado = frente;
                }

                else if (BT == 'l')
                {
                    lado_Bandeira = B_Esquerda;
                }
                else if (BT == 'r')
                {
                    lado_Bandeira = B_Direita;
                }

                else if (BT == 'O')
                {
                    ModoBusca = Ofensiva;
                }
                else if (BT == 'I')
                {
                    ModoBusca = Corna;
                }
                else if (BT == 'B' || BT == 'Z' || BT == 'X' || BT == 'j')
                {
                    SerialBT.println("Comando antigo desativado nesta versao");
                }

                else if (BT == 'h')
                {
                    mostrarMenu();
                }
                else if (BT == 'S')
                {
                    running_Sensors = true;
                    for (;;)
                    {
                        LeituraSensores sensores = copiarLeituraSensores();
                        SerialBT.printf("JSUMO_E: %d\nJSUMO_D: %d\n", sensores.jsumoEsquerdo, sensores.jsumoDireito);
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
                else if (BT == 'R')
                {
                    running_Sensors = true;
                    for (;;)
                    {
                        LeituraSensores sensores = copiarLeituraSensores();
                        SerialBT.printf("IR_E: %d\nIR_F: %d\nIR_D: %d\n", sensores.receptorEsquerdo, sensores.receptorFrente, sensores.receptorDireito);
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
                else if (BT == 'L') // Comando 'L' para printar os sensores de linha
                {
                    running_Sensors = true;
                    for (;;)
                    {
                        LeituraSensores sensores = copiarLeituraSensores();
                        SerialBT.printf("LINHA_E: %d\nLINHA_D: %d\n", sensores.linhaEsquerda, sensores.linhaDireita);
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
                else
                {
                    if (BT > 48 && BT < 58)
                    { /* Ignora números soltos se houver */
                    }
                    else
                    {
                        estrategia = BT;
                    }
                }
                checarStatus();
            }
        }
#pragma endregion

#pragma region Comandos Controle

        if (IrReceiver.decode()) // No final do Loop checamos o receptor
        {
            IrReceiver.resume();
            if (IrReceiver.decodedIRData.command == botao1) // Se recebermos 1 do controle Remoto:
            {
                // Lê o LDR 10 vezes rápido para tirar uma média estável da luz ambiente
                if (medirLDR)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        somaLDR += analogRead(pinoLDR);
                        vTaskDelay(10);
                    }

                    ldrReferencia = somaLDR / 10;
                    medirLDR = false;
                }

                for (int i = 0; i < 2; i++) // Entramos no loop
                {

                    digitalWrite(led, LOW);
                    vTaskDelay(pdMS_TO_TICKS(80));

                    if (IrReceiver.decode()) // Se o juiz apertar o 1 e logo depois o 2, identificamos rapidamente aqui
                    {
                        IrReceiver.resume();
                        if (IrReceiver.decodedIRData.command == botao2) // se Recebermos 2
                        {
                            running_Sensors = true;
                            digitalWrite(led, HIGH);
                            break;
                        }
                    }

                    digitalWrite(led, HIGH);
                    vTaskDelay(pdMS_TO_TICKS(80));
                }
            }

            if (IrReceiver.decodedIRData.command == botao2) // se Recebermos 2
            {
                running_Sensors = true;
                servoBandeira.attach(SERVOMOTOR_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
                digitalWrite(led, LOW);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
#pragma endregion
#pragma region Estrategias
    //=============================|| COMEÇO DE LUTA ||================================
    TodosLedsAmarelos();
    SerialBT.println("BORAAAAAAAAA");

    // Queda da Bandeira:
    if (estrategia != 'A') // Se a estratégia for diferente do Defensivo Posicionado:
    {
        if (lado_Bandeira == B_Esquerda)
        {
            servoBandeira.write(180); // Cai para esquerda
        }
        else if (lado_Bandeira == B_Direita)
        {
            servoBandeira.write(0); // Cai para direita
        }
    }

    // Execução de movimentação inicial:
    switch (estrategia)
    {
    case 'f': // frentão Progressivo   1/2 dohyo
        SerialBT.print("FRENTAO PROGRESSIVO");
        moverMotores(48, 48);
        delay(4);
        moverMotores(60, 60);
        delay(4);
        moverMotores(72, 72);
        delay(4);
        moverMotores(84, 84);
        delay(4);
        moverMotores(96, 96);
        delay(4);
        moverMotores(128, 128);
        delay(8);
        moverMotores(160, 160);
        delay(8);
        moverMotores(196, 196);
        delay(8);
        moverMotores(228, 228);
        delay(12);
        moverMotores(255, 255);
        delay(90);
        break;

    case 'F': // Frentão Anti Pescador 1/2 dohyo

        SerialBT.print("FRENTAO BOLADO");
        moverMotores(255, 255);
        delay(250);
        break;

    case 'b': // Costas

        SerialBT.print("COSTAS");
        if (lado == frente)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(127, 127);
            delay(100);
            break;
        }
        else if (lado == esquerda)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(-255, 255);
            delay(50);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }
        break;

    case 'c': // Curva Leve

        SerialBT.print("CURVA LEVE");

        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(50);
            moverMotores(255, 255);
            delay(90);
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(90);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }
        break;

    case 'C': // Curva Pesada

        SerialBT.print("CURVA PESADA");

        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(130);
            moverMotores(0, 0);
            delay(15);
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(50);
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(60);
            moverMotores(255, 255);
            delay(130);
            moverMotores(0, 0);
            delay(15);
            moverMotores(-255, 255);
            delay(50);
            moverMotores(255, 255);
            delay(50);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }
        break;

    case 'z': // Basico
        SerialBT.print("Basico");
        break;

    case 'e': // desempate
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(130);
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(140);
            break;
        }
        break;

    case 'v':
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(40);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(120);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(180);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(40);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(140);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(180);
            lado = esquerda;
            break;
        }
        break;

    case 'A': // Aquele lá
    {
        bool flagM = false;
        bool giroEsquerda = false, giroDireita = false;

        // Essa abertura precisa de um lado definido. Com "frente", apenas pula a manobra.
        if (lado == frente)
        {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(80)); // Espera 80ms
        while (!flagM)
        {
            if (lado == esquerda)
            {
                servoBandeira.write(180); // Bandeira cai para a esquerda
                moverMotores(255, -255);
                delay(60);
                giroEsquerda = true;
                giroDireita = false;
                flagM = true;
            }
            else if (lado == direita)
            {
                servoBandeira.write(0); // Bandeira cai para a direita
                moverMotores(-255, 255);
                delay(60);
                giroDireita = true;
                giroEsquerda = false;
                flagM = true;
            }
        }
        if (giroDireita)
        {
            moverMotores(255, 255);
            delay(110);
            moverMotores(0, 0);
            delay(20);
            moverMotores(255, -255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            break;
        }
        else if (giroEsquerda)
        {
            moverMotores(255, 255);
            delay(110);
            moverMotores(0, 0);
            delay(20);
            moverMotores(-255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            break;
        }
        break;
    }

    case 'd':
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(30);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(60);
            moverMotores(255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(30);
            lado = esquerda;
            break;
        }
        break;

    case 'g':
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(80);
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(80);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }
        break;

    case 'j':
        // O antigo debug "j" movimentava o robo sem parar. Nesta versao ele fica seguro.
        moverMotores(0, 0);
        SerialBT.println("Debug j desativado");
        break;
    }

    // Reinicia apenas os controles da BuscaCorna. Nao muda a logica do LDR.
    estadoAtual = BUSCAR;
    tempoBusca = millis();
    tempoTransferencia = 0;
    tempoAvancoLento = 0;
    ultimaDeteccaoFrontal = millis();
    avancoSemAlvo = false;
    falhaRecuperacaoLinha = false;

    // Da tempo para a task publicar a primeira leitura antes da busca comecar.
    vTaskDelay(pdMS_TO_TICKS(10));
}

#pragma endregion

#pragma region Loop AUTO
void loopAuto()
{

    while (true)
    {
        if (ModoBusca == Ofensiva)
        {
            buscaOfensiva();
        }
        else
        {
            buscaCorna();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

#pragma endregion

#endif