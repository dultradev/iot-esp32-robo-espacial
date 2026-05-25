// Projeto: Controle Remoto + Robô Explorador (Single Board)
// Etapa Atual: Integração Wi-Fi + Alertas WhatsApp (Padrão Oficial CallMeBot)

#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h> // Biblioteca necessária para formatar a URL

// ==========================================
// CONFIGURAÇÕES DE REDE E API
// ==========================================
const char* ssid = "";
const char* password = "";

// Configure com o código do país (+55), DDD e número
String phoneNumber = ""; 
String apiKey = "";

// ==========================================
// DEFINIÇÃO DOS PINOS
// ==========================================
// --- Controle Remoto ---
// --- Controle Remoto ---
#define PIN_JOY_X 4
#define PIN_JOY_Y 5
#define PIN_BOTAO 6
#define PIN_LED_CR_VERDE 8
#define PIN_LED_CR_VERMELHO 7

// --- Robô Explorador ---
#define LDR_PIN 11
#define PIR_PIN 10
#define PIN_SERVO 17
#define DHTPIN 12
#define DHTTYPE DHT22 

#define LED_ROBO_AMARELO 15
#define LED_ROBO_BRANCO 16
#define LED_ROBO_VERDE 17
#define LED_ROBO_VERMELHO 18
// ==========================================
// OBJETOS E VARIÁVEIS GLOBAIS
// ==========================================
DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;

bool roboLigado = true;
bool estadoBotaoAnterior = HIGH;
bool alertaEnviado = false; 

unsigned long tempoAnteriorSensores = 0;
const long intervaloSensores = 2000; 

const int LIMITE_INFERIOR = 1500; 
const int LIMITE_SUPERIOR = 2600; 
int limLuz = 2000; 
int probVida = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000); 
  
  // --- Configuração Servo ---
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  motorServo.setPeriodHertz(50); 
  motorServo.attach(PIN_SERVO, 500, 2400); 
  motorServo.write(90); 
  
  // --- Configuração Pinos Entradas ---
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  // --- Configuração Pinos Saídas ---
  pinMode(PIN_LED_CR_VERDE, OUTPUT);
  pinMode(PIN_LED_CR_VERMELHO, OUTPUT);
  pinMode(LED_ROBO_AMARELO, OUTPUT);
  pinMode(LED_ROBO_BRANCO, OUTPUT);
  pinMode(LED_ROBO_VERDE, OUTPUT);
  pinMode(LED_ROBO_VERMELHO, OUTPUT);

  atualizarLEDsControle();
  
  Serial.println("\n--- INICIALIZANDO SISTEMA ---");
  
  // --- Conexão Wi-Fi ---
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_ROBO_AMARELO, !digitalRead(LED_ROBO_AMARELO));
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite(LED_ROBO_AMARELO, HIGH);
  Serial.println("\n[REDE] Conectado com sucesso!");
  Serial.print("[REDE] IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  verificarBotao();

  if (roboLigado) {
    mapearJoystickParaMotor();
    
    unsigned long tempoAtual = millis();
    if (tempoAtual - tempoAnteriorSensores >= intervaloSensores) {
      tempoAnteriorSensores = tempoAtual;
      lerSensoresECalcular();
    }
  } 
  else {
    motorServo.write(90); 
    atualizarLEDsRobo("Desligado");
    
    unsigned long tempoAtual = millis();
    if (tempoAtual - tempoAnteriorSensores >= intervaloSensores) {
      tempoAnteriorSensores = tempoAtual;
      Serial.println("[CR] Status: Robô DESLIGADO remotamente.");
    }
  }

  delay(50); 
}

// ==========================================
// FUNÇÕES DO CONTROLE REMOTO
// ==========================================
void verificarBotao() {
  bool estadoBotaoAtual = digitalRead(PIN_BOTAO);

  if (estadoBotaoAtual == LOW && estadoBotaoAnterior == HIGH) {
    delay(50); 
    if (digitalRead(PIN_BOTAO) == LOW) {
      roboLigado = !roboLigado; 
      
      Serial.println("\n------------------------------------------------");
      if (!roboLigado) {
        Serial.println("[CONTROLE] Comando enviado: DESLIGAR");
      } else {
        Serial.println("[CONTROLE] Comando enviado: LIGAR");
        tempoAnteriorSensores = millis() - intervaloSensores; 
      }
      Serial.println("------------------------------------------------\n");
      
      atualizarLEDsControle();
    }
  }
  estadoBotaoAnterior = estadoBotaoAtual;
}

