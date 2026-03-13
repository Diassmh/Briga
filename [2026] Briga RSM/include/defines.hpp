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

#define Motor_Direito_Positivo     19      //19
#define Motor_Direito_Negativo     18      //18
#define Motor_Esquerdo_Positivo    17      //17    
#define Motor_Esquerdo_Negativo    16      //16

#pragma endregion

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN 26
#define SERVO_ANGULO_INICIAL 90
#define SERVO_FREQ_HZ 50
#define SERVO_RESOLUTION 16
#define SERVO_MIN_PULSE_US 544  //  Largura de pulso para 0 graus (µs)
#define SERVO_MAX_PULSE_US 2400 //  Largura de pulso para 180 graus (µs)

#pragma endregion

//========================================//Comunicacao//========================================//

#pragma region COMUNICACAO

#define led 2
#define IR 13

#define botao1 0x0
#define botao2 0x1
#define botao3 0x2      // Botões do controle remoto
#define botao8 0x7
#define botao9 0x8

#pragma endregion

//=======================================//Sensoreamento//=======================================//

#pragma region SENSOREAMENTO

#define Pino_Transistor 25 //  AUTO | Transistor que desliga

#define jSumoF 39
#define jSumoD 36
#define jSumoE 34
#define receptorF 27    // 27
#define receptorD 23    //23
#define receptorE 4    //4


#pragma endregion

//======================================//Enums//======================================//

#pragma region ENUMS

enum Direction {esquerda, direita, frente};

enum Bandeira {B_Esquerda, B_Direita};

enum Transistor{Ligado, Desligado};

enum SelecionaModo {NaoSelecionado, ModoAuto, ModoRc};

enum SelecionaBusca {Ofensiva, Criminosa, ApenasIR};  

enum EstadoCriminoso {
  BUSCA_FURTIVA,            // Jsumo Desligado, Receptores ligados
  GIRO_BORDA_ESQ,           // Borda detectada na esquerda
  GIRO_BORDA_DIR,           // Borda detectada na direita
  BUSCA_ATIVA,              // Liga os Jsumos após detectar alguma borda
  ATAQUE_IR_LENTO,          // Detectou com IR caiu aqui
  ATAQUE_JSUMO_LENTO,       // Detectou com Jsumo, caiu aqui
  ATAQUE_TOTAL              // Está perto / Rampou, caiu aqui
};
#pragma endregion

//===================================//Variaveis utilizadas//====================================//

#pragma region VARIAVEIS

Direction        lado            = esquerda;
Bandeira         lado_Bandeira   = B_Direita;
Transistor       transistor      = Ligado;
SelecionaModo    modoSelecionado = NaoSelecionado;
SelecionaBusca   ModoBusca       = Ofensiva;
EstadoCriminoso  Estado          = BUSCA_FURTIVA; 

volatile bool valorJsumoF, valorJsumoE, valorJsumoD;
volatile bool valorReceptorF, valorReceptorE, valorReceptorD;

char BT = 'A';          // char que é recebida pelo bluetooth
char estrategia = 'A';  // estratégia de início da luta (qualquer caractere)
bool running_Sensors = false;  
bool atacando = false;
bool flagPrimeiraVez = true; // Busca IR
unsigned long tempoInicioIR, tempoIncremento; 
unsigned long inicioDoAtaque = 0;
unsigned long inicioDefensivo = 0;

#pragma endregion


#endif