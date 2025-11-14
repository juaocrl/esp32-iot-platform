#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "secrets.h"

// ==========================
// VARIÁVEIS GLOBAIS ÚNICAS
// ==========================
float temperatura = 0.0, umidade = 0.0;
bool releLigado = false;
bool manualOverride = false;
bool wifiAnterior = false;
bool telegramOnline = false;
String statusServidor = "Desconhecido";
int telaAtual = 0;

#define MAX_LOGS 6
String logs[MAX_LOGS];
int logIndex = 0;

unsigned long ultimaLeitura = 0;
unsigned long ultimaChecagemBot = 0;
unsigned long ultimaRequisicaoAPI = 0;
int ultimoPingAPI = 0;

// ==========================
// HARDWARE
// ==========================
#define DHTPIN 4
#define DHTTYPE DHT11
#define RELAY1 13
#define RELAY2 27
#define RELAY_ON  LOW
#define RELAY_OFF HIGH
#define BUTTON_PIN 14

DHT dht(DHTPIN, DHTTYPE);

// ==========================
// TELEGRAM / API
// ==========================
const long long allowedUsers[] = { 7536136730LL, -1003024044221LL };
const int NUM_ALLOWED = sizeof(allowedUsers)/sizeof(allowedUsers[0]);

// 🔧 URL REAL DO SERVIDOR
const char* SERVER_URL = "https://sua-api.com/temperaturas";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ==========================
// PROTÓTIPOS
// ==========================
void conectarWiFi();
void enviarParaAPI();
void controleAutomatico(float temp);
void tratarMensagens(int n);
void iniciarDisplay();
void desenharTela(int tela);
void atualizarTelaComBotao();
void addLog(const String &msg);
String uptimeHMS();

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY1, RELAY_OFF);
  digitalWrite(RELAY2, RELAY_OFF);

  iniciarDisplay();
  conectarWiFi();
  client.setInsecure();

  addLog("Sistema iniciado");
  addLog("Bot Telegram inicializando...");
  desenharTela(0);
}

// ==========================
// LOOP PRINCIPAL
// ==========================
void loop() {
  conectarWiFi();
  atualizarTelaComBotao();

  // ======= SENSOR =======
  if (millis() - ultimaLeitura > 5000) {
    ultimaLeitura = millis();

    float t = dht.readTemperature();
    float u = dht.readHumidity();

    if (!isnan(t) && !isnan(u)) {
      temperatura = t;
      umidade = u;
      controleAutomatico(temperatura);
      enviarParaAPI();
      desenharTela(telaAtual);
    } else {
      addLog("Falha no sensor DHT");
    }
  }

  // ======= TELEGRAM =======
  if (millis() - ultimaChecagemBot > 3000) {
    int novas = bot.getUpdates(bot.last_message_received + 1);

    if (novas >= 0) {
      if (!telegramOnline) {
        telegramOnline = true;
        addLog("Telegram online");
      }
      if (novas > 0) tratarMensagens(novas);
    } else {
      if (telegramOnline) {
        telegramOnline = false;
        addLog("Telegram offline");
      }
    }

    ultimaChecagemBot = millis();
  }
}
