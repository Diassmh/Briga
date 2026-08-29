//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//
#pragma region INCLUDES

#ifndef MOVE_H
#define MOVE_H

#include <defines.hpp>

#pragma endregion

//===============================================================================================//
//========================================//MOVIMENTACAO//=======================================//
//===============================================================================================//

#pragma region PWM

void moverMotores(int velEsq, int velDir)
{
    // Variáveis estáticas lembram o último comando enviado mesmo após a função terminar
    static int ultimaVelEsq = 0;
    static int ultimaVelDir = 0;

    // Checa se algum dos motores inverteu a rotação:
    bool inverteuDir = (velDir > 0 && ultimaVelDir < 0) || (velDir < 0 && ultimaVelDir > 0);
    bool inverteuEsq = (velEsq > 0 && ultimaVelEsq < 0) || (velEsq < 0 && ultimaVelEsq > 0);

    if (inverteuDir || inverteuEsq) // Se algum deles inverteu a rotação, limpamos o PWM:
    {
        analogWrite(Motor_Direito_Positivo, LOW);
        analogWrite(Motor_Direito_Negativo, LOW);
        analogWrite(Motor_Esquerdo_Positivo, LOW);
        analogWrite(Motor_Esquerdo_Negativo, LOW);
        delayMicroseconds(60);
    }

    // --- ACIONAMENTO MOTOR DIREITO ---
    if (velDir > 0)
    {
        analogWrite(Motor_Direito_Positivo, abs(velDir));
        analogWrite(Motor_Direito_Negativo, LOW);
    }
    else if (velDir < 0)
    {
        analogWrite(Motor_Direito_Negativo, abs(velDir));
        analogWrite(Motor_Direito_Positivo, LOW);
    }
    else if (velDir == 0)
    {
        analogWrite(Motor_Direito_Positivo, 255); // mudar
        analogWrite(Motor_Direito_Negativo, 255); // mudar
    }

    // --- ACIONAMENTO MOTOR ESQUERDO ---
    if (velEsq > 0)
    {
        analogWrite(Motor_Esquerdo_Positivo, abs(velEsq));
        analogWrite(Motor_Esquerdo_Negativo, LOW);
    }
    else if (velEsq < 0)
    {
        analogWrite(Motor_Esquerdo_Negativo, abs(velEsq));
        analogWrite(Motor_Esquerdo_Positivo, LOW);
    }

    else if (velEsq == 0)
    {
        analogWrite(Motor_Esquerdo_Positivo, 255); // mudar
        analogWrite(Motor_Esquerdo_Negativo, 255); // mudar
    }
    // Atualiza o histórico para o próximo ciclo
    ultimaVelEsq = velEsq;
    ultimaVelDir = velDir;
}
#pragma endregion

#pragma region BUSCAS

// Fluxo principal da BuscaCorna:
// BUSCAR -> VIRAR/ALINHAR -> ESPERAR -> AVANCAR -> ATAQUE
// Sem o IR central detectando, os sensores de linha interrompem essas etapas.

//=====================================|| BUSCA OFENSIVA ||=======================================================

