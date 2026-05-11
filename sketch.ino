// Definição dos pinos do ESP32-S3
#define PIN_JOY_X 4
#define PIN_JOY_Y 5
#define PIN_BOTAO 6
#define PIN_LED_VERDE 8
#define PIN_LED_VERMELHO 7

// Variáveis de controle de estado
bool roboLigado = true;
bool estadoBotaoAnterior = HIGH;

// Constantes para calibração do Joystick no Wokwi (0 a 4095)
const int LIMITE_INFERIOR = 1000;
const int LIMITE_SUPERIOR = 3000;

void setup() {
  Serial.begin(115200);
  
  // Configuração dos pinos
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  
  // INPUT_PULLUP usa o resistor interno do ESP32 (aciona em LOW)
  pinMode(PIN_BOTAO, INPUT_PULLUP); 
  
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);

  // Estado inicial
  atualizarLEDs();
  Serial.println("--- Controle Remoto Inicializado ---");
  Serial.println("Status: Robô LIGADO e aguardando comandos.");
}

void loop() {
  verificarBotao();

  // Se o robô estiver ligado, lê o joystick e envia comandos
  if (roboLigado) {
    enviarComandosJoystick();
  } else {
    // Exibe continuamente (a cada 1 segundo) que o robô está desligado
    Serial.println("Status: Robô DESLIGADO.");
    delay(1000); 
    return; // Evita a leitura do joystick enquanto desligado
  }

  // Delay para não sobrecarregar o Monitor Serial
  delay(300); 
}

// Função para ler o botão e alternar o estado do robô
void verificarBotao() {
  bool estadoBotaoAtual = digitalRead(PIN_BOTAO);

  // Verifica se o botão foi pressionado (mudança de HIGH para LOW)
  if (estadoBotaoAtual == LOW && estadoBotaoAnterior == HIGH) {
    delay(50); // Debounce simples
    
    if (digitalRead(PIN_BOTAO) == LOW) {
      roboLigado = !roboLigado; // Alterna o estado para fins de simulação
      
      if (!roboLigado) {
        Serial.println("\n------------------------------");
        Serial.println("Comando enviado: DESLIGAR");
        Serial.println("------------------------------\n");
      } else {
        Serial.println("\n------------------------------");
        Serial.println("Comando enviado: LIGAR");
        Serial.println("------------------------------\n");
      }
      atualizarLEDs();
    }
  }
  estadoBotaoAnterior = estadoBotaoAtual;
}

// Função para ler os eixos e determinar a direção
void enviarComandosJoystick() {
  int valorX = analogRead(PIN_JOY_X);
  int valorY = analogRead(PIN_JOY_Y);

  String comando = "Parado";

  // Lógica de direção (Prioriza o eixo Y)
  // No Wokwi, valores próximos a 0 ou 4095 indicam as extremidades
  if (valorY < LIMITE_INFERIOR) {
    comando = "Frente";
  } 
  else if (valorY > LIMITE_SUPERIOR) {
    comando = "Trás";
  } 
  else if (valorX < LIMITE_INFERIOR) {
    comando = "Esquerda";
  } 
  else if (valorX > LIMITE_SUPERIOR) {
    comando = "Direita";
  }

  // Mostra o comando atual no Monitor Serial
  Serial.print("Comando: ");
  Serial.println(comando);
}

// Função para atualizar os LEDs de status do controle
void atualizarLEDs() {
  if (roboLigado) {
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_VERMELHO, LOW);
  } else {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_VERMELHO, HIGH);
  }
}