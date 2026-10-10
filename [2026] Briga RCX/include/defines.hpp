//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <IRremote.hpp>
#include <ESP32Servo.h>
#include <FastLED.h>

#pragma endregion

//=====================================//Objetos e Handles//=====================================//

#pragma region OBJETOS E HANDLES

BluetoothSerial SerialBT;
Servo servoBandeira;

TaskHandle_t SensorTask; // Avisando o ESP que teremos uma task

//=========================================//Bluetooth//=========================================//

// Verifica se o Bluetooth esta devidamente habilitado no menuconfig

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif // Finaliza o condicional do erro

#pragma endregion

//===============================================================================================//
//=====================================//PINOS E VARIAVEIS//=====================================//
//===============================================================================================//

//==========================================//Motores//==========================================//

#pragma region PINOS MOTORES

#define Motor_Direito_Positivo 16  // 17    arruela 19
#define Motor_Direito_Negativo 17  // 16    arruela 18
#define Motor_Esquerdo_Positivo 18 // 19    arruela 16
#define Motor_Esquerdo_Negativo 19 // 18   arruela 17

#pragma endregion

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN 26 // 26
#define SERVO_ANGULO_INICIAL 90
#define SERVO_FREQ_HZ 50
#define SERVO_RESOLUTION 16
#define SERVO_MIN_PULSE_US 544  //  Largura de pulso para 0 graus (µs)
#define SERVO_MAX_PULSE_US 2400 //  Largura de pulso para 180 graus (µs)

#pragma endregion

//========================================//Comunicacao//========================================//

#pragma region COMUNICACAO

#define leds_end 33
#define led 2
#define IR 13

#define botao1 0x0
#define botao2 0x1
#define botao3 0x2 // Botões do controle remoto
#define botao8 0x7
#define botao9 0x8

#pragma endregion

//=======================================//Sensoreamento//=======================================//

#pragma region SENSOREAMENTO

#define jSumoD 27
#define jSumoE 23
#define receptorD 4
#define receptorE 5
#define linhaE 34
#define linhaD 36
#define pinoLDR 39

#pragma endregion

//======================================//Enums//======================================//

#pragma region ENUMS

enum Direction
{
  esquerda,
  direita,
  frente
};

enum Bandeira
{
  B_Esquerda,
  B_Direita,
  Sem_Bandeira
};

enum SelecionaModo
{
  NaoSelecionado,
  ModoAuto,
  ModoRc
};

enum SelecionaBusca
{
  Corna,
  CornaDefensiva
};

enum Posicionamento
{
  Perto,
  Medio,
  Longe
};

enum EstadoRobo
{
  BUSCAR,
  ALINHAR_FRENTE,
  ESPERAR_ALVO,
  AVANCAR_LENTO,
  VIRAR_LATERAL_E,
  VIRAR_LATERAL_D,
  ATAQUE_RAPIDO
};

#pragma endregion

//===================================//Variaveis utilizadas//====================================//

#pragma region VARIAVEIS

Direction lado = esquerda;
Bandeira lado_Bandeira = B_Direita;
SelecionaModo modoSelecionado = NaoSelecionado;
Posicionamento Posicao = Medio;
SelecionaBusca ModoBusca = Corna;
EstadoRobo estadoAtual = AVANCAR_LENTO;

volatile bool valorJsumoE, valorJsumoD;
volatile bool valorReceptorE, valorReceptorD;

int valorLinhaE, valorLinhaD;
int valorLDR;

CRGB leds[5];

char BT = 'A';         // char que é recebida pelo bluetooth
char estrategia = 'A'; // estratégia de início da luta (qualquer caractere)
bool running_Sensors = false;
int num_led = 1;

// BuscaCorna
unsigned long tempoBusca = 0;
unsigned long tempoTravamento = 0;
bool irVendo = 0;
bool alvoNoRadar = false;

#pragma endregion

#endif
