extern bool wifiAnterior;
extern void addLog(const String& msg);
extern const char* const ssid;
extern const char* const password;

void conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  Serial.printf("Conectando a %s", ssid);
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiAnterior = true;
    addLog("Wi-Fi conectado");
    Serial.println(WiFi.localIP());
  } else {
    wifiAnterior = false;
    addLog("Falha Wi-Fi");
  }
}
