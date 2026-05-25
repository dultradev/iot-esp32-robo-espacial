# Controle Remoto no Wokwi do Robô Espacial

## 🚀 Objetivo da Etapa
Esta etapa do projeto consiste no desenvolvimento do **módulo de controle remoto** simulado no Wokwi. O objetivo principal é capturar as entradas analógicas de um joystick e o estado de um interruptor de segurança para processar e enviar comandos de movimentação e status para o robô explorador (físico ou simulado).

O sistema atua como a interface de comando, interpretando as intenções do operador e fornecendo feedback visual imediato sobre o estado da conexão e operação do robô.

## 🛠️ Componentes do Circuito
O circuito simulado utiliza os seguintes componentes:
- **Placa Microcontroladora:** ESP32-S3
- **Joystick Analógico:** Responsável pelo controle direcional (Eixos X e Y).
- **LED Verde:** Indicador de status "Operando Normalmente" e "Conectado".
- **LED Vermelho:** Indicador de status "Robô Desligado" ou "Emergência".
- **Botão/Interruptor:** Atua como o gatilho para o comando remoto de "DESLIGAR".

## 🎮 Funcionalidades do Firmware
- **Movimentação (4 Direções):**
    - **Frente/Trás:** Leitura do eixo Y.
    - **Esquerda/Direita:** Leitura do eixo X (ativando motores individuais).
- **Comando de Desligamento:** Ao pressionar o botão, o controle envia imediatamente uma string de comando "DESLIGAR", interrompendo a lógica de movimentação e alterando o status visual nos LEDs.
- **Feedback Serial:** Monitoramento contínuo dos comandos enviados e do estado da comunicação.

## 💻 Como Rodar no Wokwi
Para visualizar e testar o funcionamento do controle remoto, siga os passos abaixo:

1. Acesse o projeto através do link: [Projeto Wokwi - Controle Remoto](https://wokwi.com/projects/463749722457582593)
2. Clique no botão de **Play** (Iniciar Simulação).
3. Abra o **Serial Monitor** na interface do Wokwi para visualizar as saídas de texto.
4. **Interação:**
   - Mova o Joystick para ver os comandos de direção sendo impressos.
   - Pressione o Botão para alternar entre o estado de "Ligado" (LED Verde) e "Desligado" (LED Vermelho).

---
*Projeto desenvolvido como parte dos estudos de Engenharia de Computação no SENAI CIMATEC.*

# 🚀 Robô Explorador IoT: Telemetria e Controle com ESP32-S3 na AWS

Este projeto consiste no desenvolvimento do firmware e da arquitetura de nuvem para um **Robô Explorador Espacial**. O sistema utiliza um único microcontrolador **ESP32-S3** para processar, simultaneamente, os comandos manuais de um controle remoto (joystick) e a coleta autônoma de dados ambientais, calculando em tempo real a "Probabilidade de Vida" em um ambiente.

O projeto integra computação na borda (Edge Computing) com serviços de mensageria na nuvem (**AWS IoT Core**, **Amazon DynamoDB**) e envio de alertas críticos via **WhatsApp** (CallMeBot API).

---

## ⚙️ Funcionalidades

- **Controle Remoto em Tempo Real:** Leitura de um joystick analógico e atuação imediata em um Servo Motor (SG90) simulando a locomoção do robô, com trava de segurança (Botão de Emergência).
- **Sensoriamento Ambiental:** Coleta de dados de temperatura e umidade (DHT22), luminosidade (LDR) e presença/movimento (PIR).
- **Algoritmo de Probabilidade de Vida:** Avaliação local das condições do ambiente. Se a probabilidade calculada for superior a 75%, o robô entra em estado de Alerta.
- **Telemetria na Nuvem (AWS):** Envio dos dados estruturados em JSON via protocolo MQTT com criptografia TLS para o AWS IoT Core, que roteia as mensagens automaticamente para o Amazon DynamoDB.
- **Alertas via WhatsApp:** Notificações disparadas automaticamente via requisição HTTP POST para a API do CallMeBot em situações de alta probabilidade de vida.

---

## 🛠️ Arquitetura de Hardware (Pinout)

Todo o sistema foi consolidado em uma única placa **ESP32-S3**.

**Módulo Controle Remoto:**
- **Joystick X:** GPIO 4
- **Joystick Y:** GPIO 5
- **Botão (Trava de Segurança):** GPIO 6
- **LED Verde (Operando):** GPIO 8
- **LED Vermelho (Desligado):** GPIO 7

**Módulo Robô Explorador:**
- **Servo Motor (SG90):** GPIO 13 (PWM)
- **Sensor DHT22:** GPIO 12
- **Sensor PIR:** GPIO 10
- **Sensor LDR:** GPIO 9
- **LED Amarelo (Ligado):** GPIO 15
- **LED Branco (Alerta/Trava):** GPIO 16
- **LED Verde (Exploração Normal):** GPIO 17
- **LED Vermelho (Prob. Vida > 75%):** GPIO 18

---

## ☁️ Arquitetura AWS e Banco de Dados

Os dados são enviados no seguinte formato JSON para o tópico `esp32grm/ww/pub`:

```json
{
  "ID": "2",
  "timestamp": "2025-09-02T14:35:00Z",
  "temperatura_c": 24.3,
  "umidade_pct": 55.0,
  "luminosidade": 723,
  "presenca": 1,
  "probabilidade_vida": 78.0
}
