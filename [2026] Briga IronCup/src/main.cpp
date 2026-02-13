/*/=========================================================================================================
===========================================================================================================
=====================================     //    BRIGA V1   //    =============================================
==========================================================================================================
========================================================================================================*/

#pragma region Includes e Portas

//========================================//Defines e Includes//========================================//

#include <BluetoothSerial.h>
#include <Arduino.h>
#include <IRremote.hpp>
#include <ESP32Servo.h>
#include <PS4Controller.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif

#define SensorF 4
#define SensorD 27 // Pinos Sensores
#define SensorE 23

#define modoAguarda 100
#define modoAUTO 101 // modos
#define modoRC 102

#define botao1 0x0
#define botao2 0x1
#define botao3 0x2 // Botões do controle remoto
#define botao8 0x7
#define botao9 0x8

#define SENSOR_POWER 25 //  AUTO | Transistor que desliga

#define Motor_Direito_Positivo 16
#define Motor_Direito_Negativo 17
#define Motor_Esquerdo_Positivo 18
#define Motor_Esquerdo_Negativo 19

#define led 2 // Pino do Led
#define IR 13 // Pino IR

#define coefAtenuacao 1.8 // Atenuacao da velocidade em curvas (deve ser FLOAT)
#define coefReverse 0.9   // Coeficiente para balancear a re (deve ser FLOAT)
#define limiteCurva 50.0  // Velocidade limite em curvas puras (deve ser FLOAT)

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN 32 //  DOIS | Servomotor da Bandeira ESP32 porta 32

#define SERVO_LEDC_CHANNEL LEDC_CHANNEL_0 //  DOIS | Canal do PWM do servo
#define SERVO_TIMER LEDC_TIMER_0          //  DOIS | Timer do PWM do servo

#define SERVO_ANGULO_INICIAL 90
#define SERVO_FREQ_HZ 50        //  DOIS | Frequencia padrao do MG90S
#define SERVO_RESOLUTION 16     //  DOIS | Resolucao de 16 bits para o canal
#define SERVO_MIN_PULSE_US 544  //  DOIS | Largura de pulso para 0 graus (µs)
#define SERVO_MAX_PULSE_US 2400 //  DOIS | Largura de pulso para 180 graus (µs)

#pragma endregion

//========================================//Globais//========================================//

#pragma region Variaveis Globais

int modoSelecionado = modoAguarda;
// int modoSelecionado = modoRC;

BluetoothSerial SerialBT;
Servo servoBandeira;

char BT = 'a';         // char que é recebida pelo bluetooth
char estrategia = 'a'; // estratégia de início da luta (qualquer caractere)
bool running = false;  // Flag para iniciar a estratégia

const char *BRIGA = "7c:9e:bd:fb:83:80";
const char *VAMPETA = "f4:2d:c9:5a:14:38";

#pragma endregion

//======================================// Variaveis modo AUTO//========================================//

#pragma region Variaveis AUTO

volatile bool valorSensorF, valorSensorE, valorSensorD; // Variáveis que vão armazenar o que os sensores enxergam

TaskHandle_t SensorTask; // Avisando o ESP que teremos uma task

unsigned long inicioDoAtaque = 0;
unsigned long inicioDefensivo = 0;
bool detectandoFrente = false;
bool atacando = false;
bool primeiraVez = true;

#pragma endregion

//========================================// Variaveis modo RC //========================================//
#pragma region Variaveis RC

int zonaMorta = 20;               // Zona desconsiderada do controle de PS4
volatile int r2 = 0;              // Valor do gatilho direito do PS4 (R2)
volatile int l2 = 0;              // Valor do gatilho esquerdo do PS4 (L2)
volatile int direcao = 0;         // Valor do direcional esquerdo do PS4 em X (LStickX)
volatile bool velLimitada = true; // Switch definido pra limitar a velocidade do motor
int limiteVelocidade = 190;       // Limite de velocidade do motor
int velBase = 80;
bool ativarMira = true;