void buscaOfensiva()
{

    if (valorJsumoF == HIGH || valorJsumoE == HIGH || valorJsumoD == HIGH)
    {
        if (!atacando)
        {
            inicioDoAtaque = millis();
            atacando = true;
        }

        unsigned long tempoDeAtaque = millis() - inicioDoAtaque;

        // Define a velocidade máxima baseado no tempo do ciclo do ataque
        int velMax = (tempoDeAtaque < 500) ? 70 : 255;
        int velReduzida = velMax / 2;

        // 1. Oponente Centralizado
        if (valorJsumoF == HIGH && (valorJsumoE == HIGH && valorJsumoD == HIGH))
        {
            moverMotores(velMax, velMax);
        }
        // 2. Oponente na Diagonal Direita
        else if (valorJsumoF == HIGH && valorJsumoD == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velMax, velReduzida);
            }
            else
            {
                moverMotores(velMax, velMax);
            }
        }
        // 3. Oponente na Diagonal Esquerda
        else if (valorJsumoF == HIGH && valorJsumoE == HIGH)
        {
            if (tempoDeAtaque < 300)
            {
                moverMotores(velReduzida, velMax);
            }
            else
            {
                moverMotores(velMax, velMax);
            }
        }
        // 4. Oponente apenas na frente
        else if (valorJsumoF == HIGH)
        {
            moverMotores(velMax, velMax);
        }
        // 5. Oponente apenas na Direita:
        else if (valorJsumoD == HIGH)
        {
            moverMotores(velMax, 0);
        }
        // 6. Oponente apenas na Esquerda:
        else if (valorJsumoE == HIGH)
        {
            moverMotores(0, velMax);
        }
    }
    else
    {
        // 7. Openente não detectado:
        atacando = false;

        if (lado == direita)
        {
            moverMotores(100, -100);
        }
        else if (lado == esquerda)
        {
            moverMotores(-100, 100);
        }
    }
}

bool algumReceptorFrontal(const LeituraSensores &sensores)
{
    return sensores.receptorEsquerdo || sensores.receptorFrente || sensores.receptorDireito;
}

bool alvoCentralizado(const LeituraSensores &sensores)
{
    bool somenteCentro = sensores.receptorFrente && !sensores.receptorEsquerdo && !sensores.receptorDireito;
    bool todosFrontais = sensores.receptorFrente && sensores.receptorEsquerdo && sensores.receptorDireito;

    return somenteCentro || todosFrontais;
}

bool linhaEsquerdaDetectada(const LeituraSensores &sensores)
{
    return sensores.linhaEsquerda < LIMIAR_LINHA_E;
}

bool linhaDireitaDetectada(const LeituraSensores &sensores)
{
    return sensores.linhaDireita < LIMIAR_LINHA_D;
}

void iniciarRecuperacaoLinha(bool detectouLinhaE, bool detectouLinhaD)
{
    // Guarda o que o robo estava fazendo antes da fuga da linha.
    estadoAntesDaLinha = estadoAtual;

    if (detectouLinhaE && detectouLinhaD)
    {
        linhaQueAtivou = DUAS_LINHAS;

        // Quando as duas linhas aparecem, escolhemos o giro contrario ao ultimo lado perseguido.
        if (lado == esquerda)
        {
            recuperarParaDireita = true;
        }
        else if (lado == direita)
        {
            recuperarParaDireita = false;
        }
        else
        {
            recuperarParaDireita = !recuperarParaDireita;
        }
    }
    else if (detectouLinhaE)
    {
        linhaQueAtivou = SOMENTE_LINHA_E;
        recuperarParaDireita = true;
    }
    else
    {
        linhaQueAtivou = SOMENTE_LINHA_D;
        recuperarParaDireita = false;
    }

    etapaRecuperacaoLinha = RECUANDO_DA_LINHA;
    tempoEtapaLinha = millis();
    inicioRecuperacaoLinha = millis();
    inicioPisoSeguro = 0;
    falhaRecuperacaoLinha = false;
    estadoAtual = RECUPERAR_LINHA;
}

void cancelarRecuperacaoLinha()
{
    // Se o IR central voltou a ver o adversario, ele assume a prioridade.
    // O robo retoma o estado em que estava antes de encontrar a linha.
    estadoAtual = estadoAntesDaLinha;
    linhaQueAtivou = NENHUMA_LINHA;
    etapaRecuperacaoLinha = RECUANDO_DA_LINHA;
    tempoEtapaLinha = 0;
    inicioRecuperacaoLinha = 0;
    inicioPisoSeguro = 0;
    falhaRecuperacaoLinha = false;
}

