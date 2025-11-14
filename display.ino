#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variáveis vindas do principal
extern float temperatura, umidade;
extern bool releLigado, wifiAnterior, telegramOnline;
extern String statusServidor;
extern int telaAtual;
extern String logs[];
extern int logIndex;
extern String uptimeHMS();
extern unsigned long ultimaRequisicaoAPI;
extern int ultimoPingAPI;

unsigned long ultimaAtualizacao = 0;

// ==========================
// INICIAR DISPLAY
// ==========================
void iniciarDisplay() {
  Wire.begin(21, 22);
  Wire.setClock(400000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha no OLED!");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.println("IoT OK");
  display.display();
  delay(1200);
}

// ==========================
// TELAS DO SISTEMA
// ==========================
void telaSensores() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Sensor Ambiente");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setTextSize(3);
  display.setCursor(10, 18);
  display.printf("%.1fC", temperatura);
  display.setTextSize(2);
  display.setCursor(10, 50);
  display.printf("U:%.0f%%", umidade);
  display.display();
}

void telaSistema() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Status do Sistema");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.printf("Wi-Fi: %s", wifiAnterior ? "OK" : "Falha");
  display.setCursor(0, 30);
  display.printf("Rele: %s", releLigado ? "Ligado" : "Desligado");
  display.setCursor(0, 42);
  if (WiFi.status() == WL_CONNECTED)
    display.printf("Sinal: %ld dBm", WiFi.RSSI());
  else
    display.printf("Sinal: ---");
  display.setCursor(0, 54);
  if (WiFi.status() == WL_CONNECTED)
    display.printf("IP: %s", WiFi.localIP().toString().c_str());
  else
    display.printf("IP: ---");
  display.display();
}

void telaComunicacao() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Comunicacao IoT");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setCursor(0, 18);
  display.printf("API: %s", statusServidor.c_str());
  display.setCursor(0, 30);
  display.printf("Telegram: %s", telegramOnline ? "Online" : "Offline");
  display.setCursor(0, 42);
  display.printf("Uptime: %s", uptimeHMS().c_str());
  display.display();
}

void telaLogs() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setCursor(0, 0);
  display.println("Logs Recentes");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  int y = 14;
  for (int i = 0; i < logIndex && y < 64; i++) {
    display.setCursor(0, y);
    String linha = logs[i];
    if (linha.length() > 21)
      linha = linha.substring(0, 21);
    display.println(linha);
    y += 10;
  }
  display.display();
}

void telaDiagnostico() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Diagnostico ESP32");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setCursor(0, 18);
  display.printf("MAC: %s", WiFi.macAddress().c_str());
  display.setCursor(0, 30);
  display.printf("Host: %s", WiFi.getHostname());
  display.setCursor(0, 42);
  display.printf("CPU: %u MHz", getCpuFrequencyMhz());
  display.setCursor(0, 54);
  display.printf("Heap: %u KB", ESP.getFreeHeap() / 1024);
  display.display();
}

void telaAPIinfo() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Status da API");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setCursor(0, 18);
  display.printf("Ult. envio: %lus", (millis() - ultimaRequisicaoAPI) / 1000UL);
  display.setCursor(0, 30);
  display.printf("Ping: %d ms", ultimoPingAPI);
  display.setCursor(0, 42);
  display.printf("Servidor: %s", statusServidor.c_str());
  display.display();
}

// ==========================
// ⚡ Tela de ação do Telegram (com fundo piscando e nome maior)
// ==========================
void telaAcaoTelegram(const String& usuario, bool ligado) {
  const int duracao = 3000;  // tempo total em ms (3 segundos)
  unsigned long inicio = millis();

  // Texto principal
  String titulo = ligado ? "RELE LIGADO" : "RELE DESLIGADO";
  String origem = "via Telegram";
  String nome = usuario;

  while (millis() - inicio < duracao) {
    // alterna o fundo a cada 300ms
    bool fundoBranco = ((millis() / 300) % 2 == 0);

    display.clearDisplay();

    // fundo piscando
    if (fundoBranco) {
      display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
      display.setTextColor(SSD1306_WHITE);
    }

    // ====== título ======
    display.setTextSize(1); // menor que antes
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(titulo, 0, 0, &x1, &y1, &w, &h);
    int cx = (SCREEN_WIDTH - w) / 2;
    display.setCursor(cx, 8);
    display.print(titulo);

    // ====== subtítulo ======
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 72) / 2, 25);
    display.print(origem);

    // ====== nome do usuário (maior destaque) ======
    display.setTextSize(2);
    display.getTextBounds(nome, 0, 0, &x1, &y1, &w, &h);
    cx = (SCREEN_WIDTH - w) / 2;
    display.setCursor(cx, 43);
    display.print(nome);

    display.display();
    delay(100);
  }

  desenharTela(telaAtual);
}

// ==========================
// MUDANÇA DE TELAS
// ==========================
void desenharTela(int tela) {
  if (millis() - ultimaAtualizacao < 400) return;
  ultimaAtualizacao = millis();

  switch (tela) {
    case 0: telaSensores(); break;
    case 1: telaSistema(); break;
    case 2: telaComunicacao(); break;
    case 3: telaLogs(); break;
    case 4: telaDiagnostico(); break;
    case 5: telaAPIinfo(); break;
    default: tela = 0; telaSensores(); break;
  }
}

// ==========================
// BOTÃO DE TROCA
// ==========================
#define BUTTON_PIN 14
void atualizarTelaComBotao() {
  static int ultimo = HIGH;
  int estado = digitalRead(BUTTON_PIN);

  if (estado == LOW && ultimo == HIGH) {
    telaAtual++;
    if (telaAtual > 5) telaAtual = 0;
    desenharTela(telaAtual);
  }
  ultimo = estado;
}
