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
int limiteVelocidade = 140;       // Limite de velocidade do motor  160

bool ativarMira = false;

float coefAtenuacao = 1.8; // Atenuacao da velocidade em curvas (deve ser FLOAT)
float coefReverse = 0.9;   // Coeficiente para balancear a re (deve ser FLOAT)
float limiteCurva = 110.0; // Velocidade limite em curvas puras (deve ser FLOAT)  120

const char *BRIGA = "8c:94:df:58:87:0c";
const char *RESSACA = "a4:f0:0f:68:e7:f0";
const char *FUEGO = "d4:e9:f4:a4:9f:dc";
const char *CAIPORA = "b8:d6:1a:5d:fb:b0";
const char *TSUNAMI = "a0:b7:65:58:5d:24";

void iniciarTodosAzuis()
{
    fill_solid(leds, 5, CRGB::Blue);
    FastLED.show();
}

#pragma region Setup RC

void setupRC()
{
    iniciarTodosAzuis();

    if (PS4.begin(BRIGA))
    {
        while (!PS4.isConnected())
        {
            uint8_t brilhoPulsante = beatsin8(30, 40, 120);
            fill_solid(leds, 5, CHSV(160, 255, brilhoPulsante));
            FastLED.show();
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
            TodosLedsAmarelos();
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

            else if (PS4.L1())
            {

                vTaskDelay(pdMS_TO_TICKS(10));
                moverMotores(-255, -255);
                vTaskDelay(pdMS_TO_TICKS(10));
                moverMotores(0, -255);
                vTaskDelay(pdMS_TO_TICKS(370));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            else if (PS4.R1())
            {

                vTaskDelay(pdMS_TO_TICKS(10));
                moverMotores(-255, -255);
                vTaskDelay(pdMS_TO_TICKS(10));
                moverMotores(-255, 0);
                vTaskDelay(pdMS_TO_TICKS(370));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            else if (PS4.Left() > 0)
            {
                moverMotores(-255, 255); // Gira para a esquerda
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(100));
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
                moverMotores(255, -255);       // Gira para a direita
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(100));
                moverMotores(-255, 255); // Gira para a esquerda
                vTaskDelay(pdMS_TO_TICKS(90));
                moverMotores(255, 255); // Anda para frente
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(-255, 255);
                vTaskDelay(pdMS_TO_TICKS(50));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            else if (PS4.Down() > 0)
            {
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }

            else if (PS4.Up() > 0)
            {
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(25));
                moverMotores(255, -255);
                vTaskDelay(pdMS_TO_TICKS(140));
                moverMotores(0, 0);
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
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
                LeituraSensores sensores = copiarLeituraSensores();

                // O antigo modo usava tres JSumos. Agora a mira rapida usa os tres IRs frontais.
                if (sensores.receptorFrente && sensores.receptorEsquerdo && sensores.receptorDireito)
                {
                    moverMotores(255, 255);
                }
                else if (sensores.receptorFrente)
                {
                    moverMotores(127, 127);
                }
                else if (sensores.receptorEsquerdo && !sensores.receptorDireito)
                {
                    lado = esquerda;
                    moverMotores(-80, 80);
                }
                else if (sensores.receptorDireito && !sensores.receptorEsquerdo)
                {
                    lado = direita;
                    moverMotores(80, -80);
                }
                else if (lado == esquerda)
                {
                    moverMotores(-80, 80);
                }
                else
                {
                    moverMotores(80, -80);
                }

                vTaskDelay(pdMS_TO_TICKS(30));
                continue;
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

                    if (direcao < -zonaMorta)
                    { // Curva pra esquerda parado
                        int velCurva = map(abs(direcao), zonaMorta, 128, 0, limiteCurva);
                        velocidadeEsquerda = -velCurva;
                        velocidadeDireita = velCurva;
                    }
                    else if (direcao > zonaMorta)
                    { // Curva pra direita parado
                        int velCurva = map(direcao, zonaMorta, 127, 0, limiteCurva);
                        velocidadeEsquerda = velCurva;
                        velocidadeDireita = -velCurva;
                    }
                }

                //===================== || GATILHOS PUROS || =============================

                else
                { // Pelo menos um gatilho pressionado (R2 ou L2)

                    if (r2 > zonaMorta) // Gatilho direito (frente)
                    {
                        if (ativarMira) // Mira guiada ativa
                        {
                            int velocidade = map(r2, zonaMorta, 255, 0, limiteVelocidade);
                            LeituraSensores sensores = copiarLeituraSensores();

                            // Os IRs frontais corrigem a direcao sem tirar o controle da aceleracao do piloto.
                            if (sensores.receptorFrente && !sensores.receptorEsquerdo && !sensores.receptorDireito)
                            {
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade;
                            }
                            else if (sensores.receptorFrente && sensores.receptorEsquerdo && sensores.receptorDireito)
                            {
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade;
                            }
                            else if (sensores.receptorEsquerdo && sensores.receptorFrente && !sensores.receptorDireito)
                            {
                                velocidadeEsquerda = velocidade / 2;
                                velocidadeDireita = velocidade;
                                lado = esquerda;
                            }
                            else if (sensores.receptorDireito && sensores.receptorFrente && !sensores.receptorEsquerdo)
                            {
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade / 2;
                                lado = direita;
                            }
                            else if (sensores.receptorEsquerdo && !sensores.receptorDireito)
                            {
                                velocidadeEsquerda = velocidade / 4;
                                velocidadeDireita = velocidade;
                                lado = esquerda;
                            }
                            else if (sensores.receptorDireito && !sensores.receptorEsquerdo)
                            {
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade / 4;
                                lado = direita;
                            }
                            else if (sensores.receptorEsquerdo && sensores.receptorDireito)
                            {
                                // Leitura ambigua: segue reto e deixa o piloto decidir a proxima correcao.
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade;
                            }
                            else
                            {
                                velocidadeEsquerda = velocidade;
                                velocidadeDireita = velocidade;
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