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
        analogWrite(Motor_Direito_Positivo, 127);
        analogWrite(Motor_Direito_Negativo, 127);
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
        analogWrite(Motor_Esquerdo_Positivo, 127);
        analogWrite(Motor_Esquerdo_Negativo, 127);
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
        int velMax = (tempoDeAtaque < 180) ? 127 : 255;
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
            moverMotores(90, -90);
        }
        else if (lado == esquerda)
        {
            moverMotores(-90, 90);
        }
    }
}

//=======================================|| BUSCA CRIMINOSA ||============================================

void buscaCriminosa()
{

    static unsigned long tempoBuscaAtiva = 0;
    static unsigned long inicioAtaqueTotal = 0;

    /* if ((valorLinhaE == HIGH) && (valorLinhaD == LOW)){
           moverMotores(0,0);
           vTaskDelay(pdMS_TO_TICKS(10));
           Estado = GIRO_BORDA_ESQ;
       }
       else if ((valorLinhaD == HIGH) && (valorLinhaE == LOW)) {
           moverMotores(0,0);
           vTaskDelay(pdMS_TO_TICKS(10));
           Estado = GIRO_BORDA_DIR;
       }
       else if ((valorLinhaE == HIGH) && (valorLinhaD == HIGH))
           moverMotores(-255,-255);
           vTaskDelay(pdMS_TO_TICKS(100));
       */

    switch (Estado)
    {

    case BUSCA_FURTIVA:

        digitalWrite(Pino_Transistor, LOW); //  Desliga J sumos
        transistor = Desligado;

        if (valorReceptorF == HIGH || valorReceptorE == HIGH || valorReceptorD == HIGH)
        {
            Estado = ATAQUE_IR_LENTO;
            break;
        }

        else
        {
            moverMotores(85, 85);
        }
        break;
    case GIRO_BORDA_ESQ:

        moverMotores(-255, -255);
        vTaskDelay(pdMS_TO_TICKS(70));
        moverMotores(255, -255);
        vTaskDelay(pdMS_TO_TICKS(50));

        servoBandeira.write(0);

        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;

        tempoBuscaAtiva = millis();
        Estado = BUSCA_ATIVA;
        break;

    case GIRO_BORDA_DIR:

        moverMotores(-255, -255);
        vTaskDelay(pdMS_TO_TICKS(70));
        moverMotores(-255, 255);
        vTaskDelay(pdMS_TO_TICKS(50));

        servoBandeira.write(180); // Bandeira cai para a esquerda

        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;

        tempoBuscaAtiva = millis();
        Estado = BUSCA_ATIVA;
        break;

    case BUSCA_ATIVA:

        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;
        moverMotores(85, 85);

        if (valorJsumoF == HIGH || valorJsumoE == HIGH || valorJsumoD == HIGH)
        {
            Estado = ATAQUE_JSUMO_LENTO;
            break;
        }

        if (millis() - tempoBuscaAtiva >= 300)
        {
            digitalWrite(Pino_Transistor, LOW);
            transistor = Desligado;
            Estado = BUSCA_FURTIVA;
            break;
        }

        break;

    case ATAQUE_IR_LENTO:

        if (valorReceptorF == LOW && valorReceptorE == LOW && valorReceptorD == LOW)
        {
            Estado = BUSCA_FURTIVA;
            break;
        }

        if (valorReceptorF == HIGH && valorReceptorE == HIGH && valorReceptorD == HIGH)
        {

            // Investida 1
            moverMotores(127, 127);
            vTaskDelay(pdMS_TO_TICKS(40));

            // Pausa rápida
            moverMotores(0, 0);
            vTaskDelay(pdMS_TO_TICKS(20));

            // Investida 2
            moverMotores(127, 127);
            vTaskDelay(pdMS_TO_TICKS(40));

            moverMotores(0, 0);
            vTaskDelay(pdMS_TO_TICKS(20));

            inicioAtaqueTotal = millis();
            Estado = ATAQUE_TOTAL;
            break;
        }

        if (valorReceptorF == HIGH)
        {
            moverMotores(50, 50);
        }
        else if (valorReceptorE == HIGH)
        {
            moverMotores(-60, 80);
        }
        else if (valorReceptorD == HIGH)
        {
            moverMotores(80, -60);
        }

        break;

    case ATAQUE_JSUMO_LENTO:

        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;

        if (valorJsumoF == LOW && valorJsumoE == LOW && valorJsumoD == LOW)
        {
            digitalWrite(Pino_Transistor, LOW);
            transistor = Desligado;
            Estado = BUSCA_FURTIVA;
            break;
        }

        if (valorJsumoF == HIGH && valorJsumoE == HIGH && valorJsumoD == HIGH)
        {

            moverMotores(127, 127);
            vTaskDelay(pdMS_TO_TICKS(40));

            moverMotores(0, 0);
            vTaskDelay(pdMS_TO_TICKS(20));

            moverMotores(127, 127);
            vTaskDelay(pdMS_TO_TICKS(40));

            moverMotores(0, 0);
            vTaskDelay(pdMS_TO_TICKS(20));

            inicioAtaqueTotal = millis();
            Estado = ATAQUE_TOTAL;
            break;
        }

        if (valorJsumoF == HIGH)
        {
            moverMotores(50, 50);
        }
        else if (valorJsumoE == HIGH)
        {
            moverMotores(-60, 80);
        }
        else if (valorJsumoD == HIGH)
        {
            moverMotores(80, -60);
        }

        break;

    case ATAQUE_TOTAL:

        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;

        if (millis() - inicioAtaqueTotal < 300)
        {
            moverMotores(127, 127); // Arrancada firme para ganhar tração
        }
        else
        {
            moverMotores(255, 255); // Potência máxima para finalizar
        }

        if (valorJsumoF == LOW && valorJsumoE == LOW && valorJsumoD == LOW &&
            valorReceptorF == LOW && valorReceptorE == LOW && valorReceptorD == LOW)
        {
            moverMotores(0, 0);
            vTaskDelay(pdMS_TO_TICKS(50)); // Trava a roda para não cair por inércia

            digitalWrite(Pino_Transistor, LOW);
            transistor = Desligado;

            Estado = BUSCA_FURTIVA;
            break;
        }

        if (valorJsumoF == LOW && valorReceptorF == LOW)
        {
            if (valorJsumoE == HIGH || valorJsumoD == HIGH)
            {
                Estado = ATAQUE_JSUMO_LENTO;
            }
            else if (valorReceptorE == HIGH || valorReceptorD == HIGH)
            {
                digitalWrite(Pino_Transistor, LOW);
                transistor = Desligado;
                Estado = ATAQUE_IR_LENTO;
            }
        }

        break;
    }
}

