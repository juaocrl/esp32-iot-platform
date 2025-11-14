#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <Preferences.h>

// ==========================
// Persistência dos limites (NVS)
// ==========================
Preferences prefs;

// Limites padrão (podem ser ajustados via Telegram)
float LIMITE_LIGA = 27.0;  // liga relé quando temp >= LIMITE_LIGA
float LIMITE_DESL = 27.;  // desliga relé quando temp <= LIMITE_DESL

static bool limitesCarregados = false;

void carregarLimites() {
  prefs.begin("ctrltemp", true);
  float on  = prefs.getFloat("on",  -1000.0);
  float off = prefs.getFloat("off", -1000.0);
  prefs.end();
  if (on  > -999.0) LIMITE_LIGA = on;
  if (off > -999.0) LIMITE_DESL = off;
}

void salvarLimites() {
  prefs.begin("ctrltemp", false);
  prefs.putFloat("on",  LIMITE_LIGA);
  prefs.putFloat("off", LIMITE_DESL);
  prefs.end();
}

// ==========================
// Estado do diálogo interativo
// ==========================
long long sessaoChatId   = 0;     // qual chat está em sessão
bool aguardandoLiga      = false; // aguardando valor do "Liga"
bool aguardandoDesliga   = false; // aguardando valor do "Desliga"
float bufferLiga         = 0.0;   // armazena o valor de "Liga" até receber o "Desliga"

// Helper para validar float simples
bool ehNumeroFloat(const String& s) {
  if (s.length() == 0) return false;
  bool ponto = false; int start = 0;
  if (s[0] == '-' || s[0] == '+') start = 1;
  for (int i = start; i < s.length(); i++) {
    if (s[i] == '.') { if (ponto) return false; ponto = true; }
    else if (!isDigit(s[i])) return false;
  }
  return true;
}

// ==========================
// Declarações externas já existentes no seu projeto
// ==========================
extern UniversalTelegramBot bot;
extern bool releLigado, manualOverride, wifiAnterior, telegramOnline;
extern float temperatura, umidade;
extern String statusServidor;
extern int logIndex;
extern String logs[];
extern const long long allowedUsers[];
extern const int NUM_ALLOWED;
extern void addLog(const String& msg);
extern String uptimeHMS();
extern void telaAcaoTelegram(const String& usuario, bool ligado);

// Apenas para manter compatibilidade com seu controle automático:
// (o controle usa LIMITE_LIGA / LIMITE_DESL definidos acima)

// ==========================
// Autorização
// ==========================
bool autorizado(long long chat) {
  for (int i = 0; i < NUM_ALLOWED; i++)
    if (allowedUsers[i] == chat) return true;
  return false;
}

