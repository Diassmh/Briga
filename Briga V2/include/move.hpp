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

    if (inverteuDir || inverteuEsq)  // Se algum deles inverteu a rotação, limpamos o PWM:
    {
        analogWrite(Motor_Direito_Positivo, LOW);
        analogWrite(Motor_Direito_Negativo, LOW);
        analogWrite(Motor_Esquerdo_Positivo, LOW);
        analogWrite(Motor_Esquerdo_Negativo, LOW);
        delayMicroseconds(60); 
    }

    // --- ACIONAMENTO MOTOR DIREITO ---
    if (velDir >= 0)
    { 
        analogWrite(Motor_Direito_Positivo, abs(velDir)); 
        analogWrite(Motor_Direito_Negativo, LOW);         
    }
    else
    { 
        analogWrite(Motor_Direito_Negativo, abs(velDir)); 
        analogWrite(Motor_Direito_Positivo, LOW);
    }

    // --- ACIONAMENTO MOTOR ESQUERDO ---
    if (velEsq >= 0)
    { 
        analogWrite(Motor_Esquerdo_Positivo, abs(velEsq)); 
        analogWrite(Motor_Esquerdo_Negativo, LOW);
    }
    else
    { 
        analogWrite(Motor_Esquerdo_Negativo, abs(velEsq)); 
        analogWrite(Motor_Esquerdo_Positivo, LOW);
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
        int velMax = (tempoDeAtaque < 150) ? 127 : 255;
        int velReduzida = velMax / 1.5;

        // 1. Oponente Centralizado
        if (valorJsumoF == HIGH && (valorJsumoE == HIGH && valorJsumoD == HIGH))
        {
            moverMotores(velMax, velMax);
        }
        // 2. Oponente na Diagonal Direita
        else if (valorJsumoF == HIGH && valorJsumoD == HIGH)
        {
            if (tempoDeAtaque < 100)
            {
                moverMotores(velMax, velReduzida);
            }
            else
            {
                moverMotores(255, 255);
            }
        }
        // 3. Oponente na Diagonal Esquerda
        else if (valorJsumoF == HIGH && valorJsumoE == HIGH)
        {
            if (tempoDeAtaque < 100)
            {
                moverMotores(velReduzida, velMax);
            }
            else
            {
                moverMotores(255, 255);
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

//=======================================|| BUSCA DEFENSIVA ||============================================

void buscaDefensiva()
{
    static unsigned long tempoInicioCego = 0;
    static bool arcoParaEsquerda = true;
    if (valorJsumoF == HIGH && valorJsumoE == HIGH && valorJsumoD == HIGH)
    {
        moverMotores(255, 255);
        tempoInicioCego = millis();
    }
    else if (valorJsumoF == HIGH)
    {
        moverMotores(40, 40);
        tempoInicioCego = millis();
    }
    else
    {
       unsigned long tempoAtual = millis();

       
        if (tempoAtual - tempoInicioCego > 300)
        {
            arcoParaEsquerda = !arcoParaEsquerda; // Inverte o booleano (true vira false, e vice-versa)
            tempoInicioCego = tempoAtual;         // Zera o cronômetro para iniciar o novo arco
        }

        // Executa o movimento de arco
        if (arcoParaEsquerda)
        {
            // Arco para a ESQUERDA: Roda direita precisa girar mais rápido
            moverMotores(-80, 80); 
        }
        else
        {
            // Arco para a DIREITA: Roda esquerda precisa girar mais rápido
            moverMotores(80, -80); 
        }
    }
}

//=======================================|| BUSCA SÓ COM IR || ============================================

void buscaIR()
{
    if (valorReceptorF == LOW || valorReceptorE == LOW || valorReceptorD == LOW)
    {
        if (!atacando)
        {
            inicioDoAtaque = millis();
            atacando = true;
        }

        unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

        // Define a velocidade máxima baseado no tempo do ciclo do ataque
        int velMax = (tempoDeAtaque < 150) ? 127 : 255;
        int velReduzida = velMax / 1.5;

        // 1. Oponente Centralizado
        if (valorReceptorF == LOW && (valorReceptorE == LOW && valorReceptorD == LOW))
        {
            moverMotores(velMax, velMax);
        }
        // 2. Oponente na Diagonal Direita
        else if (valorReceptorF == LOW && valorReceptorD == LOW)
        {
            if (tempoDeAtaque < 100)
            {
                moverMotores(velMax, velReduzida);
            }
            else
            {
                moverMotores(255, 255);
            }
        }
        // 3. Oponente na Diagonal Esquerda
        else if (valorReceptorF == LOW && valorReceptorE == LOW)
        {
            if (tempoDeAtaque < 100)
            {
                moverMotores(velReduzida, velMax);
            }
            else
            {
                moverMotores(255, 255);
            }
        }
        // 4. Oponente apenas na frente
        else if (valorReceptorF == LOW)
        {
            moverMotores(velMax, velMax);
        }
        // 5. Oponente apenas na Direita:
        else if (valorReceptorD == LOW)
        {
            moverMotores(velMax, 0);
        }
        // 6. Oponente apenas na Esquerda:
        else if (valorReceptorE == LOW)
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
            moverMotores(70, -70);
        }
        else if (lado == esquerda)
        {
            moverMotores(-70, 70);
        }
    }

}

#endif