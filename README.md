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
