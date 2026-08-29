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

            // Primeiro fazemos todas as leituras em variaveis locais.
            // Depois publicamos tudo junto para a busca nao misturar dois momentos diferentes.
            bool novoJsumoE = digitalRead(jSumoE);
            bool novoJsumoD = digitalRead(jSumoD);

            bool novoReceptorF = !(digitalRead(receptorF));
            bool novoReceptorE = !(digitalRead(receptorE));
            bool novoReceptorD = !(digitalRead(receptorD));

            int novaLinhaE = analogRead(linhaE);
            int novaLinhaD = analogRead(linhaD);
            int novoLDR = analogRead(pinoLDR);

            portENTER_CRITICAL(&muxSensores);
            valorJsumoE = novoJsumoE;
            valorJsumoD = novoJsumoD;

            valorReceptorF = novoReceptorF;
            valorReceptorE = novoReceptorE;
            valorReceptorD = novoReceptorD;

            valorLinhaE = novaLinhaE;
            valorLinhaD = novaLinhaD;
            valorLDR = novoLDR;
            portEXIT_CRITICAL(&muxSensores);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void iniciaTaskSensores()
{
    xTaskCreatePinnedToCore(vTaskSensores, "SensorTask", 10000, NULL, 1, &SensorTask, 0);
}

#endif