void atualizarRecuperacaoLinha(const LeituraSensores &sensores)
{
    if (falhaRecuperacaoLinha)
    {
        moverMotores(0, 0);
        return;
    }

    if (millis() - inicioRecuperacaoLinha >= TEMPO_MAX_RECUPERACAO_MS)
    {
        moverMotores(0, 0);
        falhaRecuperacaoLinha = true;
        SerialBT.println("ERRO: recuperacao da linha passou do tempo maximo");
        return;
    }

    switch (etapaRecuperacaoLinha)
    {
    case RECUANDO_DA_LINHA:
    {
        moverMotores(-255, -255);

        unsigned long tempoRecuo = TEMPO_RECUO_LINHA_MS;
        if (linhaQueAtivou == DUAS_LINHAS)
        {
            tempoRecuo = TEMPO_RECUO_DUAS_LINHAS_MS;
        }

        if (millis() - tempoEtapaLinha >= tempoRecuo)
        {
            etapaRecuperacaoLinha = GIRANDO_PARA_DENTRO;
            tempoEtapaLinha = millis();
        }
        break;
    }

    case GIRANDO_PARA_DENTRO:
        if (recuperarParaDireita)
        {
            moverMotores(255, -255);
        }
        else
        {
            moverMotores(-255, 255);
        }

        if (millis() - tempoEtapaLinha >= TEMPO_GIRO_LINHA_MS)
        {
            etapaRecuperacaoLinha = CONFIRMANDO_PISO;
            tempoEtapaLinha = millis();
            inicioPisoSeguro = 0;
        }
        break;

    case CONFIRMANDO_PISO:
    {
        moverMotores(0, 0);

        bool aindaNaLinha = linhaEsquerdaDetectada(sensores) || linhaDireitaDetectada(sensores);

        if (aindaNaLinha)
        {
            // Se ainda encontrou branco, recua novamente sem sair da protecao da linha.
            etapaRecuperacaoLinha = RECUANDO_DA_LINHA;
            tempoEtapaLinha = millis();
            inicioPisoSeguro = 0;
        }
        else
        {
            if (inicioPisoSeguro == 0)
            {
                inicioPisoSeguro = millis();
            }

            if (millis() - inicioPisoSeguro >= TEMPO_CONFIRMA_PISO_MS)
            {
                estadoAtual = BUSCAR;
                tempoBusca = millis();
                avancoSemAlvo = false;
                linhaQueAtivou = NENHUMA_LINHA;
            }
        }
        break;
    }
    }
}

