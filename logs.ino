#include <Arduino.h>

extern String logs[];
extern int logIndex;
#define MAX_LOGS 6

String ultimoTipoLog = "";

void addLog(const String& msg) {
  // Remove números entre parênteses para comparar apenas o "tipo" da mensagem
  String tipoAtual = msg;
  int abre = tipoAtual.indexOf('(');
  if (abre > 0) tipoAtual = tipoAtual.substring(0, abre);
  tipoAtual.trim();

  // Ignora logs repetidos do mesmo tipo
  if (tipoAtual == ultimoTipoLog) return;
  ultimoTipoLog = tipoAtual;

  // Desloca e adiciona novo log
  for (int i = MAX_LOGS - 1; i > 0; i--) logs[i] = logs[i - 1];
  logs[0] = msg;

  if (logIndex < MAX_LOGS) logIndex++;
  Serial.println("[LOG] " + msg);
}

String uptimeHMS() {
  unsigned long s = millis() / 1000;
  unsigned long m = s / 60;
  unsigned long h = m / 60;
  s %= 60; m %= 60;
  char buf[20];
  sprintf(buf, "%02lu:%02lu:%02lu", h, m, s);
  return String(buf);
}
