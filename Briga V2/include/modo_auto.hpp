//===============================================================================================//
//======================================// MODO AUTO //==========================================//
//===============================================================================================//

#ifndef MODO_AUTO_HPP
#define MODO_AUTO_HPP

#pragma region INCLUDES
#include "defines.hpp"
#include "move.hpp"

#pragma endregion

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
    SerialBT.println(" z : Defensivo Simples");
    SerialBT.println(" A : Aquele La");

    SerialBT.println("\n--- CONFIGURACAO ---");
    SerialBT.println(" X     : Desligar Sensores");
    SerialBT.println(" B     : Busca Defensiva");
    SerialBT.println(" I     : Busca IR");
    SerialBT.println(" < / > : Define Lado (Esq/Dir)");
    SerialBT.println(" =     : Define Lado Frente (Costas Apenas)");
    SerialBT.println(" l / r : Bandeira (Left/Right)");
    SerialBT.println(" S     : Debug Sensores (Reset p/ sair)");
    SerialBT.println(" R     : Debug IR");
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
    SerialBT.println(lado_Bandeira == B_Esquerda ? "Esquerda" : "Direita");
    
    SerialBT.print("Busca: ");
    if (ModoBusca == Ofensiva) SerialBT.println("Ofensiva");
    else if (ModoBusca == Defensiva) SerialBT.println("Defensiva");
    else SerialBT.println("Apenas IR");

    SerialBT.print("Sensores: ");
    SerialBT.println(transistor == Ligado ? "Ligados" : "Desligados");
    SerialBT.println("--------------------\n");
}
#pragma endregion

#pragma region Inicio Setup

void setupAuto()
{
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(led, HIGH);
        delay(250);
        digitalWrite(led, LOW);
        delay(250);
    }

    SerialBT.begin("Briga"); // Inicia Bluetooth do Celular (SerialBT):

    bool primeiraVez = true;
    while (true)
    {
        if (SerialBT.connected() && primeiraVez)
        {
            SerialBT.println("CONECTADO");
            mostrarMenu();
            digitalWrite(led, HIGH);
            primeiraVez = false;
        }

        // ============================|| ESCOLHA DE CONFIGURAÇÕES ||================================

        if (SerialBT.available())
        {
            BT = SerialBT.read();
        
            if (BT != 13 && BT != 10) 
            {
                SerialBT.println(BT);

                if (BT == '<') { lado = esquerda;     }
                else if (BT == '>') { lado = direita; }
                else if (BT == '=') { lado = frente;  }

                else if (BT == 'l') { lado_Bandeira = B_Esquerda; }
                else if (BT == 'r') { lado_Bandeira = B_Direita;  }

                else if (BT == 'X') { transistor = Desligado; digitalWrite(Pino_Transistor, LOW); }

                else if (BT == 'B') { ModoBusca = Defensiva; }
                else if (BT == 'O') { ModoBusca = Ofensiva;  }
                else if (BT == 'I') { ModoBusca = ApenasIR; transistor = Desligado; digitalWrite(Pino_Transistor, LOW);}
                
                else if (BT == 'h') { mostrarMenu(); }
                else if (BT == 'S') 
                { 
                    running_Sensors = true;
                    for (;;)
                    {
                        SerialBT.printf("ESFREN: %d\nFRENTE: %d\nDIRFRE: %d\n", valorJsumoE, valorJsumoF, valorJsumoD);
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
                else if(BT == 'R')
                {
                    digitalWrite(Pino_Transistor,LOW);
                    transistor = Desligado;
                    for (;;)
                    {
                        SerialBT.printf("ESQ: %d\nFRENTE: %d\nDIR: %d\n", valorReceptorE, valorReceptorF, valorReceptorD);
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
                else // Se não for nenhuma config de cima, é a Estratégia Principal
                {
                    if (BT > 48 && BT < 58) { /* Ignora números soltos se houver */ }
                    else { estrategia = BT; }
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
                digitalWrite(led, LOW);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
#pragma endregion
#pragma region Estrategias
    //=============================|| COMEÇO DE LUTA ||================================

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
        delay(54);
        break;

    case 'F': // Frentão Anti Pescador 1/2 dohyo

        SerialBT.print("FRENTAO BOLADO");
        moverMotores(255, 255);
        delay(100);
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
            delay(100);
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
            delay(100);
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
            delay(40);
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
            delay(40);
            moverMotores(255, 255);
            delay(90);
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
            delay(55);
            moverMotores(255, 255);
            delay(140);
            moverMotores(255, -255);
            delay(25);
            moverMotores(0, 0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(55);
            moverMotores(255, 255);
            delay(140);
            moverMotores(-255, 255);
            delay(25);
            moverMotores(0, 0);
            delay(10);
            lado = esquerda;
            break;
        }
    case 'z': // Defensivo

        SerialBT.print("Defensivo");
        while (true)
        {
            if (valorJsumoF == HIGH && valorJsumoE == HIGH && valorJsumoD == HIGH)
            {
                break;
            }
            else if (valorJsumoF == HIGH)
            {
                moverMotores(30, 30);
            }
            else
            {
                if (lado == esquerda)
                    moverMotores(-70, 70);
                else if (lado == direita)
                    moverMotores(70, -70);
            }
            vTaskDelay(5);
        }
        break;

    case 'e': // desempate
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(110);
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(110);
            break;
        }
    case 'v':
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(55);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(110);
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
            delay(55);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(110);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(180);
            lado = esquerda;
            break;
        }
    case 'A': // Aquele lá
    {
        bool flagM = false;
        bool giroEsquerda = false, giroDireita = false;

        vTaskDelay(pdMS_TO_TICKS(80)); // Espera 40ms
        while (!flagM)
        {
            if (lado == esquerda)
            {
                servoBandeira.write(180); // Bandeira cai para a esquerda
                moverMotores(255, -255);
                delay(75);
                giroEsquerda = true;
                giroDireita = false;
                flagM = true;
            }
            else if (lado == direita)
            {
                servoBandeira.write(0); // Bandeira cai para a direita
                moverMotores(-255, 255);
                delay(75);
                giroDireita = true;
                giroEsquerda = false;
                flagM = true;
            }
        }
        if (giroDireita)
        {
            moverMotores(255, 255);
            delay(90);
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
            delay(90);
            moverMotores(0, 0);
            delay(20);
            moverMotores(-255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            break;
        }
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
            lado = esquerda;
            break;
        }
    case 'g':
        if (lado == esquerda)
        {
            moverMotores(-255, 255);
            delay(80);
            moverMotores(0,0);
            delay(10);
            lado = direita;
            break;
        }
        else if (lado == direita)
        {
            moverMotores(255, -255);
            delay(80);
            moverMotores(0,0);
            delay(10);
            lado = esquerda;
            break;
        }
    }    
}

#pragma endregion

#pragma region Loop AUTO
void loopAuto()
{
    if ((transistor == Desligado) && (ModoBusca != ApenasIR))
    {
        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;
    }

    while (true)
    {
        if (ModoBusca == Ofensiva)
        {
            buscaOfensiva();
        }
        else if (ModoBusca == Defensiva)
        {
            buscaDefensiva();
        }
        else
        {
            buscaIR(); // Utiliza apenas o IR durante 2 segundos de luta
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

#pragma endregion

#endif