#pragma endregion

// ===============================|| FUNÇÕES DO MODO AUTÔNOMO ||===========================

#pragma region Funções AUTO

enum Direction // define o lado do movimento inicial (esquerda ou direita)
{
    left,
    right,
    front
};
volatile Direction lado = left;

enum Bandeira
{
    B_Esq,
    B_Dir
};
Bandeira Lado_Bandeira = B_Esq;

void moverMotores(int velEsq, int velDir)
{
    // Limpar o PWM da ultima velocidade enviada
    analogWrite(Motor_Direito_Positivo, LOW);
    analogWrite(Motor_Direito_Negativo, LOW);
    analogWrite(Motor_Esquerdo_Positivo, LOW);
    analogWrite(Motor_Esquerdo_Negativo, LOW);
    delayMicroseconds(60);
    // 60us -> 2x tempo necessario definido no datasheet da ponte H

    if (velDir >= 0)
    {                                                     // Se positivo
        analogWrite(Motor_Direito_Positivo, abs(velDir)); // Anda para frente (pino positivo)
        analogWrite(Motor_Direito_Negativo, LOW);         // Desliga o pino negativo
    }
    else
    {                                                     // Se negativo
        analogWrite(Motor_Direito_Negativo, abs(velDir)); // Anda para tras
        analogWrite(Motor_Direito_Positivo, LOW);
    }

    if (velEsq >= 0)
    {                                                      // Se positivo
        analogWrite(Motor_Esquerdo_Positivo, abs(velEsq)); // Anda para frente
        analogWrite(Motor_Esquerdo_Negativo, LOW);
    }
    else
    {                                                      // Se negativo
        analogWrite(Motor_Esquerdo_Negativo, abs(velEsq)); // Anda para tras
        analogWrite(Motor_Esquerdo_Positivo, LOW);
    }
}

void mostrarMenu()
{
    SerialBT.println("\n========== MENU BRIGA ==========");
    SerialBT.println("--- ATAQUE ---");
    SerialBT.println(" f : Frentao Progressivo");
    SerialBT.println(" F : Frentao BOLADO (Anti-Pescador)");
    SerialBT.println(" i : Frentinho (1/4 Dohyo)");
    SerialBT.println(" b : Costas");
    SerialBT.println(" c : Curva Leve");
    SerialBT.println(" C : Curva Pesada");
    SerialBT.println(" e : Desempate ");
    SerialBT.println(" B : Bait");
    SerialBT.println(" v : Movimentacao em V");

    SerialBT.println("\n--- DEFESA ---");
    SerialBT.println(" d : Defensivo Simples");
    SerialBT.println(" D : Defensivo Posicionado");
    SerialBT.println("A : Aquele La");

    SerialBT.println("\n--- CONFIGURACAO ---");
    SerialBT.println(" < / > : Define Lado (Esq/Dir)");
    SerialBT.println(" =     : Define Lado Frente");
    SerialBT.println(" l / r : Bandeira (Left/Right)");
    SerialBT.println(" Q     : Checar Status Atual");
    SerialBT.println(" S     : Debug Sensores (Reset p/ sair)");
    SerialBT.println(" h     : MOSTRAR ESSE MENU");
    SerialBT.println("======================================\n");
}

#pragma region Task sensor