// ==========================
// Handler principal de mensagens
// ==========================
void tratarMensagens(int n) {
  // Lazy init dos limites (carrega 1x ao primeiro uso do handler)
  if (!limitesCarregados) {
    carregarLimites();
    limitesCarregados = true;
  }

  for (int i = 0; i < n; i++) {
    long long chat = atoll(bot.messages[i].chat_id.c_str());
    String cid = String(chat);
    String txt = bot.messages[i].text;
    String user = bot.messages[i].from_name;

    // Remove menção ao nome do bot (ex: /ligar@seu_bot → /ligar)
    int arroba = txt.indexOf('@');
    if (arroba > 0) txt = txt.substring(0, arroba);

    txt.trim();

    if (!autorizado(chat)) {
      bot.sendMessage(cid, "🚫 Acesso negado.", "");
      continue;
    }

    // ============ Fluxo interativo (coleta respostas numéricas) ============
    // Se há sessão ativa para este chat e a mensagem NÃO é comando:
    if (sessaoChatId == chat && txt.length() > 0 && txt[0] != '/') {
      // 1ª pergunta: valor de LIGAR
      if (aguardandoLiga) {
        String s = txt; s.trim();
        if (!ehNumeroFloat(s)) {
          bot.sendMessage(cid, "❌ Valor inválido. Envie um número, ex.: 28.0\nOu /cancelar", "");
          continue;
        }
        bufferLiga = s.toFloat();
        if (bufferLiga < -20 || bufferLiga > 100) {
          bot.sendMessage(cid, "❌ Fora do intervalo esperado. Tente algo entre -20 e 100 °C.\nOu /cancelar", "");
          continue;
        }

        aguardandoLiga = false;
        aguardandoDesliga = true;
        bot.sendMessage(cid,
          "👍 Valor de *LIGAR* recebido: " + String(bufferLiga,1) + " °C\n\n"
          "Agora digite o valor para *DESLIGAR* o relé (°C).\n"
          "_Ex.: 26.8_\n\n"
          "Dica: o valor de desligar deve ser *menor* que o de ligar.",
          "Markdown");
        continue;
      }

      // 2ª pergunta: valor de DESLIGAR
      if (aguardandoDesliga) {
        String s = txt; s.trim();
        if (!ehNumeroFloat(s)) {
          bot.sendMessage(cid, "❌ Valor inválido. Envie um número, ex.: 26.8\nOu /cancelar", "");
          continue;
        }
        float novoDesliga = s.toFloat();
        if (novoDesliga < -20 || novoDesliga > 100) {
          bot.sendMessage(cid, "❌ Fora do intervalo esperado. Tente algo entre -20 e 100 °C.\nOu /cancelar", "");
          continue;
        }
        if (novoDesliga >= bufferLiga) {
          bot.sendMessage(cid,
            "⚠️ O valor de *DESLIGAR* deve ser menor que o de *LIGAR*.\n"
            "Digite novamente o valor de *DESLIGAR* (°C), ex.: 26.8\nOu /cancelar",
            "Markdown");
          continue;
        }

        // ✅ aplica e persiste
        LIMITE_LIGA = bufferLiga;
        LIMITE_DESL = novoDesliga;
        salvarLimites();

        aguardandoDesliga = false;
        sessaoChatId = 0;

        bot.sendMessage(cid,
          "✅ *Limites atualizados!*\n"
          "Liga: " + String(LIMITE_LIGA,1) + " °C\n"
          "Desliga: " + String(LIMITE_DESL,1) + " °C\n\n"
          "Use /ajustartemp novamente se quiser alterar de novo.\n"
          "Para ver: /gettemp",
          "Markdown");
        continue;
      }
    }

    // ============ Comandos ============
    if (txt == "/start") {
      bot.sendMessage(cid,
        "🤖 *Comandos Disponíveis*\n"
        "-----------------------------\n"
        "/status - status do sistema\n"
        "/ligar - ligar relés (manual)\n"
        "/desligar - desligar relés (manual)\n"
        "/auto - modo automático\n"
        "/logs - últimos eventos\n"
        "/gettemp - ver limites atuais\n"
        "/ajustartemp - ajustar limites por diálogo\n"
        "/cancelar - cancela o ajuste em andamento\n",
        "Markdown");
    }

    else if (txt == "/status") {
      String msg =
        "📡 *Status do Sistema IoT*\n"
        "-----------------------------\n"
        "📶 Wi-Fi: " + String(wifiAnterior ? "✅ Conectado" : "🚫 Desconectado") + "\n" +
        "🌐 IP Local: " + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "N/D") + "\n" +
        "📈 Sinal Wi-Fi: " + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0) + " dBm\n" +
        "🤖 Telegram: " + String(telegramOnline ? "✅ Online" : "⚠️ Offline") + "\n" +
        "🌡️ Temperatura: " + String(temperatura,1) + " °C\n" +
        "💧 Umidade: " + String(umidade,1) + " %\n" +
        "⚙️ Relés: " + String(releLigado ? "🟢 Ligados" : "🔴 Desligados") + "\n" +
        "🔗 API: " + statusServidor + "\n" +
        "🔁 Modo: " + String(manualOverride ? "Manual" : "Automático") + "\n" +
        "⏱️ Uptime: " + uptimeHMS() + "\n"
        "-----------------------------";
      bot.sendMessage(cid, msg, "Markdown");
    }

    else if (txt == "/logs") {
      String msg = "📜 *Últimos Logs:*\n";
      for (int j = 0; j < logIndex; j++) msg += "• " + logs[j] + "\n";
      bot.sendMessage(cid, msg, "Markdown");
    }

    else if (txt == "/ligar") {
      manualOverride = true;
      releLigado = true;
      digitalWrite(13, LOW);
      digitalWrite(27, LOW);
      addLog("Relé ligado via Telegram (" + user + ")");
      telaAcaoTelegram(user, true);
      bot.sendMessage(cid, "🟢 Relés LIGADOS por " + user + " (modo manual)", "");
    }

    else if (txt == "/desligar") {
      manualOverride = true;
      releLigado = false;
      digitalWrite(13, HIGH);
      digitalWrite(27, HIGH);
      addLog("Relé desligado via Telegram (" + user + ")");
      telaAcaoTelegram(user, false);
      bot.sendMessage(cid, "🔴 Relés DESLIGADOS por " + user + " (modo manual)", "");
    }

    else if (txt == "/auto") {
      manualOverride = false;
      addLog("Modo automático ativado (" + user + ")");
      bot.sendMessage(cid, "🔁 Modo AUTOMÁTICO ativado por " + user, "");
    }

    // ======= Novos comandos de ajuste =======
    else if (txt == "/gettemp") {
      bot.sendMessage(cid,
        "🌡️ *Limites atuais*\n"
        "Liga: " + String(LIMITE_LIGA,1) + " °C\n"
        "Desliga: " + String(LIMITE_DESL,1) + " °C",
        "Markdown");
    }

    else if (txt == "/ajustartemp") {
      // inicia sessão para este chat
      sessaoChatId   = chat;
      aguardandoLiga = true;
      aguardandoDesliga = false;

      bot.sendMessage(cid,
        "🛠️ Vamos ajustar os limites.\n"
        "Digite o novo valor para *LIGAR* o relé (°C).\n\n"
        "_Ex.: 28.0_",
        "Markdown");
    }

    else if (txt == "/cancelar") {
      if (sessaoChatId == chat && (aguardandoLiga || aguardandoDesliga)) {
        aguardandoLiga = aguardandoDesliga = false;
        sessaoChatId = 0;
        bot.sendMessage(cid, "❎ Ajuste cancelado.", "");
      } else {
        bot.sendMessage(cid, "Não há ajuste em andamento.", "");
      }
    }
  }
}
