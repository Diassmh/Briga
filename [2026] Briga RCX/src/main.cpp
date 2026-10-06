/*/=========================================================================================================
===========================================================================================================
=====================================     //    BRIGA RSM   //    =============================================
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

void animacaoInicial()
{

    fill_solid(leds, 5, CRGB::Red);
    FastLED.show();
    delay(750);

    fill_solid(leds, 5, CRGB::Black);
    FastLED.show();
    delay(500);

    for (int i = 0; i < 5; i++)
    {
        delay(500);
        leds[i] = CRGB::Blue; // leds[i] = CRGB::Orange; // Purple
        FastLED.show();
    }

    delay(300);
    fill_solid(leds, 5, CRGB::Black);
    FastLED.show();
    delay(300);
}

void config()
{
    // --- MOTORES ---
    pinMode(Motor_Esquerdo_Positivo, OUTPUT);
    pinMode(Motor_Esquerdo_Negativo, OUTPUT);
    pinMode(Motor_Direito_Positivo, OUTPUT);
    pinMode(Motor_Direito_Negativo, OUTPUT);

    // --- SENSORES ---
    pinMode(jSumoE, INPUT);
    pinMode(jSumoD, INPUT);
    pinMode(receptorE, INPUT);
    pinMode(receptorD, INPUT);
    pinMode(linhaE, INPUT);
    pinMode(linhaD, INPUT);
    pinMode(pinoLDR, INPUT);

    // --- LEDS e EXTRAS ---
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

    FastLED.addLeds<WS2812, leds_end, RGB>(leds, 5);
    FastLED.setBrightness(100); // Brilho

    animacaoInicial();

    // --- IR ---
    IrReceiver.begin(IR, DISABLE_LED_FEEDBACK, led);

    // --- SERVO ---
    servoBandeira.setPeriodHertz(SERVO_FREQ_HZ);
    servoBandeira.attach(SERVOMOTOR_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    servoBandeira.write(SERVO_ANGULO_INICIAL);
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

        uint8_t brilhoPulsante = beatsin8(25, 50, 250);      // 30,40,120
        fill_solid(leds, 5, CHSV(210, 255, brilhoPulsante)); // roxo:210 vermelho: 0
        FastLED.show();

        if (IrReceiver.decode())
        {

            IrReceiver.resume();

            if (IrReceiver.decodedIRData.command == botao8) // Botão 8 -> MODO AUTO
            {
                digitalWrite(led, HIGH);
                LedFicaVerde(0);
                LedFicaVerde(1);
                vTaskDelay(pdMS_TO_TICKS(1500));
                modoSelecionado = ModoAuto;
            }
            else if (IrReceiver.decodedIRData.command == botao9) // Botão 9 -> MODO RC
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