void FunctionSensorTask(void *pvParameters)
{

    while (!running) // Enquanto a flag running = 0 (robô aguardando comandos)
    {
        vTaskDelay(5); // Aguarda
    }

    /* if (Lado_Bandeira == B_Esq)
         {
             servoBandeira.write(0); // Cai para esquerda
             delay(100);
             servoBandeira.detach();
         }
     else if (Lado_Bandeira == B_Dir)
         {
             servoBandeira.write(180); // Cai para direita
             delay(100);
             servoBandeira.detach();
         }
     */
    for (;;) // Se o running = true  (loop infinito para checar os jsumos e o receptor)
    {

        if (IrReceiver.decode()) // verifica se algum sinal infravermelho foi recebido
        {
            IrReceiver.resume();                            // Volta a analisar o receptor
            if (IrReceiver.decodedIRData.command == botao3) // Se o sinal recebido é 3
            {
                digitalWrite(led, 0); // Apaga o led
                moverMotores(0, 0);   // Para os motores
                ESP.restart();        // Reinicia o ESP
            }
        }

        valorSensorF = digitalRead(SensorF);
        valorSensorD = digitalRead(SensorD); // Lê os sensores e armazena nas variáveis 0 ou 1
        valorSensorE = digitalRead(SensorE);

        if (valorSensorE == HIGH)
        {
            lado = left; // Guarda o ultimo lado que o adversário estava
        }
        else if (valorSensorD == HIGH)
        {
            lado = right; // Guarda o ultimo lado que o adversário estava
        }
        vTaskDelay(10); // espera 10ms para não sobrecarregar o ESP (vamos testar esse tempo)
    }
}

#pragma endregion

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
    pinMode(SensorF, INPUT);
    pinMode(SensorE, INPUT);
    pinMode(SensorD, INPUT);

    // --- LEDS e EXTRAS ---
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

    pinMode(SENSOR_POWER, OUTPUT);
    digitalWrite(SENSOR_POWER, HIGH); // Transistor HIGH

    // --- IR ---
    IrReceiver.begin(IR, DISABLE_LED_FEEDBACK, led);

    // --- SERVO ---
    servoBandeira.setPeriodHertz(SERVO_FREQ_HZ);
    servoBandeira.attach(SERVOMOTOR_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    servoBandeira.write(SERVO_ANGULO_INICIAL);
    delay(200);
}

#pragma endregion

// ==========================================================
// SETUP + LOOP   || MODO AUTO ||
// ==========================================================

#pragma region Setup AUTO

