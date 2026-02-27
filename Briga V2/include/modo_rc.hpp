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

int zonaMorta = 20;               // Zona desconsiderada do controle de PS4
volatile int r2 = 0;              // Valor do gatilho direito do PS4 (R2)
volatile int l2 = 0;              // Valor do gatilho esquerdo do PS4 (L2)
volatile int direcao = 0;         // Valor do direcional esquerdo do PS4 em X (LStickX)
volatile bool velLimitada = true; // Switch definido pra limitar a velocidade do motor
int limiteVelocidade = 160;       // Limite de velocidade do motor
bool ativarMira = false;

float coefAtenuacao = 1.8; // Atenuacao da velocidade em curvas (deve ser FLOAT)
float coefReverse = 0.9;   // Coeficiente para balancear a re (deve ser FLOAT)
float limiteCurva = 200;   // Velocidade limite em curvas puras (deve ser FLOAT)

const char *BRIGA = "7c:9e:bd:fb:83:80";
const char *RESSACA = "a4:f0:0f:68:e7:f0";
const char *FUEGO = "b0:cb:d8:03:a6:88";

#pragma region Setup RC

void setupRC()
{
    if (PS4.begin(BRIGA))
    { // MAC Adress definido no inicio do código
        Serial.println("Bluetooth inicializado, aguardando controle...");
        while (!PS4.isConnected())
        {
            Serial.println("Tentando Conectar");
            digitalWrite(led, LOW);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

#pragma endregion

#pragma region Loop RC

void loopRC()
{

    while (true)
    {
        if (PS4.isConnected())
        {
            digitalWrite(led, HIGH);
            int velocidadeEsquerda = 0; // Velocidade do motor esquerdo
            int velocidadeDireita = 0;  // Velocidade do motor direito

            // ======================= || COMANDOS COM BOTÕES (MACROS) || ===================

            if (PS4.Cross())
            { // X = Frentão
                moverMotores(255, 255);
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }

            else if (PS4.Left() > 0)
            {

                moverMotores(-255, 255); // Gira para a esquerda
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(150));
                moverMotores(255, -255); // Gira para a direita
                vTaskDelay(pdMS_TO_TICKS(90));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(255, -255);
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            else if (PS4.Right() > 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10)); // Delay para garantir o fim da logica anterior

                moverMotores(255, -255); // Gira para a direita
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(150));
                moverMotores(-255, 255); // Gira para a esquerda
                vTaskDelay(pdMS_TO_TICKS(90));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(-255, 255);
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            else if (PS4.Square() && !ativarMira)
            {
                ativarMira = true;
                running_Sensors = true;
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            else if (PS4.Square())
            {
                ativarMira = false;
                running_Sensors = false;
                vTaskDelay(pdMS_TO_TICKS(200));
            }

            // ======================= || LIMITADOR DE VELOCIDADE || =======================

            // Limita a velocidade a 127
            else if (PS4.Triangle() && !velLimitada)
            {
                velLimitada = true;
                limiteVelocidade = 160;         // Define o limite reduzido
                vTaskDelay(pdMS_TO_TICKS(200)); // Evita multiplas leituras
            }

            // Permite a velocidade de 255
            else if (PS4.Triangle())
            {
                velLimitada = false;
                limiteVelocidade = 255;         // Restaura o limite maximo
                vTaskDelay(pdMS_TO_TICKS(200)); // Evita multiplas leituras
            }

            else if (PS4.Circle())
            {
                running_Sensors = true;
                if (valorJsumoF == HIGH && valorJsumoE == HIGH && valorJsumoD == HIGH)
                {
                    moverMotores(255, 255);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    continue;
                }
                else if (valorJsumoF == HIGH)
                {
                    moverMotores(127, 127);
                    vTaskDelay(pdMS_TO_TICKS(30));
                    continue;
                }
                else
                {
                    if (lado == esquerda)
                    {
                        moverMotores(-80, 80);
                        vTaskDelay(pdMS_TO_TICKS(30));
                        continue;
                    }
                    else if (lado == direita)
                    {
                        moverMotores(80, -80);
                        vTaskDelay(pdMS_TO_TICKS(30));
                        continue;
                    }
                }
            }

            // ======================= || MOVIMENTAÇÃO LIVRE || ==========================

            else
            {

                r2 = PS4.R2Value();      // Armazena o valor de R2
                l2 = PS4.L2Value();      // Armazena o valor de L2
                direcao = PS4.LStickX(); // Armazena o valor em X do Direcional Esquerdo

                // Se nenhum gatilho estiver pressionado (considerando a zona morta)

                //===================== || CURVAS PURAS || =============================

                if (r2 <= zonaMorta && l2 <= zonaMorta)
                {
                    velocidadeEsquerda = 0;
                    velocidadeDireita = 0;

                    // Se os gatilhos estao parados, verifica se o analogico quer girar
                    if (direcao < -zonaMorta)
                    { // Curva pra esquerda parado
                        velocidadeEsquerda = -int((limiteCurva / 255) * (abs(direcao)));
                        velocidadeDireita = int((limiteCurva / 255) * (abs(direcao)));
                    }
                    else if (direcao > zonaMorta)
                    { // Curva pra direita parado
                        velocidadeEsquerda = int((limiteCurva / 255) * (abs(direcao)));
                        velocidadeDireita = -int((limiteCurva / 255) * (abs(direcao)));
                    }
                    // Se nem gatilho nem analogico estiverem ativos, as velocidades continuam 0.
                }

                //===================== || GATILHOS PUROS || =============================

                else
                { // Pelo menos um gatilho pressionado (R2 ou L2)

                    if (r2 > zonaMorta) // Gatilho direito (frente)
                    {
                        if (ativarMira) // Mira guiada ativa
                        {
                            int velocidade = map(r2, zonaMorta, 255, 0, limiteVelocidade);
                            int somaSensores = (valorJsumoE) + (valorJsumoF) + (valorJsumoD);

                            if (somaSensores == 3)
                            {
                                velocidadeEsquerda = 255;
                                velocidadeDireita = 255;
                            }
                            else if (somaSensores == 2)
                            {
                                velocidadeEsquerda = ((-valorJsumoE) * (velocidade / 4)) + (valorJsumoD * (velocidade / 2)) + ((valorJsumoF) * (velocidade / 2));
                                velocidadeDireita = ((valorJsumoE) * (velocidade / 2)) + ((-valorJsumoD) * (velocidade / 4)) + ((valorJsumoF) * (velocidade / 2));
                            }
                            else if (somaSensores == 1)
                            {
                                velocidadeEsquerda = ((valorJsumoD) * (velocidade / 1.5)) + ((valorJsumoF) * (velocidade / 2));
                                velocidadeDireita = ((valorJsumoE) * (velocidade / 1.5)) + ((valorJsumoF) * (velocidade / 2));
                            }
                            else
                            {
                                velocidadeEsquerda = limiteVelocidade;
                                velocidadeDireita = limiteVelocidade;
                            }
                        }
                        else // Mira guiada desativada
                        {
                            int velocidade = map(r2, zonaMorta, 255, 0, limiteVelocidade);
                            velocidadeEsquerda = velocidade;
                            velocidadeDireita = velocidade;
                        }
                    }
                    else
                    { // Gatilho esquerdo (re)
                        int velocidade = map(l2, zonaMorta, 255, 0, limiteVelocidade);
                        velocidadeEsquerda = -velocidade;
                        velocidadeDireita = -int(coefReverse * velocidade);
                    }

                    //===================== || GATILHOS + CURVA || =============================

                    // Aplica a curva sobre a velocidade existente

                    if (direcao < -zonaMorta)
                    { // Curva pra esquerda em movimento
                        // Reduz a velocidade da roda interna (esquerda para frente, direita para re)
                        if (r2 > zonaMorta)
                            velocidadeEsquerda *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);
                        else
                            velocidadeDireita *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);
                    }
                    else if (direcao > zonaMorta)
                    { // Curva pra direita em movimento
                        // Reduz a velocidade da roda interna (direita para frente, esquerda para re)
                        if (r2 > zonaMorta)
                            velocidadeDireita *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);
                        else
                            velocidadeEsquerda *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);
                    }
                }
            }

            moverMotores(velocidadeEsquerda, velocidadeDireita);
        }

        else
        {
            moverMotores(0, 0); // Para os motores caso a conexão seja perdida
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#pragma endregion

#endif