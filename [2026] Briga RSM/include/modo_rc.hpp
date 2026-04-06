//===============================================================================================//
//======================================// MODO RC //============================================//
//===============================================================================================//

#ifndef MODO_RC_HPP
#define MODO_RC_HPP

#pragma region INCLUDES

#include "defines.hpp"
#include "move.hpp"
#include <PS4Controller.h>

#pragma endregion

// ============================== CONFIG RC ==============================

int zonaMorta = 11;
bool velLimitada = true;
bool ativarMira = false;
int limiteVelocidade = 160;
int limiteCurva = 120;

float expoDirecao = 1.5;  // Expo do analógico, quanto maior, mais suave fica perto do centro

// Estados anteriores dos botões.
bool lastTriangle = false;
bool lastSquare = false;

const char *BRIGA = "7c:9e:bd:fb:83:80";
const char *RESSACA = "a4:f0:0f:68:e7:f0";
const char *FUEGO = "d4:e9:f4:a4:9f:dc";
const char *CAIPORA = "a4:f0:0f:69:bc:c0";

// ============================== FUNÇÕES AUXILIARES ==============================

// Aplica expo em um valor entre -1 e 1.
float aplicarExpo(float valor, float expo)
{
    if (valor >= 0)
    {
        return pow(valor, expo);
    }
    else
    {
        return -pow(-valor, expo);
    }
}

// Pega o valor do gatilho (0 a 255) e transforma em 0.0 a 1.0.
float normalizarTrigger(int valor)
{
    if (valor <= zonaMorta) return 0.0;

    float saida = float(valor - zonaMorta) / float(255 - zonaMorta);

    if (saida < 0.0) saida = 0.0;
    if (saida > 1.0) saida = 1.0;

    return saida;
}

// Pega o valor do analógico esquerdo e transforma em -1.0 a 1.0, e aplica zona morta e exponencial (expoDirecao).
float normalizarDirecao(int valor)
{
    if (abs(valor) <= zonaMorta) return 0.0;

    float saida;

    if (valor > 0)
    {
        saida = float(valor - zonaMorta) / float(127 - zonaMorta);   // Lado direito do analógico
    }
    else
    {
        saida = float(valor + zonaMorta) / float(128 - zonaMorta);   // Lado esquerdo do analógico
    }

    if (saida > 1.0) saida = 1.0;
    if (saida < -1.0) saida = -1.0;

    return aplicarExpo(saida, expoDirecao);
}

// Gera uma correção automática de direção usando sensores.
// Esquerda = valor negativo
// Direita = valor positivo
float calcularCorrecaoSensores()
{
    float correcao = 0.0;

    // JSumo pesa mais
    if (valorJsumoE) correcao -= 0.55;
    if (valorJsumoD) correcao += 0.55;

    // IR pesa menos
    if (valorReceptorE) correcao -= 0.25;
    if (valorReceptorD) correcao += 0.25;

    if (correcao > 1.0) correcao = 1.0;
    if (correcao < -1.0) correcao = -1.0;

    return correcao;
}

// ============================== SETUP RC ==============================

void setupRC()
{
    velLimitada = true;
    ativarMira = false;
    limiteVelocidade = 160;
    running_Sensors = false;

    if (PS4.begin(BRIGA))
    {
        while (!PS4.isConnected())
        {
            digitalWrite(led, !digitalRead(led));
            vTaskDelay(120);
        }
    }

    digitalWrite(led, HIGH);
}

// ============================== LOOP RC ==============================