void setupAuto()
{
    // Inicia Bluetooth do Celular (SerialBT):
    for (int i = 0; i < 4; i++) // Inicialização
    {
        digitalWrite(led, 0);
        delay(300);
        digitalWrite(led, 1);
        delay(300);
    }

    SerialBT.begin("Briga");

    // Task dos sensores é chamada somente no modo AUTO:
    xTaskCreatePinnedToCore(FunctionSensorTask, "SensorTask", 10000, NULL, 1, &SensorTask, 0);

    moverMotores(0, 0); // Garante que os motores estão desligados

    running = false; // Task dos sensores permanece desativada
    digitalWrite(led, 0);
    while (true)
    {

        if (SerialBT.connected() && primeiraVez)
        {
            SerialBT.println("CONECTADO");
            mostrarMenu();
            digitalWrite(led, 1);
            primeiraVez = false;
        }
        vTaskDelay(1);

        if (SerialBT.available())
        {
            BT = SerialBT.read();
            SerialBT.println(BT);
        }

        // DIREÇÃO DE ESTRATEGIAS ---------------------------

        if (BT == '<')
        {
            lado = left;
        }
        else if (BT == '>')
        {
            lado = right;
        }
        else if (BT == '=')
        {
            lado = front;
        }

        // LADO QUE A BANDEIRA VAI CAIR -----------------------

        else if (BT == 'l')
        {
            Lado_Bandeira = B_Esq;
        }
        else if (BT == 'r')
        {
            Lado_Bandeira = B_Dir;
        }

        else if (BT == 'Q')
        { // Char 'Q' checa as estrategias
            SerialBT.println("Check");
            SerialBT.print("Estrategia: ");
            SerialBT.println(estrategia);
            SerialBT.print("Lado: ");
            if (lado == right)
                SerialBT.println("direita");
            else
                SerialBT.println("esquerda");
            SerialBT.print("Bandeira: ");
            if (Lado_Bandeira == B_Esq)
                SerialBT.println("Esquerda");
            else
                SerialBT.println("Direita");
        }
        else if (BT == 'S')
        {
            running = true;
            for (;;)
            {
                SerialBT.printf("ESFREN: %d\nFRENTE: %d\nDIRFRE: %d\n", valorSensorE, valorSensorF, valorSensorD);
                delay(50);
            }
        }
        else
        {
            if (!(BT == 13 || BT == 10 || (BT > 48 && BT < 58))) // Filtro para não recebermos ENTER
            {                                                    // Se não for ENTER ou numero altera a estrategia inicial
                estrategia = BT;
            }
        }

        if (IrReceiver.decode()) // No final do Loop checamos o receptor
        {
            IrReceiver.resume();
            if (IrReceiver.decodedIRData.command == botao1) // Se recebermos um 1:
            {
                for (int i = 0; i < 3; i++) // Entramos no loop
                {

                    digitalWrite(led, 0);
                    delay(100); // Piscamos o LED
                    digitalWrite(led, 1);
                    delay(100);
                    if (IrReceiver.decode()) // Se o juiz apertar o 1 e logo depois o 2, identificamos rapidamente aqui
                    {
                        IrReceiver.resume();
                        if (IrReceiver.decodedIRData.command == botao2) // se Recebermos 2
                        {
                            running = true;
                            digitalWrite(led, 1);
                            break;
                        }
                    }
                }
            }

            if (IrReceiver.decodedIRData.command == botao2) // se Recebermos 2
            {
                digitalWrite(led, 1);
                running = true;
                break;
            }
        }
    }

    // COMEÇO DE LUTA --------------------------------------------

    SerialBT.println("BORAAAAAAAAA");

    // Queda da Bandeira:
    if (estrategia != 'D')
    { // Se a estratégia for diferente do Defensivo Posicionado:

        if (Lado_Bandeira == B_Esq)
        {
            servoBandeira.write(180); // Cai para esquerda
        }
        else if (Lado_Bandeira == B_Dir)
        {
            // Cai para direita
            servoBandeira.write(0);
        }
    }

    // Execução de movimentação inicial:
    switch (estrategia)
    {
    case 'f': // frentão Progressivo   1/2 dohyo
        SerialBT.print("FRENTAO PROGRESSIVO");
        moverMotores(48, 48);
        delay(4);
        moverMotores(60, 60);
        delay(4);
        moverMotores(72, 72);
        delay(4);
        moverMotores(84, 84);
        delay(4);
        moverMotores(96, 96);
        delay(4);
        moverMotores(128, 128);
        delay(8);
        moverMotores(160, 160);
        delay(8);
        moverMotores(196, 196);
        delay(8);
        moverMotores(228, 228);
        delay(12);
        moverMotores(255, 255);
        delay(54);
        break;

    case 'F': // Frentão Anti Pescador 1/2 dohyo

        SerialBT.print("FRENTAO BOLADO");
        moverMotores(255, 255);
        delay(100);
        break;

    case 'i': // Frentinho 1/4 do dohyo

        SerialBT.print("FRENTINHO");
        moverMotores(127, 127);
        delay(200);
        break;

    case 'b': // Costas

        SerialBT.print("COSTAS");
        if (lado == front)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(127, 127);
            delay(100);
            break;
        }
        else if (lado == left)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(-255, 255);
            delay(50);
            moverMotores(255, 255);
            delay(100);
            moverMotores(0, 0);
            delay(10);
            lado = right;
            break;
        }
        else if (lado == right)
        {
            moverMotores(-255, -255);
            delay(120);
            moverMotores(255, -255);
            delay(50);
            moverMotores(255, 255);
            delay(100);
            moverMotores(0, 0);
            delay(10);
            lado = left;
            break;
        }

    case 'c': // Curva Leve

        SerialBT.print("CURVA LEVE");

        if (lado == left)
        {
            moverMotores(-255, 255);
            delay(40);
            moverMotores(255, 255);
            delay(100);
            moverMotores(0, 0);
            delay(10);
            lado = right;
            break;
        }
        else if (lado == right)
        {
            moverMotores(255, -255);
            delay(40);
            moverMotores(255, 255);
            delay(100);
            moverMotores(0, 0);
            delay(10);
            lado = left;
            break;
        }

    case 'C': // Curva Pesada

        SerialBT.print("CURVA PESADA");

        if (lado == left)
        {
            moverMotores(-255, 255);
            delay(55);
            moverMotores(255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            lado = right;
            break;
        }
        else if (lado == right)
        {
            moverMotores(255, -255);
            delay(55);
            moverMotores(255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            lado = left;
            break;
        }
    case 'd': // Defensivo

        SerialBT.print("Defensivo");
        while (true)
        {
            if (valorSensorF == HIGH && valorSensorE == HIGH && valorSensorD == HIGH)
            {
                break;
            }
            else if (valorSensorF == HIGH)
            {
                moverMotores(30, 30);
            }
            else
            {
                if (lado == left)
                    moverMotores(-70, 70);
                else if (lado == right)
                    moverMotores(70, -70);
            }
            vTaskDelay(5);
        }
        break;
    case 'D': // Defensivo Posicionado

        SerialBT.print("Defensivo Posicionado");

        while (true)
        {

            if (valorSensorF == HIGH)
            {
                if (!detectandoFrente)
                {
                    inicioDefensivo = millis(); // Marca o tempo em que o adversário começou a ser visto com o sensorF
                    detectandoFrente = true;
                }

                if (millis() - inicioDefensivo > 350)
                { // Se o inimigo já está na frente a mais de 350ms...
                    break;
                }
                else
                {
                    moverMotores(0, 0); // Garante que está parado
                }
            }

            else if ((valorSensorE == HIGH) && (valorSensorD == LOW) && (valorSensorF == LOW))
            {
                servoBandeira.write(0);
                delay(50);
                moverMotores(255, -255);
                delay(50);
                moverMotores(255, 255);
                delay(120);
                // TESTAR PARA VER SE PRECISAMOS FORÇAR O GIRO NO PROPRIO EIXO NA DIREÇÃO DO ADVERSARIO
                break;
            }
            else if ((valorSensorD == HIGH) && (valorSensorE == LOW) && (valorSensorF == LOW))
            {
                servoBandeira.write(180);
                delay(50);
                moverMotores(-255, 255);
                delay(50);
                moverMotores(255, 255);
                delay(120);
                // TESTAR PARA VER SE PRECISAMOS FORÇAR O GIRO NO PROPRIO EIXO NA DIREÇÃO DO ADVERSARIO
                break;
            }
        }
        break;
    case 'e': // desempate
        if (lado == left)
        {
            moverMotores(-255, 255);
            delay(100);
            break;
        }
        else if (lado == right)
        {
            moverMotores(255, -255);
            delay(100);
            break;
        }
    case 't': // Código para testar na hora

        break;
    case 'v':
        if (lado == left)
        {
            moverMotores(-255, 255);
            delay(55);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, -255);
            delay(110);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(180);
            lado = right;
            break;
        }
        else if (lado == right)
        {
            moverMotores(255, -255);
            delay(55);
            moverMotores(255, 255);
            delay(160);
            moverMotores(0, 0);
            delay(10);
            moverMotores(-255, 255);
            delay(110);
            moverMotores(0, 0);
            delay(10);
            moverMotores(255, 255);
            delay(180);
            lado = left;
            break;
        }
    case 'A': // Aquele lá
        bool flagM = false;
        bool giroEsquerda = false, giroDireita = false;

        delay(80);
        while (!flagM)
        {

            if (lado == left)
            {
                moverMotores(255, -255);
                delay(75);
                giroEsquerda = true;
                giroDireita = false;
                flagM = true;
            }
            else if (lado == right)
            {
                moverMotores(-255, 255);
                delay(75);
                giroDireita = true;
                giroEsquerda = false;
                flagM = true;
            }
        }
        if (giroDireita)
        {
            moverMotores(255, 255);
            delay(90);
            moverMotores(0, 0);
            delay(20);
            moverMotores(255, -255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            break;
        }
        else if (giroEsquerda)
        {
            moverMotores(255, 255);
            delay(90);
            moverMotores(0, 0);
            delay(20);
            moverMotores(-255, 255);
            delay(150);
            moverMotores(0, 0);
            delay(10);
            break;
        }
    }
}

