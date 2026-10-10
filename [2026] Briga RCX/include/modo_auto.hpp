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

    SerialBT.println("\n--- POSICAO ---");
    SerialBT.println(" P : Perto");
    SerialBT.println(" M : Medio");
    SerialBT.println(" L : Longe");

    SerialBT.println("\n--- ATAQUE ---");
    SerialBT.println(" f : Frentao Progressivo");
    SerialBT.println(" F : Frentao BOLADO");
    SerialBT.println(" b : Costas Simples");
    SerialBT.println(" B : Costas Ofensiva");
    SerialBT.println(" c : Curva Leve");
    SerialBT.println(" C : Curva Pesada");
    SerialBT.println(" e : Desempate ");
    SerialBT.println(" v : Movimentacao em V");
    SerialBT.println(" d : Diagonal Rapida");
    SerialBT.println(" g : Giro 90°");
    SerialBT.println(" A : Aquele La");
    SerialBT.println(" z : Basico");

    SerialBT.println("\n--- BUSCAS ---");
    SerialBT.println(" X : Corna Defensiva");

    SerialBT.println("\n--- CONFIGURACAO ---");
    SerialBT.println(" < / > : Define Lado (Esq/Dir)");
    SerialBT.println(" =     : Define Lado Frente");
    SerialBT.println(" l / r / q : Bandeira (Left/Right/Sem)");
    SerialBT.println(" S     : Debug Sensores");
    SerialBT.println(" R     : Debug Motores");
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

    SerialBT.print("Posicionamento: ");
    SerialBT.println(Posicao == Perto ? "Perto" : (Posicao == Medio ? "Medio" : "Longe"));

    SerialBT.print("Bandeira: ");
    SerialBT.println(lado_Bandeira == B_Esquerda ? "Esquerda" : "Direita");

    SerialBT.print("Busca: ");
    if (ModoBusca == Corna)
    {
        SerialBT.println("Corna");
    }
    else if (ModoBusca == CornaDefensiva)
    {
        SerialBT.println("Corna Defensiva");
    }
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

    SerialBT.begin("Briga_RCX"); // Inicia Bluetooth do Celular (SerialBT):

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

                // LEITURA DA POSIÇÃO
                else if (BT == 'P')
                {
                    Posicao = Perto;
                }
                else if (BT == 'M')
                {
                    Posicao = Medio;
                }
                else if (BT == 'L')
                {
                    Posicao = Longe;
                }

                else if (BT == 'l')
                {
                    lado_Bandeira = B_Esquerda;
                }
                else if (BT == 'r')
                {
                    lado_Bandeira = B_Direita;
                }
                else if (BT == 'q')
                {
                    lado_Bandeira = Sem_Bandeira;
                }
                else if (BT == 'X')
                {
                    ModoBusca = CornaDefensiva;
                }
                else if (BT == 'h')
                {
                    mostrarMenu();
                }
                else if (BT == 'S')
                {
                    running_Sensors = true;

                    // 1. Limpa a tela APENAS UMA VEZ ao entrar no modo debug
                    SerialBT.print("\033[2J");

                    for (;;)
                    {
                        if (SerialBT.available())
                        {
                            char comandoDbg = SerialBT.read();

                            if (comandoDbg == 'X' || comandoDbg == 'x')
                            {
                                SerialBT.println("\n\nSaindo do modo debug...");
                                running_Sensors = false;
                                break;
                            }
                        }

                        // 2. Move o cursor para o topo esquerdo SEM limpar a tela (Zero Flicker)
                        SerialBT.print("\033[H");

                        SerialBT.println("--- MODO DEBUG GERAL ---");
                        SerialBT.println("  (Envie 'X' para sair)\n");

                        // Todos os sensores alinhados com espaçamento fixo numérico
                        SerialBT.printf("LINHAS:  [E: %4d | D: %4d]\n", valorLinhaE, valorLinhaD);
                        SerialBT.printf("LDR:     [ %4d ]\n", valorLDR);
                        SerialBT.printf("IR:      [E: %4d | D: %4d]\n", valorReceptorE, valorReceptorD);
                        SerialBT.printf("JSUMO:   [E: %4d | D: %4d]\n", valorJsumoE, valorJsumoD);

                        // Atualização fluida a cada 100ms
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                }
                else if (BT == 'R')
                {
                    SerialBT.println("\n--- DEBUG DE MOTORES ATIVO ---");
                    SerialBT.println("Envie 'X' para Sair");

                    unsigned long tempoUltimoComando = millis();
                    int estadoMotor = 0; // 0 = Frente, 1 = Ré, 2 = Direita, 3 = Esquerda

                    SerialBT.println("Motor: FRENTE");
                    moverMotores(50, 50);

                    for (;;)
                    {

                        if (SerialBT.available())
                        {
                            char comandoDbg = SerialBT.read();
                            if (comandoDbg == 'X' || comandoDbg == 'x')
                            {
                                moverMotores(0, 0);
                                break;
                            }
                        }

                        if (millis() - tempoUltimoComando >= 4000)
                        {
                            tempoUltimoComando = millis();
                            estadoMotor++;

                            if (estadoMotor > 3)
                                estadoMotor = 0; // Volta pro começo do ciclo

                            switch (estadoMotor)
                            {
                            case 0:
                                SerialBT.println("Motor: FRENTE");
                                moverMotores(50, 50);
                                break;
                            case 1:
                                SerialBT.println("Motor: RE");
                                moverMotores(-50, -50);
                                break;
                            case 2:
                                SerialBT.println("Motor: DIREITA");
                                moverMotores(50, -50);
                                break;
                            case 3:
                                SerialBT.println("Motor: ESQUERDA");
                                moverMotores(-50, 50);
                                break;
                            }
                        }

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

                for (int i = 0; i < 2; i++) // Entramos no loop
                {

                    digitalWrite(led, LOW);
                    vTaskDelay(pdMS_TO_TICKS(80));

                    if (IrReceiver.decode()) // Se o juiz apertar o 1 e logo depois o 2, identificamos rapidamente aqui
                    {
                        IrReceiver.resume();
                        if (IrReceiver.decodedIRData.command == botao2) // se Recebermos 2
                        {
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
        else if (lado_Bandeira == Sem_Bandeira)
        {
            servoBandeira.write(90);
        }
    }

    // Lógica do multiplicador com base na posição
    float multDist = 1.0;

    if (Posicao == Perto)
    {
        multDist = 0.7; // Corta o tempo de avanço
    }
    else if (Posicao == Medio)
    {
        multDist = 1.0; // Mantém os tempos originais
    }
    else if (Posicao == Longe)
    {
        multDist = 1.6; // Aumenta o tempo para cruzar o dohyo
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
        delay(int(90 * multDist));
        break;

    case 'F': // Frentão Anti Pescador 1/2 dohyo
        SerialBT.print("FRENTAO BOLADO");
        moverMotores(255, 255);
        delay(int(250 * multDist));
        break;

    case 'b': // Costas Simples
        SerialBT.print("COSTAS SIMPLES");
        if (lado == frente)
        {
            moverMotores(-255, -255);
            delay(int(150 * multDist));
            moverMotores(0, 0);
            delay(10);
            break;
        }
        else if (lado == esquerda)
        {
            moverMotores(-255, -255);
            delay(int(70 * multDist));
            moverMotores(255, -255);
            delay(50);
            moverMotores(-255, -255);
            delay(int(80 * multDist));
            moverMotores(0, 0);
            delay(15);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(-255, -255);
            delay(int(70 * multDist));
            moverMotores(-255, 255);
            delay(50);
            moverMotores(-255, -255);
            delay(int(80 * multDist));
            moverMotores(0, 0);
            delay(15);
            lado = direita;
            break;
        }

    case 'B': // Costas Ofensiva
        SerialBT.print("COSTAS OFENSIVA");
        if (lado == frente)
        {
            moverMotores(-255, -255);
            delay(int(150 * multDist));
            moverMotores(0, 0);
            delay(10);
            break;
        }
        else if (lado == esquerda)
        {
            moverMotores(-255, -255);
            delay(int(120 * multDist));
            moverMotores(-255, 255);
            delay(50);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(int(160 * multDist));
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(-255, -255);
            delay(int(120 * multDist));
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(int(160 * multDist));
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }

    case 'c': // Curva Leve
        SerialBT.print("CURVA LEVE");
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(int(120 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(40);
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
            delay(int(120 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(40);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }

    case 'C': // Curva Pesada
        SerialBT.print("CURVA PESADA");
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(int(170 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(int(50 * multDist));
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
            delay(int(170 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(50);
            moverMotores(255, 255);
            delay(int(50 * multDist));
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }

    case 'z': // Basico
        SerialBT.print("Basico");
        break;

    case 'e': // Desempate
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

    case 'v': // Movimento em V
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(40);
            moverMotores(255, 255);
            delay(int(160 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(120);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(int(180 * multDist));
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(40);
            moverMotores(255, 255);
            delay(int(160 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(140);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(int(180 * multDist));
            lado = esquerda;
            break;
        }

    case 'a': // Aquele lá
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(int(110 * multDist));
            moverMotores(0, 0);
            delay(20);
            moverMotores(255, -255);
            delay(130);
            moverMotores(0, 0);
            delay(10);
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(60);
            moverMotores(255, 255);
            delay(int(110 * multDist));
            moverMotores(0, 0);
            delay(20);
            moverMotores(-255, 255);
            delay(130);
            moverMotores(0, 0);
            delay(10);
            break;
        }

    case 'd': // Diagonal
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(60);
            moverMotores(255, 255);
            delay(int(150 * multDist));
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
            delay(int(150 * multDist));
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(30);
            lado = esquerda;
            break;
        }

    case 'g': // Giro 45°
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(50);
            moverMotores(0, 0);
            delay(500);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(50);
            moverMotores(0, 0);
            delay(500);
            lado = esquerda;
            break;
        }

    case 'j':
        while (true)
        {
            moverMotores(255, 255);
            delay(10);
        }
    }
    running_Sensors = true;
}

#pragma endregion

#pragma region Loop AUTO
void loopAuto()
{

    while (true)
    {
        if (ModoBusca == Corna)
        {
            buscaCorna();
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

#pragma endregion

#endif