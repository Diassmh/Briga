/*/=========================================================================================================
===========================================================================================================
=====================================     //    BRIGA V1   //    =============================================
==========================================================================================================
========================================================================================================*/

#pragma region Includes e Portas

//========================================//Defines e Includes//========================================//

#define DECODE_SONY
#include <BluetoothSerial.h>
//#include <analogWrite.h>
#include <Arduino.h>
#include <IRremote.hpp>
#include <ESP32Servo.h>
#include <PS4Controller.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif


#define SensorF 34
#define SensorD 39          // Pinos Sensores
#define SensorE 36

#define modoArguarda 0
#define modoAUTO 1             // modos
#define modoRC 2

#define botao1 0x0
#define botao2 0x1
#define botao3 0x2          // Botões do controle remoto
#define botao8 0x7
#define botao9 0x8 

#define SENSOR_POWER 25          //  AUTO | Transistor que desliga 

#define Motor_Direito_Positivo 16
#define Motor_Direito_Negativo 17           // Pinos Motores
#define Motor_Esquerdo_Positivo 19      
#define Motor_Esquerdo_Negativo 18

#define led 2                       // Pino do Led
#define IR 13                       // Pino IR

#define coefAtenuacao          1.8            // Atenuacao da velocidade em curvas (deve ser FLOAT)
#define coefReverse            0.9            // Coeficiente para balancear a re (deve ser FLOAT)
#define limiteCurva            127.0          // Velocidade limite em curvas puras (deve ser FLOAT)

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN         23             //  DOIS | Servomotor da haste ESP32 porta 23

#define SERVO_LEDC_CHANNEL     LEDC_CHANNEL_0 //  DOIS | Canal do PWM do servo
#define SERVO_TIMER            LEDC_TIMER_0   //  DOIS | Timer do PWM do servo


#define SERVO_ANGULO_INICIAL   90 
#define SERVO_FREQ_HZ          50             //  DOIS | Frequencia padrao do MG90S
#define SERVO_RESOLUTION       16             //  DOIS | Resolucao de 16 bits para o canal
#define SERVO_MIN_PULSE_US     544            //  DOIS | Largura de pulso para 0 graus (µs)
#define SERVO_MAX_PULSE_US     2400           //  DOIS | Largura de pulso para 180 graus (µs)

#pragma endregion

//========================================//Globais//========================================//

#pragma region Variaveis Globais

int modoSelecionado = 0;        // 0=Nada, 1=Auto, 2=RC

BluetoothSerial SerialBT;
Servo servoBandeira;

char BT = 'a';            // char que é recebida pelo bluetooth
char estrategia = 'a';     // estratégia de início da luta (qualquer caractere)
bool running = false;      // Flag para iniciar a estratégia  

const char* BRIGA = "7c:9e:bd:fb:83:80";           

#pragma endregion 


//======================================// Variaveis modo AUTO//========================================//

#pragma region Variaveis AUTO

volatile bool valorSensorF, valorSensorE, valorSensorD;    // Variáveis que vão armazenar o que os sensores enxergam

TaskHandle_t SensorTask;                     //Avisando o ESP que teremos uma task

unsigned long inicioDoAtaque = 0;       
unsigned long inicioDefensivo = 0;
bool detectandoFrente = false;
bool atacando = false;

#pragma endregion


//========================================// Variaveis modo RC //========================================//
#pragma region Variaveis RC


int zonaMorta                  = 20;          // Zona desconsiderada do controle de PS4
volatile int r2                = 0;           // Valor do gatilho direito do PS4 (R2)
volatile int l2                = 0;           // Valor do gatilho esquerdo do PS4 (L2)
volatile int direcao           = 0;           // Valor do direcional esquerdo do PS4 em X (LStickX)
volatile bool velLimitada      = true;        // Switch definido pra limitar a velocidade do motor
int limiteVelocidade           = 180;         // Limite de velocidade do motor


#pragma endregion


// ===============================|| FUNÇÕES DO MODO AUTÔNOMO ||===========================

#pragma region Funções AUTO