#pragma endregion

#pragma region Loop AUTO

void loopAuto()
{
    while (true)
    {
        {
            // Em caso de testes:
            // SerialBT.printf("ESFREN: %d\nFEN: %d\nDIFRE: %d\n", valueSensorE, valueSensorF, valueSensorD);
            if (valorSensorF == HIGH)
            {
                if (!atacando)
                {
                    inicioDoAtaque = millis();
                    atacando = true;
                }

                // Cálculo do tempo que passou desde que começamos a ver o adversário com o sensor do meio
                unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

                if (tempoDeAtaque < 100) // Tempo da "aproximação lenta" (em ms)
                {
                    moverMotores(127, 127); //  Velocidade de aproximação
                }
                else // Se o sensorF esta vendo por mais de 200 ms:
                {
                    moverMotores(255, 255); // Potencia máxima
                }
            }
            else
            {
                // Se perdi o oponente, aviso que não estou mais atacando.
                // Assim, na próxima vez que eu ver ele, a lógica "lenta" reinicia.
                atacando = false;

                if (lado == right)
                {
                    moverMotores(100, -100);
                }
                else if (lado == left)
                {
                    moverMotores(-100, 100);
                }
            }
            vTaskDelay(10); // Pequeno delay pro ESP não fritar
        }
    }
}
#pragma endregion