void buscaCorna()
{
    // Cada decisao usa uma copia unica das leituras feitas pela task de sensores.
    LeituraSensores sensores = copiarLeituraSensores();

    bool detectouLinhaE = linhaEsquerdaDetectada(sensores);
    bool detectouLinhaD = linhaDireitaDetectada(sensores);
    bool adversarioNaFrente = sensores.receptorFrente;

    // O IR frontal central tem prioridade sobre os sensores de linha.
    // Enquanto ele estiver vendo o adversario, o Briga continua a acao atual.
    if (estadoAtual == RECUPERAR_LINHA && adversarioNaFrente)
    {
        cancelarRecuperacaoLinha();
    }

    // Sem adversario no centro, a linha volta a proteger o robo normalmente.
    if (estadoAtual != RECUPERAR_LINHA &&
        !adversarioNaFrente &&
        (detectouLinhaE || detectouLinhaD))
    {
        iniciarRecuperacaoLinha(detectouLinhaE, detectouLinhaD);
    }

    if (estadoAtual == RECUPERAR_LINHA)
    {
        atualizarRecuperacaoLinha(sensores);
        return;
    }

    // A logica do LDR foi mantida como estava ate a equipe definir como ele funcionara no robo.
    if (abs(sensores.ldr - ldrReferencia) > 300)
    {
        estadoAtual = ATAQUE_RAPIDO;
    }

    // Máquina de Estados
    switch (estadoAtual)
    {

    case BUSCAR:
        digitalWrite(2, LOW); // Usando apenas o LED 2

        if (tempoBusca == 0)
        {
            tempoBusca = millis();
        }

        // Procura primeiro no ultimo lado em que o adversario foi visto.
        if (lado == direita)
        {
            moverMotores(VEL_BUSCA, -VEL_BUSCA);
        }
        else
        {
            moverMotores(-VEL_BUSCA, VEL_BUSCA);
        }

        // Os sensores da frente tem prioridade sobre os laterais.
        if (algumReceptorFrontal(sensores))
        {
            if (sensores.receptorEsquerdo && !sensores.receptorDireito)
            {
                lado = esquerda;
            }
            else if (sensores.receptorDireito && !sensores.receptorEsquerdo)
            {
                lado = direita;
            }

            ultimaDeteccaoFrontal = millis();
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else if (millis() - tempoBusca >= TEMPO_BUSCA_MS)
        {
            // Pequeno avanco para nao ficar procurando sempre no mesmo ponto.
            avancoSemAlvo = true;
            tempoAvancoLento = millis();
            estadoAtual = AVANCAR_LENTO;
        }
        break;

    case ALINHAR_FRENTE:
        if (alvoCentralizado(sensores))
        {
            moverMotores(0, 0);
            tempoTravamento = millis();
            ultimaDeteccaoFrontal = millis();
            estadoAtual = ESPERAR_ALVO;
        }
        else if (sensores.receptorEsquerdo && sensores.receptorFrente && !sensores.receptorDireito)
        {
            // Ja esta quase de frente, entao corrige fazendo uma curva para a esquerda.
            moverMotores(50, 100);
            lado = esquerda;
            ultimaDeteccaoFrontal = millis();
        }
        else if (sensores.receptorDireito && sensores.receptorFrente && !sensores.receptorEsquerdo)
        {
            moverMotores(100, 50);
            lado = direita;
            ultimaDeteccaoFrontal = millis();
        }
        else if (sensores.receptorEsquerdo && !sensores.receptorDireito)
        {
            moverMotores(-100, 100);
            lado = esquerda;
            ultimaDeteccaoFrontal = millis();
        }
        else if (sensores.receptorDireito && !sensores.receptorEsquerdo)
        {
            moverMotores(100, -100);
            lado = direita;
            ultimaDeteccaoFrontal = millis();
        }
        else if (sensores.receptorEsquerdo && sensores.receptorDireito)
        {
            // Leitura ambigua: mantem a ultima direcao conhecida, sem escolher esquerda por ordem do if.
            if (lado == esquerda)
            {
                moverMotores(-80, 80);
            }
            else if (lado == direita)
            {
                moverMotores(80, -80);
            }
            else
            {
                moverMotores(50, 50);
            }
            ultimaDeteccaoFrontal = millis();
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else
        {
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case ESPERAR_ALVO:
        moverMotores(0, 0);
        digitalWrite(2, HIGH);

        if (alvoCentralizado(sensores))
        {
            ultimaDeteccaoFrontal = millis();

            if (millis() - tempoTravamento >= 400)
            {
                digitalWrite(2, LOW);
                avancoSemAlvo = false;
                tempoAvancoLento = millis();
                estadoAtual = AVANCAR_LENTO;
            }
        }
        else if (algumReceptorFrontal(sensores))
        {
            // Se o adversario saiu do centro, volta a alinhar em vez de desistir dele.
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else
        {
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case AVANCAR_LENTO:
        if (algumReceptorFrontal(sensores))
        {
            // Durante a aproximacao os dois motores continuam para frente.
            // A correcao e feita diminuindo apenas o lado de dentro da curva.
            avancoSemAlvo = false;
            ultimaDeteccaoFrontal = millis();

            if (alvoCentralizado(sensores))
            {
                moverMotores(VEL_APROXIMACAO, VEL_APROXIMACAO);
            }
            else if (sensores.receptorEsquerdo && !sensores.receptorDireito)
            {
                moverMotores(VEL_APROXIMACAO_LENTA, VEL_APROXIMACAO_RAPIDA);
                lado = esquerda;
            }
            else if (sensores.receptorDireito && !sensores.receptorEsquerdo)
            {
                moverMotores(VEL_APROXIMACAO_RAPIDA, VEL_APROXIMACAO_LENTA);
                lado = direita;
            }
            else
            {
                moverMotores(VEL_APROXIMACAO, VEL_APROXIMACAO);
            }
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else if (avancoSemAlvo)
        {
            moverMotores(VEL_APROXIMACAO, VEL_APROXIMACAO);

            if (millis() - tempoAvancoLento >= TEMPO_AVANCO_SEM_ALVO_MS)
            {
                estadoAtual = BUSCAR;
                tempoBusca = millis();
                avancoSemAlvo = false;
            }
        }
        else if (millis() - ultimaDeteccaoFrontal <= TEMPO_PERDA_ALVO_MS)
        {
            // Mantem o movimento por poucos milissegundos para nao reagir a uma falha isolada.
            moverMotores(VEL_APROXIMACAO, VEL_APROXIMACAO);
        }
        else
        {
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case VIRAR_LATERAL_E:
        if (algumReceptorFrontal(sensores))
        {
            ultimaDeteccaoFrontal = millis();
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else if (sensores.jsumoEsquerdo)
        {
            moverMotores(-VEL_GIRO_LATERAL, VEL_GIRO_LATERAL);
            lado = esquerda;
        }
        else
        {
            // O alvo saiu do sensor de 90 graus. Continua para o mesmo lado procurando os frontais.
            tempoTransferencia = millis();
            estadoAtual = TRANSFERIR_LATERAL_E;
        }
        break;

    case VIRAR_LATERAL_D:
        if (algumReceptorFrontal(sensores))
        {
            ultimaDeteccaoFrontal = millis();
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito)
        {
            moverMotores(VEL_GIRO_LATERAL, -VEL_GIRO_LATERAL);
            lado = direita;
        }
        else
        {
            tempoTransferencia = millis();
            estadoAtual = TRANSFERIR_LATERAL_D;
        }
        break;

    case TRANSFERIR_LATERAL_E:
        moverMotores(-VEL_TRANSFERENCIA, VEL_TRANSFERENCIA);

        if (algumReceptorFrontal(sensores))
        {
            ultimaDeteccaoFrontal = millis();
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoEsquerdo)
        {
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (sensores.jsumoDireito && !sensores.jsumoEsquerdo)
        {
            lado = direita;
            estadoAtual = VIRAR_LATERAL_D;
        }
        else if (millis() - tempoTransferencia >= TEMPO_TRANSFERENCIA_MS)
        {
            lado = esquerda;
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case TRANSFERIR_LATERAL_D:
        moverMotores(VEL_TRANSFERENCIA, -VEL_TRANSFERENCIA);

        if (algumReceptorFrontal(sensores))
        {
            ultimaDeteccaoFrontal = millis();
            estadoAtual = ALINHAR_FRENTE;
        }
        else if (sensores.jsumoDireito)
        {
            estadoAtual = VIRAR_LATERAL_D;
        }
        else if (sensores.jsumoEsquerdo && !sensores.jsumoDireito)
        {
            lado = esquerda;
            estadoAtual = VIRAR_LATERAL_E;
        }
        else if (millis() - tempoTransferencia >= TEMPO_TRANSFERENCIA_MS)
        {
            lado = direita;
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case ATAQUE_RAPIDO:
        moverMotores(255, 255); // Avança com tudo (adversário rampado)
        if (abs(sensores.ldr - ldrReferencia) <= 300)
        {
            estadoAtual = BUSCAR;
            tempoBusca = millis();
        }
        break;

    case RECUPERAR_LINHA:
        // Esse estado e tratado no inicio da funcao quando o IR central nao esta detectando.
        break;

    default:
        estadoAtual = BUSCAR;
        tempoBusca = millis();
        break;
    }
}

#endif