enum Direction             // define o lado do movimento inicial (esquerda ou direita)
{
    left,
    right,
    front
};
Direction lado = left;

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

    if (velDir >= 0) {  // Se positivo
        analogWrite(Motor_Direito_Positivo, abs(velDir));  // Anda para frente (pino positivo)
        analogWrite(Motor_Direito_Negativo, LOW);          // Desliga o pino negativo

    } else {  // Se negativo
        analogWrite(Motor_Direito_Negativo, abs(velDir));  // Anda para tras
        analogWrite(Motor_Direito_Positivo, LOW);
    }

    if (velEsq >= 0) {  // Se positivo
        analogWrite(Motor_Esquerdo_Positivo, abs(velEsq));  // Anda para frente
        analogWrite(Motor_Esquerdo_Negativo, LOW);

    } else {  // Se negativo
        analogWrite(Motor_Esquerdo_Negativo, abs(velEsq));  // Anda para tras
        analogWrite(Motor_Esquerdo_Positivo, LOW);
    }
}

void FunctionSensorTask(void *pvParameters)
{
    for (int i = 0; i < 4; i++)   // Inicialização 
    {
        digitalWrite(led, 0);
        delay(250);
        digitalWrite(led, 1);
        delay(250);
    }
    while (!running)    // Enquanto a flag running = 0 (robô aguardando comandos)
    {
        vTaskDelay(5);     // Aguarda
    }


    for (;;)       // Se o running = true  (loop infinito para checar os jsumos e o receptor)
    {   

        if (IrReceiver.decode())      // verifica se algum sinal infravermelho foi recebido     
        {
            IrReceiver.resume();      // Volta a analisar o receptor 
            if (IrReceiver.decodedIRData.command == botao3)      // Se o sinal recebido é 3
            {
                digitalWrite(led, 0);             //Apaga o led
                moverMotores(0,0);                // Para os motores 
                ESP.restart();                    // Reinicia o ESP
            }
        }

        valorSensorF = digitalRead(SensorF);
        valorSensorD = digitalRead(SensorD);        //Lê os sensores e armazena nas variáveis 0 ou 1
        valorSensorE = digitalRead(SensorE);
        
        if (valorSensorE == HIGH)      
        {
            lado = left;        //Guarda o ultimo lado que o adversário estava
        }
        else if (valorSensorD == HIGH)
        {
            lado = right;      //Guarda o ultimo lado que o adversário estava
        }
        vTaskDelay(1);   // espera 1ms para não sobrecarregar o ESP (vamos testar esse tempo)
    }
}

#pragma endregion

// ===============================|| FUNÇÕES DO MODO RC ||===============================
#pragma region Funções RC

// Quero colocar alguns macros aqui

#pragma endregion
// ======================================|| CONFIG ||=======================================
#pragma region Config

