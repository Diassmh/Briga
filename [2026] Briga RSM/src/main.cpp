/*/=========================================================================================================
===========================================================================================================
=====================================     //    BRIGA V1   //    =============================================
==========================================================================================================
========================================================================================================*/

#pragma region Includes

#include "defines.hpp"
#include "modo_auto.hpp"
#include "modo_rc.hpp"
#include "move.hpp"
#include "sensor_task.hpp"

// ======================================|| CONFIG ||=======================================
#pragma region Config

void config()
{
    // --- MOTORES ---
    pinMode(Motor_Esquerdo_Positivo, OUTPUT);
    pinMode(Motor_Esquerdo_Negativo, OUTPUT);
    pinMode(Motor_Direito_Positivo, OUTPUT);
    pinMode(Motor_Direito_Negativo, OUTPUT);

    // --- SENSORES ---
    pinMode(jSumoF, INPUT);
    pinMode(jSumoE, INPUT);
    pinMode(jSumoD, INPUT);
    pinMode(receptorF, INPUT);
    pinMode(receptorE, INPUT);
    pinMode(receptorD, INPUT);
    
    // --- LEDS e EXTRAS ---
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

    pinMode(Pino_Transistor, OUTPUT);
    digitalWrite(Pino_Transistor, HIGH); 

    // --- IR ---
    IrReceiver.begin(IR, DISABLE_LED_FEEDBACK, led);

    // --- SERVO ---
    servoBandeira.setPeriodHertz(SERVO_FREQ_HZ);
    servoBandeira.attach(SERVOMOTOR_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    servoBandeira.write(SERVO_ANGULO_INICIAL);
    //vTaskDelay(pdMS_TO_TICKS(200));
    //servoBandeira.detach();
    
}

#pragma endregion

// ==========================================================
// SETUP PRINCIPAL
// ==========================================================

#pragma region Setup MAIN

void setup()
{
    Serial.begin(115200);
    config();
    iniciaTaskSensores();
}

#pragma endregion

// ==========================================================
// LOOP PRINCIPAL
// ==========================================================

#pragma region Loop MAIN

void loop()
{

    while (modoSelecionado == NaoSelecionado)
    {
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500)     // Piscar do Led
        {
            lastBlink = millis();
            digitalWrite(led, !digitalRead(led)); // Lê o estado, inverte e escreve
        }

        if (IrReceiver.decode())
        {

            IrReceiver.resume();

            if (IrReceiver.decodedIRData.command == botao8) // Botão 8 -> MODO AUTO
            {                            
                digitalWrite(led, HIGH); 
                vTaskDelay(pdMS_TO_TICKS(1500));
                modoSelecionado = ModoAuto;
            }
            else if (IrReceiver.decodedIRData.command == botao9)  // Botão 9 -> MODO RC
            {                           
                digitalWrite(led, HIGH); 
                vTaskDelay(pdMS_TO_TICKS(1500));
                modoSelecionado = ModoRc;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    if (modoSelecionado == ModoAuto)
    {
        setupAuto();
        loopAuto();
    }
    else if (modoSelecionado == ModoRc)
    {
        setupRC();
        loopRC();
    }
}

#pragma endregion