void loopRC()
{
    while (true)
    {
        if (PS4.isConnected())
        {
            digitalWrite(led, HIGH);

            // Essas duas variáveis guardam o que vai ser mandado pros motores.
            int velocidadeEsquerda = 0;
            int velocidadeDireita = 0;

            // ======================= || TOGGLES || =======================

            bool triangleNow = PS4.Triangle();
            bool squareNow = PS4.Square();

            // TRIANGLE: alterna entre 160 e 255
            // Só troca quando detecta novo aperto.
            if (triangleNow && !lastTriangle)
            {
                if (limiteVelocidade == 160)
                {
                    limiteVelocidade = 255;
                    velLimitada = false;
                }
                else
                {
                    limiteVelocidade = 160;
                    velLimitada = true;
                }
            }

            // SQUARE: liga/desliga aim assist
            if (squareNow && !lastSquare)
            {
                ativarMira = !ativarMira;
                running_Sensors = ativarMira;
            }

            // Atualiza os estados antigos dos botões
            lastTriangle = triangleNow;
            lastSquare = squareNow;

            // ======================= || MACROS || ========================

            // X = avanço reto forte
            if (PS4.Cross())
            {
                moverMotores(240, 255);
                vTaskDelay(30);
                continue;
            }

            // R1 = para
            else if (PS4.Down())
            {
                moverMotores(0, 0);
                vTaskDelay(30);
                continue;
            }

            // faz < no dohjo
            else if (PS4.Left() > 0)
            {
                moverMotores(-255, 255);
                vTaskDelay(50);
                moverMotores(255, 255);
                vTaskDelay(100);
                moverMotores(255, -255);
                vTaskDelay(90);
                moverMotores(255, 255);
                vTaskDelay(50);
                moverMotores(255, -255);
                vTaskDelay(50);
                moverMotores(0, 0);
                vTaskDelay(10);
                continue;
            }

            // faz > no dohjo
            else if (PS4.Right() > 0)
            {
                vTaskDelay(10);
                moverMotores(255, -255);
                vTaskDelay(50);
                moverMotores(255, 255);
                vTaskDelay(100);
                moverMotores(-255, 255);
                vTaskDelay(90);
                moverMotores(255, 255);
                vTaskDelay(50);
                moverMotores(-255, 255);
                vTaskDelay(50);
                moverMotores(0, 0);
                vTaskDelay(10);
                continue;
            }

            // virar 90 graos para a esquerda
            else if (PS4.L1() > 0)
            {
                vTaskDelay(10);
                moverMotores(0, 255);
                vTaskDelay(120);
                moverMotores(0, 0);
                vTaskDelay(10);
                continue;
            }

            // virar 90 graus para a direita
            else if (PS4.R1() > 0)
            {
                vTaskDelay(10);
                moverMotores(255, 0);
                vTaskDelay(120);
                moverMotores(0, 0);
                vTaskDelay(10);
                continue;
            }

            else if (PS4.Circle())
            {
                running_Sensors = true;

                if (valorJsumoF == HIGH && valorJsumoE == HIGH && valorJsumoD == HIGH)
                {
                    moverMotores(255, 255);
                    vTaskDelay(50);
                    continue;
                }
                else if (valorJsumoF == HIGH)
                {
                    moverMotores(180, 180);
                    vTaskDelay(30);
                    continue;
                }
                else if (valorJsumoE == HIGH)
                {
                    moverMotores(70, 220);
                    vTaskDelay(30);
                    continue;
                }
                else if (valorJsumoD == HIGH)
                {
                    moverMotores(220, 70);
                    vTaskDelay(30);
                    continue;
                }
                else if (valorReceptorF == HIGH)
                {
                    moverMotores(140, 140);
                    vTaskDelay(30);
                    continue;
                }
                else if (valorReceptorE == HIGH)
                {
                    moverMotores(40, 140);
                    vTaskDelay(30);
                    continue;
                }
                else if (valorReceptorD == HIGH)
                {
                    moverMotores(140, 40);
                    vTaskDelay(30);
                    continue;
                }
                else
                {
                    if (lado == esquerda)
                    {
                        moverMotores(-80, 80);
                        vTaskDelay(30);
                        continue;
                    }
                    else if (lado == direita)
                    {
                        moverMotores(80, -80);
                        vTaskDelay(30);
                        continue;
                    }
                }
            }

            else if (PS4.Options())
            {
                moverMotores(255, 255);
                vTaskDelay(30);
                moverMotores(-255, -255);
                vTaskDelay(30);
                continue;
            }

            // ======================= || MOVIMENTAÇÃO LIVRE || ==========================

            int r2 = PS4.R2Value();
            int l2 = PS4.L2Value();
            int direcaoRaw = PS4.LStickX();

            float frente = normalizarTrigger(r2);          // 0 a 1
            float re = normalizarTrigger(l2);              // 0 a 1
            float direcao = normalizarDirecao(direcaoRaw); // -1 a 1

            // Frente positiva, ré negativa
            float aceleracao = frente - re;

            if (ativarMira && aceleracao > 0.05)
            {
                float correcao = calcularCorrecaoSensores();

                if (valorJsumoF || valorJsumoE || valorJsumoD ||
                    valorReceptorF || valorReceptorE || valorReceptorD)
                {
                    // 65% da direção continua sendo do controle, o resto vem da mira
                    direcao = (direcao * 0.65) + correcao;
                }
            }

            // ======================= || GIRO PARADO || ==========================

            if (abs(aceleracao) < 0.03)
            {
                if (direcao < 0)
                {
                    int velCurva = int((-direcao) * limiteCurva);
                    velocidadeEsquerda = -velCurva;
                    velocidadeDireita = velCurva;
                }
                else if (direcao > 0)
                {
                    int velCurva = int((direcao) * limiteCurva);
                    velocidadeEsquerda = velCurva;
                    velocidadeDireita = -velCurva;
                }
                else
                {
                    velocidadeEsquerda = 0;
                    velocidadeDireita = 0;
                }
            }

            // ======================= || MOVIMENTO COM CURVA || ==========================

            else
            {
                // Mistura diferencial:
                // esquerda = frente + curva
                // direita  = frente - curva
                float velEsq = aceleracao + direcao;
                float velDir = aceleracao - direcao;

                // Se algum lado passou de 1, normaliza os dois
                float pico = max(abs(velEsq), abs(velDir));

                if (pico > 1.0)
                {
                    velEsq /= pico;
                    velDir /= pico;
                }

                velocidadeEsquerda = int(velEsq * limiteVelocidade);
                velocidadeDireita = int(velDir * limiteVelocidade);
            }

            // Envia pros motores
            moverMotores(velocidadeEsquerda, velocidadeDireita);
        }
        else
        {
            // Se perder conexão, para o robô
            moverMotores(0, 0);
            digitalWrite(led, LOW);
        }

        vTaskDelay(10);
    }
}

#endif
