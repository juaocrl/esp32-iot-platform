#include <HTTPClient.h>
#include <WiFi.h>

// 🔗 Variáveis vindas do principal
extern float temperatura, umidade;
extern String statusServidor;
extern unsigned long ultimaRequisicaoAPI;
extern int ultimoPingAPI;
extern void addLog(const String& msg);
extern const char* SERVER_URL;

// 🆔 UUID real do sensor cadastrado no banco
#define SENSOR_ID "1"

// ==========================
// FUNÇÃO — Envio para API
// ==========================
void enviarParaAPI() {
  // 🚫 Verifica conexão Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    statusServidor = "Sem Wi-Fi";
    addLog("API não enviada (sem Wi-Fi)");
    return;
  }

  HTTPClient http;
  unsigned long inicio = millis();

  // 🌐 Monta requisição
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  // 📦 JSON do envio
  String json = "{";
  json += "\"sensor_id\":\"" + String(SENSOR_ID) + "\",";
  json += "\"temperature\":" + String(temperatura, 1) + ",";
  json += "\"humidity\":" + String(umidade, 1);
  json += "}";

  // 🚀 Envia POST
  int code = http.POST(json);
  ultimoPingAPI = millis() - inicio;
  ultimaRequisicaoAPI = millis();

  // ✅ Resultado
  if (code == 200) {
    statusServidor = "OK";
    addLog("Envio API OK (" + String(ultimoPingAPI) + " ms)");
  } 
  else if (code == 400) {
    statusServidor = "Requisição inválida (400)";
    addLog("Erro API 400 (" + String(ultimoPingAPI) + " ms)");
  } 
  else if (code == 404) {
    statusServidor = "Endpoint não encontrado (404)";
    addLog("Erro API 404 (" + String(ultimoPingAPI) + " ms)");
  }
  else if (code == 500) {
    statusServidor = "Erro interno (500)";
    addLog("Erro API 500 (" + String(ultimoPingAPI) + " ms)");
  } 
  else if (code > 0) {
    statusServidor = "Erro HTTP " + String(code);
    addLog("Erro API " + String(code) + " (" + String(ultimoPingAPI) + " ms)");
  } 
  else {
    statusServidor = "Falha de conexão";
    addLog("Falha ao enviar API (" + String(ultimoPingAPI) + " ms)");
  }

  http.end();
}
