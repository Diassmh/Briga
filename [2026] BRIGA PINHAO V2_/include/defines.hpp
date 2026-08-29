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

#define Motor_Direito_Positivo 16  // 16
#define Motor_Direito_Negativo 17  // 17
#define Motor_Esquerdo_Positivo 19 // 19
#define Motor_Esquerdo_Negativo 18 // 18

#pragma endregion

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN 5 // 5
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

#define jSumoD 39
#define jSumoE 36
#define receptorF 4
#define receptorD 27
#define receptorE 23
#define linhaE 34
#define linhaD 35
#define pinoLDR 13

#pragma endregion

//==============================//Ajustes da BuscaCorna//==============================//

#pragma region AJUSTES BUSCA CORNA

// Valores iniciais. Eles ainda precisam ser calibrados com o robo montado.
#define LIMIAR_LINHA_E 2000
#define LIMIAR_LINHA_D 2000

#define VEL_BUSCA 80
#define VEL_GIRO_LATERAL 255
#define VEL_TRANSFERENCIA 100
#define VEL_APROXIMACAO 60
#define VEL_APROXIMACAO_LENTA 30
#define VEL_APROXIMACAO_RAPIDA 70

#define TEMPO_BUSCA_MS 300
#define TEMPO_AVANCO_SEM_ALVO_MS 160
#define TEMPO_PERDA_ALVO_MS 80
#define TEMPO_TRANSFERENCIA_MS 180

#define TEMPO_RECUO_LINHA_MS 150
#define TEMPO_RECUO_DUAS_LINHAS_MS 180
#define TEMPO_GIRO_LINHA_MS 100
#define TEMPO_CONFIRMA_PISO_MS 30
#define TEMPO_MAX_RECUPERACAO_MS 1000

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
  B_Direita
};

enum SelecionaModo
{
  NaoSelecionado,
  ModoAuto,
  ModoRc
};

enum SelecionaBusca
{
  Ofensiva,
  Criminosa,
  ApenasIR,
  Defensiva,
  Corna
};

enum EstadoCriminoso
{
  BUSCA_FURTIVA,      // Jsumo Desligado, Receptores ligados
  GIRO_BORDA_ESQ,     // Borda detectada na esquerda
  GIRO_BORDA_DIR,     // Borda detectada na direita
  BUSCA_ATIVA,        // Liga os Jsumos após detectar alguma borda
  ATAQUE_IR_LENTO,    // Detectou com IR caiu aqui
  ATAQUE_JSUMO_LENTO, // Detectou com Jsumo, caiu aqui
  ATAQUE_TOTAL        // Está perto / Rampou, caiu aqui
};

enum EstadoDefensivo
{
  BUSCA_GIRANDO,
  ATAQUE_IR_LENTOD,
  ATAQUE_JSUMO_LENTOD
};

enum EstadoRobo
{
  BUSCAR,
  ALINHAR_FRENTE,
  ESPERAR_ALVO,
  AVANCAR_LENTO,
  VIRAR_LATERAL_E,
  VIRAR_LATERAL_D,
  TRANSFERIR_LATERAL_E,
  TRANSFERIR_LATERAL_D,
  RECUPERAR_LINHA,
  ATAQUE_RAPIDO
};

// Etapas separadas deixam a fuga da linha interrompivel, sem usar delay().
enum EtapaRecuperacaoLinha
{
  RECUANDO_DA_LINHA,
  GIRANDO_PARA_DENTRO,
  CONFIRMANDO_PISO
};

enum LinhaDetectada
{
  NENHUMA_LINHA,
  SOMENTE_LINHA_E,
  SOMENTE_LINHA_D,
  DUAS_LINHAS
};

#pragma endregion

//===================================//Variaveis utilizadas//====================================//

#pragma region VARIAVEIS

Direction lado = esquerda;
Bandeira lado_Bandeira = B_Direita;
SelecionaModo modoSelecionado = NaoSelecionado;
SelecionaBusca ModoBusca = Corna; // Busca nova fica como padrao, mas a ofensiva continua disponivel
EstadoCriminoso EstadoC = BUSCA_FURTIVA;
EstadoDefensivo EstadoD = BUSCA_GIRANDO;
EstadoRobo estadoAtual = BUSCAR;
EstadoRobo estadoAntesDaLinha = BUSCAR; // Permite retomar a acao se o IR central voltar a detectar

volatile bool valorJsumoF, valorJsumoE, valorJsumoD;
volatile bool valorReceptorF, valorReceptorE, valorReceptorD;
volatile int valorLinhaE, valorLinhaD;
volatile int valorLDR;
bool medirLDR = true;
int ldrReferencia = 0;
long somaLDR = 0;

CRGB leds[5];

char BT = 'A';         // char que é recebida pelo bluetooth
char estrategia = 'z'; // Sem abertura por padrao enquanto a busca nova e a linha sao testadas
bool running_Sensors = false;
bool atacando = false;
bool flagPrimeiraVez = true; // Busca IR
unsigned long tempoInicioIR, tempoIncremento;
unsigned long inicioDoAtaque = 0;
unsigned long inicioDefensivo = 0;
int num_led = 1;

// Variáveis Busca Defensiva
unsigned long tempoCicloJsumo = 0;
bool jsumoEmPulso = false;
unsigned long tempoAlinhamento = 0;
bool alvoAlinhado = false;

// BuscaCorna
unsigned long tempoBusca = 0;
unsigned long tempoTravamento = 0;
unsigned long tempoTransferencia = 0;
unsigned long tempoAvancoLento = 0;
unsigned long ultimaDeteccaoFrontal = 0;
bool avancoSemAlvo = false;

EtapaRecuperacaoLinha etapaRecuperacaoLinha = RECUANDO_DA_LINHA;
LinhaDetectada linhaQueAtivou = NENHUMA_LINHA;
unsigned long tempoEtapaLinha = 0;
unsigned long inicioRecuperacaoLinha = 0;
unsigned long inicioPisoSeguro = 0;
bool recuperarParaDireita = true;
bool falhaRecuperacaoLinha = false;

// A task escreve todos os sensores dentro da mesma secao critica.
// A busca copia essa fotografia antes de decidir o movimento.
struct LeituraSensores
{
  bool jsumoEsquerdo;
  bool jsumoDireito;
  bool receptorFrente;
  bool receptorEsquerdo;
  bool receptorDireito;
  int linhaEsquerda;
  int linhaDireita;
  int ldr;
};

portMUX_TYPE muxSensores = portMUX_INITIALIZER_UNLOCKED;

LeituraSensores copiarLeituraSensores()
{
  LeituraSensores leitura;

  portENTER_CRITICAL(&muxSensores);
  leitura.jsumoEsquerdo = valorJsumoE;
  leitura.jsumoDireito = valorJsumoD;
  leitura.receptorFrente = valorReceptorF;
  leitura.receptorEsquerdo = valorReceptorE;
  leitura.receptorDireito = valorReceptorD;
  leitura.linhaEsquerda = valorLinhaE;
  leitura.linhaDireita = valorLinhaD;
  leitura.ldr = valorLDR;
  portEXIT_CRITICAL(&muxSensores);

  return leitura;
}

#pragma endregion

#endif