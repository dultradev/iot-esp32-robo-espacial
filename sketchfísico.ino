// Projeto: Controle Remoto + Robô Explorador (Single Board)
// Integração: Sensores, CallMeBot (HTTP) e AWS IoT Core (MQTT TLS)

#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
// ==========================================
// CONFIGURAÇÕES GERAIS E CREDENCIAIS
// ==========================================

const char* AWS_IOT_PUBLISH_TOPIC = "esp32grm/ww/pub";
const char* AWS_IOT_SUBSCRIBE_TOPIC = "esp32grm/ww/sub";

// ==========================================
// DEFINIÇÃO DOS PINOS E OBJETOS
// ==========================================
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

DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;
WiFiClientSecure net;
PubSubClient client(net);

// Variáveis de Controle
bool roboLigado = true;
bool estadoBotaoAnterior = HIGH;
bool alertaEnviado = false; 

unsigned long tempoAnteriorSensores = 0;
const long intervaloSensores = 2000; 

const int LIMITE_INFERIOR = 1500; 
const int LIMITE_SUPERIOR = 2600; 
int limLuz = 2000; 
int probVida = 0;
unsigned long msgID = 1; // ID do payload AWS

// ==========================================
// FUNÇÕES AWS IOT CORE
// ==========================================
String getTimestamp() {
  time_t now = time(nullptr);
  struct tm* t = gmtime(&now);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
  return String(buf);
}

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("[AWS] Mensagem recebida no tópico: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  
  if (message) {
    Serial.print("[AWS] Comando recebido: ");
    Serial.println(message);
    
    // Integração: Permite que o AWS desligue o robô remotamente também
    String comando = String(message);
    if (comando == "DESLIGAR") {
      roboLigado = false;
      atualizarLEDsControle();
      Serial.println("[SISTEMA] Trava de segurança acionada via Nuvem!");
    } else if (comando == "LIGAR") {
      roboLigado = true;
      atualizarLEDsControle();
    }
  }
}

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[REDE] Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_ROBO_AMARELO, !digitalRead(LED_ROBO_AMARELO));
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_ROBO_AMARELO, HIGH);
  Serial.println("\n[REDE] Wi-Fi conectado!");

  // Sincroniza NTP (Crucial para o certificado TLS)
  configTime(0, 0, "pool.ntp.org");
  Serial.print("[REDE] Sincronizando horário NTP");
  while (time(nullptr) < 1000000000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[REDE] Horário sincronizado!");

  // Configuração dos Certificados
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageHandler);

  Serial.print("[AWS] Conectando ao IoT Core");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(500);
  }

  if (client.connected()) {
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    Serial.println("\n[AWS] Conectado e inscrito no tópico de comandos!");
  } else {
    Serial.println("\n[AWS] Falha na conexão!");
  }
}

void publishMessage(float temp, float umid, int luz, bool presenca, int probVida) {
  if (!client.connected()) return; // Não tenta publicar se estiver offline

  StaticJsonDocument<256> doc;
  doc["ID"]                 = String(msgID++); 
  doc["timestamp"]          = getTimestamp();  
  doc["temperatura_c"]      = temp;
  doc["umidade_pct"]        = umid;
  doc["luminosidade"]       = luz;
  doc["presenca"]           = presenca ? 1 : 0; 
  doc["probabilidade_vida"] = probVida;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.print("[AWS] Dados enviados → ID: ");
  Serial.print(msgID - 1);
  Serial.print(" | Prob. Vida: ");
  Serial.print(probVida);
  Serial.println("%");
}

void reconnectMQTT() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    Serial.println("[AWS] Tentando reconectar ao IoT Core...");
    if (client.connect(THINGNAME)) {
      client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
      Serial.println("[AWS] Reconectado com sucesso.");
    }
  }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000); 
  
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  motorServo.setPeriodHertz(50); 
  motorServo.attach(PIN_SERVO, 500, 2400); 
  motorServo.write(90); 
  
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(PIN_LED_CR_VERDE, OUTPUT);
  pinMode(PIN_LED_CR_VERMELHO, OUTPUT);
  pinMode(LED_ROBO_AMARELO, OUTPUT);
  pinMode(LED_ROBO_BRANCO, OUTPUT);
  pinMode(LED_ROBO_VERDE, OUTPUT);
  pinMode(LED_ROBO_VERMELHO, OUTPUT);

  atualizarLEDsControle();
  
  Serial.println("\n--- INICIALIZANDO SISTEMA E NUVEM ---");
  connectAWS(); // Inicia Wi-Fi, NTP e AWS IoT
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================
void loop() {
  // Mantém a conexão MQTT viva e checa novas mensagens
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop(); 

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
// LÓGICA LOCAL DO ROBÔ E CONTROLE
// ==========================================
void verificarBotao() {
  bool estadoBotaoAtual = digitalRead(PIN_BOTAO);
  if (estadoBotaoAtual == LOW && estadoBotaoAnterior == HIGH) {
    delay(50); 
    if (digitalRead(PIN_BOTAO) == LOW) {
      roboLigado = !roboLigado; 
      Serial.println(roboLigado ? "[CONTROLE] Comando: LIGAR" : "[CONTROLE] Comando: DESLIGAR");
      if(roboLigado) tempoAnteriorSensores = millis() - intervaloSensores; 
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
  digitalWrite(PIN_LED_CR_VERDE, roboLigado ? HIGH : LOW);
  digitalWrite(PIN_LED_CR_VERMELHO, roboLigado ? LOW : HIGH);
}

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

  Serial.println("\n[ROBÔ] --- Relatório ---");
  Serial.print("Temp: "); Serial.print(temp); Serial.print("C | Umi: "); Serial.print(umi); Serial.println("%");
  
  // Publica a telemetria na AWS
  publishMessage(temp, umi, luz, presenca, probVida);

  // Aciona o WhatsApp se passar do gatilho
  if (probVida > 75) {
    if (!alertaEnviado) {
      enviarMensagemWhatsApp("Alerta! Alta probabilidade de vida detectada no planeta.");
      alertaEnviado = true; 
    }
  } else {
    alertaEnviado = false; 
  }
}

void atualizarLEDsRobo(String estado) {
  digitalWrite(LED_ROBO_AMARELO, LOW);
  digitalWrite(LED_ROBO_BRANCO, LOW);
  digitalWrite(LED_ROBO_VERDE, LOW);
  digitalWrite(LED_ROBO_VERMELHO, LOW);

  if (estado == "Desligado") {
    digitalWrite(LED_ROBO_BRANCO, HIGH);
  } else if (estado == "Ligado") {
    digitalWrite(LED_ROBO_AMARELO, HIGH); 
    digitalWrite(LED_ROBO_VERDE, HIGH);   
  } else if (estado == "Alerta") {
    digitalWrite(LED_ROBO_AMARELO, HIGH); 
    digitalWrite(LED_ROBO_BRANCO, HIGH);  
    digitalWrite(LED_ROBO_VERMELHO, HIGH); 
  }
}

void enviarMensagemWhatsApp(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
    WiFiClient clientHTTP;
    HTTPClient http;
    
    http.begin(clientHTTP, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    int httpResponseCode = http.POST("");
    if (httpResponseCode == 200) {
      Serial.println("[REDE] WhatsApp enviado com sucesso!");
    } else {
      Serial.print("[REDE] Erro WhatsApp: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
