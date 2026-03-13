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
                    moverMotores(0, 0);   // Para os motores
                    ESP.restart();        // Reinicia o ESP
                }
            }

            valorJsumoF = digitalRead(jSumoF);
            valorJsumoE = digitalRead(jSumoE);
            valorJsumoD = digitalRead(jSumoD);
            
            valorReceptorF = !(digitalRead(receptorF));
            valorReceptorE = !(digitalRead(receptorE));
            valorReceptorD = !(digitalRead(receptorD));


            if ((valorJsumoE) || (valorReceptorE))
            {
                lado = esquerda;
            }
            else if ((valorJsumoD) || (valorReceptorD))
            {
                lado = direita;
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