//=======================================|| BUSCA SÓ COM IR || ============================================

void buscaIR()
{

    if (flagPrimeiraVez)
    {
        tempoInicioIR = millis();
        flagPrimeiraVez = false;
    }

    if (millis() - tempoInicioIR > 5000)
    {
        digitalWrite(Pino_Transistor, HIGH);
        transistor = Ligado;
        ModoBusca = Ofensiva;
    }

    if (valorReceptorF == HIGH || valorReceptorE == HIGH || valorReceptorD == HIGH)
    {
        if (!atacando)
        {
            inicioDoAtaque = millis();
            atacando = true;
        }

        unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

        // Define a velocidade máxima baseado no tempo do ciclo do ataque
        int velMax = (tempoDeAtaque < 500) ? 100 : 255;
        int velReduzida = velMax / 2;

        // 1. Oponente Centralizado
        if (valorReceptorF == HIGH && (valorReceptorE == HIGH && valorReceptorD == HIGH))
        {
            moverMotores(velMax, velMax);
        }
        // 2. Oponente na Diagonal Direita
        else if (valorReceptorF == HIGH && valorReceptorD == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velMax, velReduzida);
            }
            else
            {
                moverMotores(255, 255);
            }
        }
        // 3. Oponente na Diagonal Esquerda
        else if (valorReceptorF == HIGH && valorReceptorE == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velReduzida, velMax);
            }
            else
            {
                moverMotores(255, 255);
            }
        }
        // 4. Oponente apenas na frente
        else if (valorReceptorF == HIGH)
        {
            moverMotores(velMax, velMax);
        }
        // 5. Oponente apenas na Direita:
        else if (valorReceptorD == HIGH)
        {
            moverMotores(velMax, 0);
        }
        // 6. Oponente apenas na Esquerda:
        else if (valorReceptorE == HIGH)
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
            moverMotores(90, -90);
        }
        else if (lado == esquerda)
        {
            moverMotores(-90, 90);
        }
    }
}

#endif