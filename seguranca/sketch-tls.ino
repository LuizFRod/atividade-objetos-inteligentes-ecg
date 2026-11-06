// Demonstrador academico: AD8232 SIMULADO -> ADC ESP32 -> MQTT.
// Nao realiza medicao em pessoas. MQTT com TLS e autenticacao.
// A senha e recebida pelo monitor serial apenas em RAM; nunca salve no projeto.
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
constexpr char MQTT_HOST[] = "i066e913.ala.us-east-1.emqxsl.com";
constexpr uint16_t MQTT_PORT = 8883;
constexpr char MQTT_USERNAME[] = "esp32-84729";
const char MQTT_CA_CERT[] = R"EMQXCA(-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----)EMQXCA";
char mqttPassword[129] = {};
size_t passwordLength = 0;
bool passwordReady = false;
bool passwordOverflow = false;

void pedirSenha() {
  memset(mqttPassword, 0, sizeof(mqttPassword));
  passwordLength = 0;
  passwordReady = false;
  passwordOverflow = false;
  Serial.println("SENHA MQTT: usuario esp32-84729. Digite no monitor serial e pressione Enter.");
  Serial.println("A senha nao e exibida nem gravada nos arquivos; vale somente nesta execucao.");
}

void lerSenhaSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (passwordReady) continue;
    if (c == '\r' || c == '\n') {
      if (passwordOverflow) { Serial.println("Senha muito longa. Tente novamente."); pedirSenha(); }
      else if (passwordLength) { mqttPassword[passwordLength] = 0; passwordReady = true; Serial.println("Credencial recebida em RAM. Aguardando Wi-Fi e horario NTP."); }
    } else if (c == '\b' || c == 127) {
      if (passwordLength) mqttPassword[--passwordLength] = 0;
    } else if (passwordLength < sizeof(mqttPassword) - 1) {
      mqttPassword[passwordLength++] = c;
    } else { passwordOverflow = true; }
  }
}

constexpr int LED = 2;
constexpr int ECG = 34;   // ADC1, leitura do componente externo
constexpr int LO_PLUS = 32;
constexpr int LO_MINUS = 33;
constexpr uint32_t INTERVALO_US = 4000;
constexpr int TAM_LOTE = 25;
const char *TOPICO = "aula/ecg/esp32-84729/amostras";
WiFiClientSecure wifi;
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
    "\"eletrodos\":\"%s\",\"origem\":\"ad8232-simulado\",\"transporte\":\"mqtts\",\"sessao\":\"%s\",\"perdidas_temporizacao\":%lu,\"amostras\":[",
    (unsigned long)primeiroIndice, loteValido ? "conectados" : "desconectados",
    clientId, (unsigned long)perdidas);
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
  wifi.setCACert(MQTT_CA_CERT);
  wifi.setHandshakeTimeout(20);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  pedirSenha();
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  mqtt.setBufferSize(1024);
  mqtt.setSocketTimeout(5);
  uint64_t mac = ESP.getEfuseMac();
  snprintf(clientId, sizeof(clientId), "ecg84729-%08lx-%08lx", (unsigned long)mac, (unsigned long)esp_random());
  nextSample = micros() + INTERVALO_US;
  Serial.println("AD8232 SIMULADO: OUTPUT -> GPIO34; LOP/ LON -> GPIO32/33; LED GPIO2");
}

void loop() {
  lerSenhaSerial();
  uint32_t nowMs = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (nowMs - lastWifi >= 5000) {
      lastWifi = nowMs;
      WiFi.reconnect();
    }
  } else if (passwordReady && time(nullptr) > 1700000000 && !mqtt.connected() && (lastMqtt == 0 || nowMs - lastMqtt >= 5000)) {
    lastMqtt = nowMs;
    Serial.printf("Conectando MQTT: %s\n", clientId);
    if (mqtt.connect(clientId, MQTT_USERNAME, mqttPassword)) {
      Serial.println("MQTTS CONECTADO: TLS com CA verificada e autenticacao aceita.");
    } else {
      Serial.printf("Conexao MQTT falhou: estado=%d. Sem alternativa em texto claro.\n", mqtt.state());
      if (mqtt.state() == 4 || mqtt.state() == 5) pedirSenha();
    }
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
