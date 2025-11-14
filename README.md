<h1 align="center">🚀 ESP32 IoT Platform – Gateway Inteligente</h1>

<p align="center">
  <strong>Monitoramento ambiental • Automação • Controle remoto via Telegram • Display OLED</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Device-ESP32-blue?style=for-the-badge">
  <img src="https://img.shields.io/badge/Platform-Arduino-orange?style=for-the-badge">
  <img src="https://img.shields.io/badge/Sensor-DHT11/DHT22-green?style=for-the-badge">
  <img src="https://img.shields.io/badge/Control-Relay-red?style=for-the-badge">
  <img src="https://img.shields.io/badge/Telegram-Bot-informational?style=for-the-badge">
</p>

---

## 📌 Sobre o Projeto

Este repositório contém o firmware do meu **gateway IoT baseado em ESP32**, capaz de:

✔️ Monitorar **temperatura e umidade**  
✔️ Controlar um **relé** para automação  
✔️ Exibir informações em um **display OLED (SSD1306)**  
✔️ Enviar e receber comandos via **Telegram Bot**  
✔️ Registrar eventos em um sistema simples de logs  

O intuito é integrar esse dispositivo a uma **plataforma IoT completa**, com backend, dashboard e histórico de dados.

---

## 📡 Diagrama Geral do Sistema

```
 ┌──────────────────────────┐
 │        ESP32 DevKit      │
 │                          │
 │  ┌──────────────┐       │
 │  │  DHT11/22     │------│---- Temperatura / Umidade
 │  └──────────────┘       │
 │                          │
 │  ┌──────────────┐       │
 │  │   Relé        │------│---- Acionamento de carga
 │  └──────────────┘       │
 │                          │
 │  ┌──────────────┐       │
 │  │ OLED 128x64   │------│---- Interface de status
 │  └──────────────┘       │
 │                          │
 │  Wi-Fi 2.4GHz            │
 │        │                 │
 └────────┼─────────────────┘
          │
          ▼
   Telegram Bot (comandos / alertas)
```

---

## 🔧 Funcionalidades

### Monitoramento
- Leitura de **temperatura e umidade**
- Exibição no display OLED
- Logs rotativos

### Automação
- Controle de relé via Telegram
- Possível botão físico (dependendo da versão)
- Futuras regras automáticas

### Telegram Bot
- `/status`
- `/ligar`
- `/desligar`
- `/uptime`
- `/logs`

### Display OLED
- Temperatura
- Umidade
- Relé on/off
- Wi-Fi status
- Telegram status
- Logs

---

## Estrutura do Código

```
📦 esp32-iot-platform
│
├── ESP32_IoT.ino
├── control.ino
├── api.ino
├── network.ino
├── display.ino
├── telegram.ino
├── logs.ino
└── secrets.example.h
```

---

## Hardware Necessário

- ESP32 DevKit  
- Sensor DHT11 ou DHT22  
- Display SSD1306  
- Relé  
- Protoboard / jumpers  
- Fonte 5V  

---

## Configuração de Credenciais

Crie:

```
cp secrets.example.h secrets.h
```

Edite:

```cpp
const char* WIFI_SSID = "SuaRede";
const char* WIFI_PASS = "Senha";
#define BOT_TOKEN "SeuToken"
```

---

## Bibliotecas

- Adafruit SSD1306  
- Adafruit GFX  
- Adafruit Unified Sensor  
- DHT sensor  
- UniversalTelegramBot  
- ArduinoJson  
- WiFiClientSecure  

---

## Como Rodar

1. Clone
2. Crie secrets.h
3. Ajuste pinos
4. Compile e envie

---

## Roadmap

- API REST  
- Dashboard Web  
- Histórico  
- Alertas  
- Múltiplos sensores  

---

## Autor

**João Victor da Silva Moura**  
GitHub: https://github.com/juaocrl  

---

## Conecte-se comigo

<a href="https://www.linkedin.com/in/joao-victormoura/" target="_blank">
<img src="https://img.shields.io/badge/LinkedIn-Perfil%20Profissional-blue?style=for-the-badge&logo=linkedin">
</a>