void configHardware() {
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
    //pinMode(SENSOR_POWER, OUTPUT);    Rolê do transistor, por hora não usamos
    
    // --- SERVO ---  Ativamos somente no MODO AUTO

    if (modoSelecionado == modoAUTO){
        servoBandeira.setPeriodHertz(SERVO_FREQ_HZ);    
        servoBandeira.attach(SERVOMOTOR_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
        servoBandeira.write(SERVO_ANGULO_INICIAL); 
        delay(200);  
    }    
}

#pragma endregion

// ==========================================================
// SETUP + LOOP   || MODO AUTO ||
// ==========================================================

#pragma region Setup AUTO

void setupAUTO() {

    Serial.println("Iniciando Modo Autonomo...");

    configHardware();   
    
    // Inicia Bluetooth do Celular (SerialBT):
    SerialBT.begin("Briga"); 
    SerialBT.println("CONECTADO");

    // Task dos sensores é chamada somente no modo AUTO:
    xTaskCreatePinnedToCore(FunctionSensorTask, "SensorTask", 10000, NULL, 1, &SensorTask, 0);

    moverMotores(0,0);     // Garante que os motores estão desligados

    running = false;       // Task dos sensores permanece desativada

    while (true)
    { // Inicio quando bluetooth recebe o char '0'
        
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
        else if (BT == '='){
            lado = front;
        }
        
        // LADO QUE A BANDEIRA VAI CAIR -----------------------

        else if(BT == 'l'){
            Lado_Bandeira = B_Esq;
        }
        else if(BT == 'r'){
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
        }
        else
        {
            if (!(BT == 13 || BT == 10 || (BT > 48 && BT < 58)))   // Filtro para não recebermos ENTER
            { // Se não for ENTER ou numero altera a estrategia inicial
                estrategia = BT;
            }
        }

        if (IrReceiver.decode())  // No final do Loop checamos o receptor 
        {
            IrReceiver.resume();
            if (IrReceiver.decodedIRData.command == botao1) // Se recebermos um 1:
            {
                for (int i = 0; i < 3; i++)      // Entramos no loop
                {
                    digitalWrite(led, 0);   
                    delay(30);                    // Piscamos o LED
                    digitalWrite(led, 1);
                    delay(30);
                    if (IrReceiver.decode())   // Se o juiz apertar o 1 e logo depois o 2, identificamos rapidamente aqui
                    {
                        IrReceiver.resume();
                        if (IrReceiver.decodedIRData.command == botao2)   // se Recebermos 2
                        {
                            digitalWrite(led, 1);
                            break;
                        }
                    }
                }
            }

            if (IrReceiver.decodedIRData.command == botao2)   // se Recebermos 2
            {
                digitalWrite(led, 1);
                break;
            }
        }
    }
    
    // COMEÇO DE LUTA --------------------------------------------

    SerialBT.println("BORAAAAAAAAA");
    running = true;
   
    // Queda da Bandeira:  
    if(estrategia != 'D'){      // Se a estratégia for diferente do Defensivo Posicionado:
        
        if (Lado_Bandeira == B_Esq){
            servoBandeira.write(0);        // Cai para esquerda
        }   
        else if(Lado_Bandeira == B_Dir){
            servoBandeira.write(180);      // Cai para direita
        }
    } 
    
    // Execução de movimentação inicial:
    switch (estrategia)
    {
    case 'f':    // frentão Progressivo   3/4 dohyo
        SerialBT.print("FRENTAO PROGRESSIVO");
        moverMotores(48,48);
        delay(4);
        moverMotores(60,60);
        delay(4);
        moverMotores(72,72);
        delay(4);
        moverMotores(84,84);
        delay(4);
        moverMotores(96,96);
        delay(4);
        moverMotores(128,128);
        delay(8);
        moverMotores(160,160);
        delay(8);
        moverMotores(196,196);
        delay(8);
        moverMotores(228,228);
        delay(12);
        moverMotores(255,255);
        delay(108);
        break;

    case 'F': // Frentão Anti Pescador  3/4 dohyo
        
        SerialBT.print("FRENTAO BOLADO");
        moverMotores(255, 255);
        delay(200);
        break;
    
    case 'i':      // Frentinho 1/4 do dohyo 
       
    SerialBT.print("FRENTINHO");
        moverMotores(127, 127);
        delay(200);
        break;
    
    case 'b':          // Costas
        
    SerialBT.print("COSTAS");
        if (lado == front){
            moverMotores(-255,-255);
            delay(60);
            break;
        }
        else if (lado == left){
            moverMotores(-255,-255);
            delay(60);
            moverMotores(-255,255);
            delay(50);
            moverMotores(255,255);
            delay(70);
            moverMotores(0,0);
            delay(10);
            lado = right;
            break;
        }
         else if (lado == right){
            moverMotores(-255,-255);
            delay(60);
            moverMotores(255,-255);
            delay(50);
            moverMotores(255,255);
            delay(70);
            moverMotores(0,0);
            delay(10);
            lado = left;
            break;
        }
        

    case 'c':   // Curva Leve
        
       SerialBT.print("CURVA LEVE");

       if (lado == left){
           moverMotores(-255, 255);
           delay(50);
           moverMotores(255, 255);
           delay(100);
           moverMotores(0,0);
           delay(10);
           lado = right;
           break;
       }
        else if (lado == right){
           moverMotores(255,-255);
           delay(50);
           moverMotores(255, 255);
           delay(100);
           moverMotores(0,0);
           delay(10);
           lado = left;
           break;
        } 
    
    case 'C':   // Curva Pesada
        
        SerialBT.print("CURVA PESADA");   

        if (lado == left){
           moverMotores(-255, 255);
           delay(100);
           moverMotores(255, 255);
           delay(100);
           moverMotores(0,0);
           delay(10);
           lado = right;
           break;
        }
        else if (lado == right){
           moverMotores(255,-255);
           delay(100);
           moverMotores(255, 255);
           delay(100);
           moverMotores(0,0);
           delay(10);
           lado = left;
           break;
        }
    case 'd':  // Defensivo

        SerialBT.print("Defensivo"); 
        while(true){
          if (valorSensorF == HIGH && valorSensorE == HIGH && valorSensorD == HIGH) 
            {
                break;
            }
           
            else if (valorSensorF == HIGH)
            {
                moverMotores(0,0);
            }
            else
            {
                if (lado == left)
                    moverMotores(-100, 100);
                else if (lado == right)
                    moverMotores(100, -100);
            }
        } 
        break;   
    case 'D':                // Defensivo Posicionado    
       
        SerialBT.print("Defensivo Posicionado"); 
        
        while(true){
           
            if(valorSensorF == HIGH)
            {
                if (!detectandoFrente) {
                    inicioDefensivo = millis(); // Marca o tempo em que o adversário começou a ser visto com o sensorF
                    detectandoFrente = true;
                }
               
                if (millis() - inicioDefensivo > 200){   // Se o inimigo já está na frente a mais de 200ms...
                    break; 
                }   
                else{
                moverMotores(0,0);        // Garante que está parado
                }
            }    

            else if ((valorSensorE == HIGH) && (valorSensorD == LOW) && (valorSensorF == LOW)){
                servoBandeira.write(0);
                delay(50);
                moverMotores(255,-255);
                delay(50);
                moverMotores(255,255);
                delay(50);
                 // TESTAR PARA VER SE PRECISAMOS FORÇAR O GIRO NO PROPRIO EIXO NA DIREÇÃO DO ADVERSARIO
                break;
            }
            else if ((valorSensorD == HIGH) && (valorSensorE == LOW) && (valorSensorF == LOW)){
                servoBandeira.write(180);
                delay(50);
                moverMotores(-255,255);
                delay(50);
                moverMotores(255,255);
                delay(50);
                // TESTAR PARA VER SE PRECISAMOS FORÇAR O GIRO NO PROPRIO EIXO NA DIREÇÃO DO ADVERSARIO
                break;
            }    
        }
        break;
    case 'e':  // desempate
        if(lado == left){
            moverMotores(-255,255);
            delay(100);
            break;
        }
        else if (lado == right){
            moverMotores(255,-255);
            delay(100);
            break;
        }
    }

}

#pragma endregion

#pragma region Loop AUTO

void loopAutonomo() {
    //Em caso de testes:
    //SerialBT.printf("ESFREN: %d\nFEN: %d\nDIFRE: %d\n", valueSensorE, valueSensorF, valueSensorD);
    if (valorSensorF == HIGH) 
    {
        if (!atacando) 
        {
            inicioDoAtaque = millis(); 
            atacando = true; 
        }

        // Cálculo do tempo que passou desde que começamos a ver o adversário com o sensor do meio
        unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

        if (tempoDeAtaque < 200)                 // Tempo da "aproximação lenta" (em ms)
        {
            moverMotores(127, 127);             //  Velocidade de aproximação
        }
        else                                    // Se o sensorF esta vendo por mais de 200 ms:
        {
            moverMotores(255, 255);             // Potencia máxima
        }
    }
    else                             
    {
        // Se perdi o oponente, aviso que não estou mais atacando.
        // Assim, na próxima vez que eu ver ele, a lógica "lenta" reinicia.
        atacando = false; 

        if (lado == right){ 
            moverMotores(150, -150);
        }
        else if (lado == left){
            moverMotores(-150, 150);
        }
        
    
    }
     vTaskDelay(1);   // Pequeno delay pro ESP não fritar
}

#pragma endregion

// ==========================================================
// SETUP + LOOP == MODO RC ==  
// ==========================================================

#pragma region Setup RC

void setupRC() {
    
    Serial.println("Iniciando Modo RC ...");
    configHardware();
    
    PS4.begin(BRIGA);   // MAC Adress definido no inicio do código
    
    Serial.println("Aguardando conexao do controle...");
    
    while (!PS4.isConnected()) {
        digitalWrite(led,LOW);
    }

    Serial.println("Controle conectado!");
}

#pragma endregion

#pragma region Loop RC

void loopRC() {       
    
    if (PS4.isConnected()) {
        
        int velocidadeEsquerda = 0;           // Velocidade do motor esquerdo
        int velocidadeDireita = 0;            // Velocidade do motor direito

        // ======================= || COMANDOS COM BOTÕES (MACROS) || ===================

        if (PS4.Cross()) {                      // X = Frentão        
            moverMotores(255, 255);
        }    
        

        // ======================= || LIMITADOR DE VELOCIDADE || =======================
        
        // Limita a velocidade a 180
        else if (PS4.Triangle() && !velLimitada) {
            velLimitada = true;
            limiteVelocidade = 180;           // Define o limite reduzido
            vTaskDelay(pdMS_TO_TICKS(200));   // Evita multiplas leituras
        } 

        // Permite a velocidade de 255
        else if (PS4.Triangle()) {
            velLimitada = false;
            limiteVelocidade = 255;           // Restaura o limite maximo
            vTaskDelay(pdMS_TO_TICKS(200));   // Evita multiplas leituras
        }

        // ======================= || MOVIMENTAÇÃO LIVRE || ==========================

        else {
            
            r2 = PS4.R2Value();               // Armazena o valor de R2
            l2 = PS4.L2Value();               // Armazena o valor de R2
            direcao = PS4.LStickX();          // Armazena o valor em X do Direcional Esquerdo

            // Se nenhum gatilho estiver pressionado (considerando a zona morta)

            //===================== || CURVAS PURAS || =============================

            if (r2 <= zonaMorta && l2 <= zonaMorta) {
                velocidadeEsquerda = 0;
                velocidadeDireita = 0;

                // Se os gatilhos estao parados, verifica se o analogico quer girar
                if (direcao < -zonaMorta) {          // Curva pra esquerda parado
                    velocidadeEsquerda = -int(limiteCurva * (abs(direcao) / limiteCurva));
                    velocidadeDireita = int(limiteCurva * (abs(direcao) / limiteCurva));
                } else if (direcao > zonaMorta) {    // Curva pra direita parado
                    velocidadeEsquerda = int(limiteCurva * (abs(direcao) / limiteCurva));
                    velocidadeDireita = -int(limiteCurva * (abs(direcao) / limiteCurva));
                }
                // Se nem gatilho nem analogico estiverem ativos, as velocidades continuam 0.
            }
            
             //===================== || GATILHOS PUROS || =============================  

            else{       // Pelo menos um gatilho pressionado (R2 ou L2)
                
                if (r2 > zonaMorta) {                // Gatilho direito (frente)
                    int velocidade = map(r2, zonaMorta, 255, 0, limiteVelocidade);
                    velocidadeEsquerda = velocidade;
                    velocidadeDireita = velocidade; 
                }    
                else {                      // Gatilho esquerdo (re)
                    int velocidade = map(l2, zonaMorta, 255, 0, limiteVelocidade);
                    velocidadeEsquerda = -velocidade;
                    velocidadeDireita = - int(coefReverse * velocidade);
                }
                 
                //===================== || GATILHOS + CURVA || ============================= 

                // Aplica a curva sobre a velocidade existente

                if (direcao < -zonaMorta) {          // Curva pra esquerda em movimento
                    // Reduz a velocidade da roda interna (esquerda para frente, direita para re)
                    if (r2 > zonaMorta) 
                        velocidadeEsquerda *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);
                    else 
                        velocidadeDireita *= (1.0 - (abs(direcao) * coefAtenuacao) / 255.0);

                } else if (direcao > zonaMorta) {    // Curva pra direita em movimento
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
    else{
         moverMotores(0,0);  // Para os motores caso a conexão seja perdida
    }
   

}      

#pragma endregion

// ==========================================================
// SETUP PRINCIPAL 
// ==========================================================

#pragma region Setup MAIN

void setup() {
    Serial.begin(115200);
    
    // 1. Inicia Apenas o Básico (IR e LED)
    pinMode(led, OUTPUT);
    IrReceiver.begin(IR, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
    
    Serial.println("=== LOBBY DE SELEÇÃO ===");
    Serial.println("Aperte 8 para AUTO | Aperte 9 para RC");

    // 2. Loop de Espera (SELECIOAR MODO)
    while (modoSelecionado == modoArguarda) 
    {
        // Pisca lento para indicar "Esperando escolha"
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            digitalWrite(led, !digitalRead(led));
            lastBlink = millis();
        }

        if (IrReceiver.decode()) {
            IrReceiver.resume(); 
            
            if (IrReceiver.decodedIRData.command == botao8) { // Botão 8 -> MODO AUTO
                modoSelecionado = modoAUTO;
                digitalWrite(led, 1); // Confirma visualmente
                delay(1000);
            }
            else if (IrReceiver.decodedIRData.command == botao9) { // Botão 9 -> MODO RC
                modoSelecionado = modoRC;
                digitalWrite(led, 1); // Confirma visualmente
                delay(1000);
            }
        }
    }

    // 3. Direciona para o Setup correto
    if (modoSelecionado == modoAUTO) {
        setupAUTO();
    } 
    else if (modoSelecionado == modoRC) {
        setupRC();
    }
}

#pragma endregion

// ==========================================================
// LOOP PRINCIPAL 
// ==========================================================

#pragma region Loop MAIN

void loop() {
    if (modoSelecionado == modoAUTO) {
        loopAutonomo();
    } 
    else if (modoSelecionado == modoRC) {
        loopRC();
    }
}

#pragma endregion