// ==========================================================
// SETUP + LOOP == MODO RC ==
// ==========================================================

#pragma region Setup RC

void setupRC()
{
    xTaskCreatePinnedToCore(FunctionSensorTask, "SensorTask", 10000, NULL, 1, &SensorTask, 0);
    running = true;

    if (PS4.begin(BRIGA))
    { // MAC Adress definido no inicio do código
        Serial.println("Bluetooth inicializado, aguardando controle...");
        while (!PS4.isConnected())
        {
            Serial.println("Tentando Conectar");
            digitalWrite(led, LOW);
            vTaskDelay(5);
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
                delay(50);
                continue;
            }

            else if (PS4.Left() > 0)
            {

                moverMotores(-255, 255); // Gira para a esquerda
                delay(50);
                moverMotores(255, 255); // Anda para frente
                delay(150);
                moverMotores(255, -255); // Gira para a direita
                delay(90);
                moverMotores(255, 255); // Anda para frente
                delay(50);
                moverMotores(255, -255);
                delay(50);
                moverMotores(0, 0);
                delay(10);
            }
            else if (PS4.Right() > 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10)); // Delay para garantir o fim da logica anterior

                moverMotores(255, -255); // Gira para a direita
                delay(50);
                moverMotores(255, 255); // Anda para frente
                delay(150);
                moverMotores(-255, 255); // Gira para a esquerda
                delay(90);
                moverMotores(255, 255); // Anda para frente
                delay(50);
                moverMotores(-255, 255);
                delay(50);
                moverMotores(0, 0);
                delay(10);
            }

            else if (PS4.Square() && !ativarMira)
            {
                ativarMira = true;
                delay(100);
            }
            else if (PS4.Square())
            {
                ativarMira = false;
                delay(100);
            }

            // ======================= || LIMITADOR DE VELOCIDADE || =======================

            // Limita a velocidade a 127
            else if (PS4.Triangle() && !velLimitada)
            {
                velLimitada = true;
                limiteVelocidade = 127;         // Define o limite reduzido
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
                if (valorSensorF == HIGH && valorSensorE == HIGH && valorSensorD == HIGH)
                {
                    moverMotores(255, 255);
                    delay(50);
                    continue;
                }
                else if (valorSensorF == HIGH)
                {
                    moverMotores(127, 127);
                    delay(30);
                    continue;
                }
                else
                {
                    if (lado == left)
                    {
                        moverMotores(-80, 80);
                        delay(30);
                        continue;
                    }
                    else if (lado == right)
                    {
                        moverMotores(80, -80);
                        delay(30);
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
                        velocidadeEsquerda = -int(limiteCurva * (abs(direcao) / limiteCurva));
                        velocidadeDireita = int(limiteCurva * (abs(direcao) / limiteCurva));
                    }
                    else if (direcao > zonaMorta)
                    { // Curva pra direita parado
                        velocidadeEsquerda = int(limiteCurva * (abs(direcao) / limiteCurva));
                        velocidadeDireita = -int(limiteCurva * (abs(direcao) / limiteCurva));
                    }
                    // Se nem gatilho nem analogico estiverem ativos, as velocidades continuam 0.
                }

                //===================== || GATILHOS PUROS || =============================

                else
                { // Pelo menos um gatilho pressionado (R2 ou L2)

                    if (r2 > zonaMorta)
                    { // Gatilho direito (frente)
                        if (ativarMira)
                        {
                            int velocidade = map(r2, zonaMorta, 255, 0, limiteVelocidade);
                            int somaSensores = (valorSensorE) + (valorSensorF) + (valorSensorD);

                            if (somaSensores == 3)
                            {
                                velocidadeEsquerda = 255;
                                velocidadeDireita = 255;
                            }
                            else if (somaSensores == 2)
                            {
                                velocidadeEsquerda = ((-valorSensorE) * (velocidade / 4)) + (valorSensorD * (velocidade / 2)) + ((valorSensorF) * (velocidade / 2));
                                velocidadeDireita = ((valorSensorE) * (velocidade / 2)) + ((-valorSensorD) * (velocidade / 4)) + ((valorSensorF) * (velocidade / 2));
                            }
                            else if (somaSensores == 1)
                            {
                                velocidadeEsquerda = ((valorSensorD) * (velocidade / 2)) + ((valorSensorF) * (velocidade / 2));
                                velocidadeDireita = ((valorSensorE) * (velocidade / 2)) + ((valorSensorF) * (velocidade / 2));
                            }
                            else
                            {
                                velocidadeEsquerda = velBase;
                                velocidadeDireita = velBase;
                            }
                        }
                        else
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

        vTaskDelay(10);
    }
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
}

#pragma endregion

// ==========================================================
// LOOP PRINCIPAL
// ==========================================================

#pragma region Loop MAIN

void loop()
{

    while (modoSelecionado == modoAguarda)
    {
        // Pisca lento para indicar "Esperando escolha"
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500)
        {
            lastBlink = millis();
            digitalWrite(led, !digitalRead(led)); // Lê o estado, inverte e escreve
        }

        if (IrReceiver.decode())
        {

            IrReceiver.resume();

            if (IrReceiver.decodedIRData.command == botao8)
            {                            // Botão 8 -> MODO AUTO
                digitalWrite(led, HIGH); // Confirma visualmente
                delay(1500);
                modoSelecionado = modoAUTO;
            }
            else if (IrReceiver.decodedIRData.command == botao9)
            {                            // Botão 9 -> MODO RC
                digitalWrite(led, HIGH); // Confirma visualmente
                delay(1500);
                modoSelecionado = modoRC;
            }
        }
        vTaskDelay(30);
    }

    if (modoSelecionado == modoAUTO)
    {
        setupAuto();
        loopAuto();
    }
    else if (modoSelecionado == modoRC)
    {
        setupRC();
        loopRC();
    }
}

#pragma endregion