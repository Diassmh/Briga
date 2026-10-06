//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//
#pragma region INCLUDES

#ifndef MOVE_H
#define MOVE_H

#include <defines.hpp>

#pragma endregion

//===============================================================================================//
//========================================//MOVIMENTACAO//=======================================//
//===============================================================================================//

#pragma region PWM

void moverMotores(int velEsq, int velDir)
{
    // Variáveis estáticas lembram o último comando enviado mesmo após a função terminar
    static int ultimaVelEsq = 0;
    static int ultimaVelDir = 0;

    // Checa se algum dos motores inverteu a rotação:
    bool inverteuDir = (velDir > 0 && ultimaVelDir < 0) || (velDir < 0 && ultimaVelDir > 0);
    bool inverteuEsq = (velEsq > 0 && ultimaVelEsq < 0) || (velEsq < 0 && ultimaVelEsq > 0);

    if (inverteuDir || inverteuEsq) // Se algum deles inverteu a rotação, limpamos o PWM:
    {
        analogWrite(Motor_Direito_Positivo, LOW);
        analogWrite(Motor_Direito_Negativo, LOW);
        analogWrite(Motor_Esquerdo_Positivo, LOW);
        analogWrite(Motor_Esquerdo_Negativo, LOW);
        delayMicroseconds(60);
    }

    // --- ACIONAMENTO MOTOR DIREITO ---
    if (velDir > 0)
    {
        analogWrite(Motor_Direito_Positivo, abs(velDir));
        analogWrite(Motor_Direito_Negativo, LOW);
    }
    else if (velDir < 0)
    {
        analogWrite(Motor_Direito_Negativo, abs(velDir));
        analogWrite(Motor_Direito_Positivo, LOW);
    }
    else if (velDir == 0)
    {
        analogWrite(Motor_Direito_Positivo, 255); // mudar
        analogWrite(Motor_Direito_Negativo, 255); // mudar
    }

    // --- ACIONAMENTO MOTOR ESQUERDO ---
    if (velEsq > 0)
    {
        analogWrite(Motor_Esquerdo_Positivo, abs(velEsq));
        analogWrite(Motor_Esquerdo_Negativo, LOW);
    }
    else if (velEsq < 0)
    {
        analogWrite(Motor_Esquerdo_Negativo, abs(velEsq));
        analogWrite(Motor_Esquerdo_Positivo, LOW);
    }

    else if (velEsq == 0)
    {
        analogWrite(Motor_Esquerdo_Positivo, 255);
        analogWrite(Motor_Esquerdo_Negativo, 255);
    }
    // Atualiza o histórico para o próximo ciclo
    ultimaVelEsq = velEsq;
    ultimaVelDir = velDir;
}
#pragma endregion

#pragma region BUSCAS

void buscaCorna()
{

    if ((valorLinhaE < 500 && valorLinhaE > 0) && (valorLinhaD < 500 && valorLinhaD > 0))
    {
        moverMotores(-255, -255);
        delay(130);
        moverMotores(255, -255);
        delay(70);
        moverMotores(0, 0);
        delay(20);
        estadoAtual = AVANCAR_LENTO;
    }
    else if (valorLinhaE < 500 && valorLinhaE > 0)
    {
        servoBandeira.write(0);
        moverMotores(-255, -255); // Ré rápida
        delay(150);
        moverMotores(0, 0);
        delay(15);
        moverMotores(255, -255); // Gira para a direita
        delay(80);
        moverMotores(0, 0);
        delay(20);
        estadoAtual = AVANCAR_LENTO;
    }
    else if (valorLinhaD < 500 && valorLinhaD > 0)
    {
        servoBandeira.write(180);
        moverMotores(-255, -255); // Ré rápida
        delay(150);
        moverMotores(0, 0);
        delay(15);
        moverMotores(-255, 255); // Gira para a esquerda
        delay(80);
        moverMotores(0, 0);
        delay(20);
        estadoAtual = AVANCAR_LENTO;
    }

    if (valorLDR < 100)
    {
        estadoAtual = ATAQUE_RAPIDO;
    }
    else if (estadoAtual != ATAQUE_RAPIDO)
    {
        if (!irVendo)
        {
            if (valorJsumoE)
            {
                estadoAtual = VIRAR_LATERAL_E;
            }
            else if (valorJsumoD)
            {
                estadoAtual = VIRAR_LATERAL_D;
            }
        }
    }

    // Máquina de Estados
    switch (estadoAtual)
    {

    case ALINHAR_FRENTE:
        if (valorReceptorE && valorReceptorD)
        {
            moverMotores(0, 0); // Para os motores
            tempoTravamento = millis();
            estadoAtual = ESPERAR_ALVO;
        }
        else if (valorReceptorE)
        {
            moverMotores(-255, 255); // Ajuste fino para a esquerda
            delay(30);
            moverMotores(0, 0);
            delay(10);
        }
        else if (valorReceptorD)
        {
            moverMotores(255, -255); // Ajuste fino para a direita
            delay(30);
            moverMotores(0, 0);
            delay(10);
        }
        else
        {
            estadoAtual = AVANCAR_LENTO; // Se perder o alvo completamente, volta a rastejar
        }
        break;

    case ESPERAR_ALVO:
    {
        // Se o adversário se mover e sair da mira de um dos lados, volta a alinhar
        if (!valorReceptorE || !valorReceptorD)
        {
            estadoAtual = ALINHAR_FRENTE;
            break;
        }

        unsigned long tempoDecorrido = millis() - tempoTravamento;

        if (tempoDecorrido >= 1000 && (tempoDecorrido % 1000) < 60)
        {
            moverMotores(199, 199); // Acelerada de leve
        }
        else
        {
            moverMotores(0, 0); // Permanece travado aguardando
        }
        break;
    }
    case AVANCAR_LENTO:

        if (irVendo)
        {
            estadoAtual = ALINHAR_FRENTE;
        }
        else
        {
            if (ModoBusca == Corna)
            {
                moverMotores(40, 40);
            }
            else if (ModoBusca == CornaDefensiva)
            {

                if (millis() % 2000 < 60)
                {
                    moverMotores(199, 199); // PWM suficiente para dar um "tranco" visível
                }
                else
                {
                    moverMotores(0, 0); // Fica como uma rocha no resto do tempo
                }
            }
        }
        break;

    case VIRAR_LATERAL_E:

        moverMotores(-255, 255);
        vTaskDelay(40);
        moverMotores(0, 0);
        vTaskDelay(10);

        if (irVendo)
        {
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (!valorJsumoE)
        {
            estadoAtual = AVANCAR_LENTO;
        }

        break;

    case VIRAR_LATERAL_D:

        moverMotores(255, -255); // Gira rápido à direita (JSumo viu)
        vTaskDelay(40);
        moverMotores(0, 0);
        vTaskDelay(10);

        if (irVendo)
        {

            estadoAtual = ALINHAR_FRENTE;
        }
        else if (!valorJsumoD)
        {
            estadoAtual = AVANCAR_LENTO;
        }
        break;

    case ATAQUE_RAPIDO:

        moverMotores(255, 255);

        if (valorLDR >= 100)
        {
            estadoAtual = AVANCAR_LENTO;
        }
        break;
    }
}

#endif