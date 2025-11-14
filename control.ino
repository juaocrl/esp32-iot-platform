#include <DHT.h>
#include <UniversalTelegramBot.h>

// ==========================
// VARIÁVEIS GLOBAIS
// ==========================
extern float temperatura, umidade;
extern bool releLigado, manualOverride;
extern UniversalTelegramBot bot;
extern const long long allowedUsers[];
extern const int NUM_ALLOWED;
extern void addLog(const String& msg);

// 🔥 Limites configuráveis via Telegram
extern float LIMITE_LIGA;  // temperatura para ligar os relés
extern float LIMITE_DESL;  // temperatura para desligar os relés

// ==========================
// HARDWARE
// ==========================
#define RELAY1 13
#define RELAY2 27
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ==========================
// ENVIO PARA TODOS (você + grupo)
// ==========================
static inline void notifyAll(const String& msg) {
  for (int i = 0; i < NUM_ALLOWED; i++) {
    bot.sendMessage(String((long long)allowedUsers[i]), msg, "");
  }
}

// ==========================
// CONTROLE AUTOMÁTICO
// ==========================
void controleAutomatico(float temp) {
  // Se o modo manual estiver ativo, o controle automático é ignorado
  if (manualOverride) return;

  // === CONDIÇÃO PARA LIGAR ===
  if (!releLigado && temp >= LIMITE_LIGA) {
    releLigado = true;
    digitalWrite(RELAY1, RELAY_ON);
    digitalWrite(RELAY2, RELAY_ON);

    addLog("Temperatura alta -> Rele ON");
    String msg = "🔥 ALERTA! Temperatura ≥ " + String(LIMITE_LIGA, 1) + "°C. Relés ligados.";
    notifyAll(msg);
  }

  // === CONDIÇÃO PARA DESLIGAR ===
  else if (releLigado && temp <= LIMITE_DESL) {
    releLigado = false;
    digitalWrite(RELAY1, RELAY_OFF);
    digitalWrite(RELAY2, RELAY_OFF);

    addLog("Temperatura normal -> Rele OFF");
    String msg = "✅ Temperatura ≤ " + String(LIMITE_DESL, 1) + "°C. Relés desligados.";
    notifyAll(msg);
  }
}
