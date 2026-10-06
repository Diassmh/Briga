//===============================================================================================//
//=====================================//TASK SENSORES//=========================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "defines.hpp"
#include "move.hpp"

#pragma endregion

void vTaskSensores(void *pvParameters)
{
    for (;;)
    {
        if (running_Sensors)
        {
            if (IrReceiver.decode()) // verifica se algum sinal infravermelho foi recebido
            {
                IrReceiver.resume();                            // Volta a analisar o receptor
                if (IrReceiver.decodedIRData.command == botao3) // Se o sinal recebido é 3
                {
                    digitalWrite(led, LOW); // Apaga o led
                    moverMotores(0, 0);     // Para os motores
                    ESP.restart();          // Reinicia o ESP
                }
            }

            valorJsumoE = digitalRead(jSumoE);
            valorJsumoD = digitalRead(jSumoD);

            valorReceptorE = !(digitalRead(receptorE));
            valorReceptorD = !(digitalRead(receptorD));

            valorLinhaE = analogRead(linhaE);
            valorLinhaD = analogRead(linhaD);
            valorLDR = analogRead(pinoLDR);

            if (valorReceptorE || valorReceptorD)
            {
                irVendo = true;
                if (!alvoNoRadar)
                {

                    fill_solid(leds, 5, CRGB::White);
                    FastLED.setBrightness(25);
                    FastLED.show();
                    alvoNoRadar = true;
                }
            }
            else
            {
                irVendo = false;
                if (alvoNoRadar)
                {
                    fill_solid(leds, 5, CRGB::Yellow);
                    FastLED.show();
                    alvoNoRadar = false;
                }
            }

            if (valorJsumoD)
            {
                lado = direita;
            }
            else if (valorJsumoE)
            {
                lado = esquerda;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void iniciaTaskSensores()
{
    xTaskCreatePinnedToCore(vTaskSensores, "SensorTask", 10000, NULL, 1, &SensorTask, 0);
}

#endif