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
        analogWrite(Motor_Esquerdo_Positivo, 255); // mudar
        analogWrite(Motor_Esquerdo_Negativo, 255); // mudar
    }
    // Atualiza o histórico para o próximo ciclo
    ultimaVelEsq = velEsq;
    ultimaVelDir = velDir;
}
#pragma endregion

#pragma region BUSCAS

//=====================================|| BUSCA OFENSIVA ||=======================================================

void buscaOfensiva()
{

    if (valorJsumoF == HIGH || valorJsumoE == HIGH || valorJsumoD == HIGH)
    {
        if (!atacando)
        {
            inicioDoAtaque = millis();
            atacando = true;
        }

        unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

        // Define a velocidade máxima baseado no tempo do ciclo do ataque
        int velMax = (tempoDeAtaque < 500) ? 70 : 255;
        int velReduzida = velMax / 2;

        // 1. Oponente Centralizado
        if (valorJsumoF == HIGH && (valorJsumoE == HIGH && valorJsumoD == HIGH))
        {
            moverMotores(velMax, velMax);
        }
        // 2. Oponente na Diagonal Direita
        else if (valorJsumoF == HIGH && valorJsumoD == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velMax, velReduzida);
            }
            else
            {
                moverMotores(velMax, velMax);
            }
        }
        // 3. Oponente na Diagonal Esquerda
        else if (valorJsumoF == HIGH && valorJsumoE == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velReduzida, velMax);
            }
            else
            {
                moverMotores(velMax, velMax);
            }
        }
        // 4. Oponente apenas na frente
        else if (valorJsumoF == HIGH)
        {
            moverMotores(velMax, velMax);
        }
        // 5. Oponente apenas na Direita:
        else if (valorJsumoD == HIGH)
        {
            moverMotores(velMax, 0);
        }
        // 6. Oponente apenas na Esquerda:
        else if (valorJsumoE == HIGH)
        {
            moverMotores(0, velMax);
        }
    }
    else
    {
        // 7. Openente não detectado:
        atacando = false;

        if (lado == direita)
        {
            moverMotores(100, -100);
        }
        else if (lado == esquerda)
        {
            moverMotores(-100, 100);
        }
    }
}

void buscaCorna()
{

    if (valorLinhaE < 2000)
    {
        moverMotores(-255, -255); // Ré rápida
        delay(150);
        moverMotores(255, -255); // Gira para a direita
        delay(100);
        estadoAtual = AVANCAR_LENTO; // Volta a andar lentinho caso não veja nada
    }
    else if (valorLinhaD < 2000)
    {
        moverMotores(-255, -255); // Ré rápida
        delay(150);
        moverMotores(-255, 255); // Gira para a esquerda
        delay(100);
        estadoAtual = AVANCAR_LENTO;
    }

    if (abs(valorLDR - ldrReferencia) > 300)
    {
        estadoAtual = ATAQUE_RAPIDO;
    }
    else if (estadoAtual != ATAQUE_RAPIDO)
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

    // Máquina de Estados
    switch (estadoAtual)
    {

    case BUSCAR:
        digitalWrite(2, LOW);  // Usando apenas o LED 2
        moverMotores(80, -80); // Gira no próprio eixo devagar procurando alvo

        if (valorReceptorE || valorReceptorF || valorReceptorD)
        {
            estadoAtual = ALINHAR_FRENTE;
        }

        else if (millis() - tempoBusca >= 300)
        {
            estadoAtual = AVANCAR_LENTO;
        }
        break;

    case ALINHAR_FRENTE:
        if ((valorReceptorF && !valorReceptorE && !valorReceptorD) || (valorReceptorF && valorReceptorE && valorReceptorD))
        {
            moverMotores(0, 0); // Para os motores
            tempoTravamento = millis();
            estadoAtual = ESPERAR_ALVO;
        }
        else if (valorReceptorE)
        {
            moverMotores(-100, 100); // Ajuste fino para a esquerda
        }
        else if (valorReceptorD)
        {
            moverMotores(100, -100); // Ajuste fino para a direita
        }
        else
        {
            estadoAtual = BUSCAR;
        }
        break;

    case ESPERAR_ALVO:
        moverMotores(0, 0); // Mantém parado
        digitalWrite(2, HIGH);

        // Aguarda exatos 400ms antes de agir
        if (millis() - tempoTravamento >= 400)
        {
            digitalWrite(2, LOW);
            estadoAtual = AVANCAR_LENTO;
        }

        if (!valorReceptorF)
        {
            estadoAtual = BUSCAR;
        }
        break;

    case AVANCAR_LENTO:
        moverMotores(60, 60); // Avança devagar para facilitar a rampa
        break;

    case VIRAR_LATERAL_E:
        moverMotores(-255, 255); // Gira rápido à esquerda (JSumo viu)
        if (!valorJsumoE)
        {
            estadoAtual = BUSCAR;
        }
        break;

    case VIRAR_LATERAL_D:
        moverMotores(255, -255); // Gira rápido à direita (JSumo viu)
        if (!valorJsumoD)
        {
            estadoAtual = BUSCAR;
        }
        break;

    case ATAQUE_RAPIDO:
        moverMotores(255, 255); // Avança com tudo (adversário rampado)
        if (abs(valorLDR - ldrReferencia) <= 300)
        {
            estadoAtual = BUSCAR;
        }
        break;
    }
}

#endif