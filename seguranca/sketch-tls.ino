// VARIANTE PREPARADA, NAO ENSAIADA COM BROKER REAL
// Demonstrador academico: AD8232 SIMULADO -> ADC ESP32 -> MQTT.
// Nao realiza medicao em pessoas. Broker publico apenas para teste.
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "secrets.h"

constexpr int LED = 2;
constexpr int ECG = 34;   // ADC1, leitura do componente externo
constexpr int LO_PLUS = 32;
constexpr int LO_MINUS = 33;
constexpr uint32_t INTERVALO_US = 4000;
constexpr int TAM_LOTE = 25;
const char *TOPICO = "aula/ecg/esp32-84729/amostras";
WiFiClientSecure wifi;
bool tlsReady = false;
PubSubClient mqtt(wifi);
char clientId[48];
uint32_t nextSample, indice = 0, primeiroIndice = 0;
uint32_t lastMqtt = 0, lastWifi = 0, perdidas = 0;
int amostras[TAM_LOTE], quantidade = 0;
bool loteValido = true;

void publicarLote() {
  if (!mqtt.connected()) { quantidade = 0; return; }
  char msg[768];
  int n = snprintf(msg, sizeof(msg),
    "{\"dispositivo\":\"esp32-84729\",\"indice\":%lu,\"intervalo_us\":4000,"
    "\"eletrodos\":\"%s\",\"origem\":\"ad8232-simulado\",\"perdidas_temporizacao\":%lu,\"amostras\":[",
    (unsigned long)primeiroIndice, loteValido ? "conectados" : "desconectados",
    (unsigned long)perdidas);
  for (int i = 0; i < quantidade && n > 0 && n < (int)sizeof(msg); i++) {
    n += snprintf(msg + n, sizeof(msg) - n, "%s%d", i ? "," : "", amostras[i]);
  }
  if (n > 0 && n + 3 < (int)sizeof(msg)) {
    snprintf(msg + n, sizeof(msg) - n, "]}");
    bool ok = mqtt.publish(TOPICO, msg, false);
    Serial.printf("MQTT %s | indice=%lu | ADC=%d | eletrodos=%s | perdas_tempo=%lu\n",
      ok ? "OK" : "FALHA", (unsigned long)primeiroIndice,
      amostras[quantidade-1], loteValido ? "conectados" : "desconectados",
      (unsigned long)perdidas);
  }
  quantidade = 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(ECG, INPUT);
  pinMode(LO_PLUS, INPUT);
  pinMode(LO_MINUS, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(ECG, ADC_11db);
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "", 6);
  tlsReady = strlen(MQTT_HOST) > 0 && strlen(MQTT_USERNAME) > 0 && strlen(MQTT_PASSWORD) > 0 && strlen(MQTT_CA_CERT) > 0;
  if (tlsReady) { wifi.setCACert(MQTT_CA_CERT); mqtt.setServer(MQTT_HOST, MQTT_PORT); }
  else Serial.println("TLS pendente: configure secrets.h; MQTT bloqueado");
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  mqtt.setBufferSize(1024);
  mqtt.setSocketTimeout(1);
  uint64_t mac = ESP.getEfuseMac();
  snprintf(clientId, sizeof(clientId), "ecg84729-%08lx-%08lx", (unsigned long)mac, (unsigned long)esp_random());
  nextSample = micros() + INTERVALO_US;
  Serial.println("AD8232 SIMULADO: OUTPUT -> GPIO34; LOP/ LON -> GPIO32/33; LED GPIO2");
}

void loop() {
  uint32_t nowMs = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (nowMs - lastWifi >= 5000) {
      lastWifi = nowMs;
      WiFi.reconnect();
    }
  } else if (tlsReady && time(nullptr) > 1700000000 && !mqtt.connected() && (lastMqtt == 0 || nowMs - lastMqtt >= 5000)) {
    lastMqtt = nowMs;
    Serial.printf("Conectando MQTT: %s\n", clientId);
    mqtt.connect(clientId, MQTT_USERNAME, MQTT_PASSWORD);
  }
  if (mqtt.connected()) mqtt.loop();

  uint32_t now = micros();
  if ((int32_t)(now - nextSample) < 0) return;
  uint32_t missed = (now - nextSample) / INTERVALO_US;
  if (missed) {
    perdidas += missed;
    indice += missed;
    quantidade = 0; // nao misturar amostras separadas por lacuna no mesmo lote
  }
  nextSample += (missed + 1) * INTERVALO_US;
  bool connected = digitalRead(LO_PLUS) == LOW && digitalRead(LO_MINUS) == LOW;
  int adc = analogRead(ECG); // O sinal vem do modulo, nao de formula no ESP32.
  // Wokwi possui ADC simplificado: limiar didatico de pico, nao deteccao clinica.
  digitalWrite(LED, !connected ? HIGH : (!mqtt.connected() ? ((nowMs / 500) % 2) : adc > 2000));
  if (quantidade && connected != loteValido) quantidade = 0;
  if (!quantidade) { primeiroIndice = indice; loteValido = connected; }
  amostras[quantidade++] = adc;
  indice++;
  if (quantidade == TAM_LOTE) publicarLote();
}