void mapearJoystickParaMotor() {
  int valorX = analogRead(PIN_JOY_X);
  int valorY = analogRead(PIN_JOY_Y);

  if (valorY < LIMITE_INFERIOR) { motorServo.write(90); } 
  else if (valorY > LIMITE_SUPERIOR) { motorServo.write(90); } 
  else if (valorX < LIMITE_INFERIOR) { motorServo.write(180); } 
  else if (valorX > LIMITE_SUPERIOR) { motorServo.write(0); } 
  else { motorServo.write(90); }
}

void atualizarLEDsControle() {
  if (roboLigado) {
    digitalWrite(PIN_LED_CR_VERDE, HIGH);
    digitalWrite(PIN_LED_CR_VERMELHO, LOW);
  } else {
    digitalWrite(PIN_LED_CR_VERDE, LOW);
    digitalWrite(PIN_LED_CR_VERMELHO, HIGH);
  }
}

// ==========================================
// FUNÇÕES DO ROBÔ EXPLORADOR
// ==========================================
void lerSensoresECalcular() {
  float temp = dht.readTemperature();
  float umi = dht.readHumidity();
  int luz = analogRead(LDR_PIN);
  bool presenca = digitalRead(PIR_PIN) == HIGH;
  
  if (isnan(temp) || isnan(umi)) { temp = 0; umi = 0; }

  probVida = 0;
  if (temp >= 15.0 && temp <= 30.0) probVida += 25;
  if (umi >= 40.0 && umi <= 70.0) probVida += 25;
  if (luz > limLuz) probVida += 20;
  if (presenca) probVida += 30;
  if (probVida > 100) probVida = 100; 

  String estadoRobo = (probVida > 75) ? "Alerta" : "Ligado";
  atualizarLEDsRobo(estadoRobo);

  Serial.println("\n[ROBÔ] --- Relatório de Sensores ---");
  Serial.print("Temp: "); Serial.print(temp); Serial.print("C | Umi: "); Serial.print(umi); Serial.println("%");
  Serial.print("Luz: "); Serial.print(luz); Serial.print(" | PIR: "); Serial.println(presenca ? "Detectada" : "Livre");
  Serial.print(">>> Probabilidade de Vida: "); Serial.print(probVida); Serial.println("%");

  if (probVida <= 75) {
    Serial.println("[ROBÔ] Exploração normal. (LEDs Amarelo e Verde)");
    alertaEnviado = false; 
  } else {
    Serial.println("[ROBÔ] ALERTA! Alta probabilidade (>75%). (LEDs Amarelo, Branco e Vermelho)");
    
    if (!alertaEnviado) {
      // Passa a mensagem bruta, a função de rede fará o UrlEncode
      enviarMensagemWhatsApp("Alerta! Alta probabilidade de vida detectada no planeta.");
      alertaEnviado = true; 
    } else {
      Serial.println("[REDE] Alerta já enviado anteriormente. Aguardando normalização.");
    }
  }
}

void atualizarLEDsRobo(String estado) {
  digitalWrite(LED_ROBO_AMARELO, LOW);
  digitalWrite(LED_ROBO_BRANCO, LOW);
  digitalWrite(LED_ROBO_VERDE, LOW);
  digitalWrite(LED_ROBO_VERMELHO, LOW);

  if (estado == "Desligado") {
    digitalWrite(LED_ROBO_BRANCO, HIGH);
  } 
  else if (estado == "Ligado") {
    digitalWrite(LED_ROBO_AMARELO, HIGH); 
    digitalWrite(LED_ROBO_VERDE, HIGH);   
  } 
  else if (estado == "Alerta") {
    digitalWrite(LED_ROBO_AMARELO, HIGH); 
    digitalWrite(LED_ROBO_BRANCO, HIGH);  
    digitalWrite(LED_ROBO_VERMELHO, HIGH); 
  }
}

// ==========================================
// FUNÇÃO DE REDE (HTTP POST - CallMeBot Doc)
// ==========================================
void enviarMensagemWhatsApp(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    
    // Constrói a URL usando http:// e aplicando urlEncode na mensagem
    String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
    
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("[REDE] Iniciando conexão com a API do WhatsApp...");
    http.begin(client, url);

    // Adiciona o cabeçalho especificado pela documentação
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Envia a requisição POST (parâmetros já estão na URL)
    int httpResponseCode = http.POST("");
    
    if (httpResponseCode == 200) {
      Serial.println("[REDE] Mensagem enviada com sucesso ao WhatsApp!");
    } else {
      Serial.println("[REDE] Erro ao enviar a mensagem.");
      Serial.print("HTTP response code: ");
      Serial.println(httpResponseCode);
    }

    // Libera os recursos de rede
    http.end();
  } else {
    Serial.println("[REDE] Erro: Wi-Fi desconectado.");
  }
}
