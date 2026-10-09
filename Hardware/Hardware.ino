#include <WiFi.h>
#include <PubSubClient.h>
#include <sys/time.h>

// ---------- SETTINGS ----------
const char* WIFI_SSID = "Gowri's";
const char* WIFI_PASS = "hehehehe";
const char* MQTT_BROKER = "broker.hivemq.com";
const char* TOPIC = "aquawatch/fswd-0201/ph";   // must match the dashboard
// ------------------------------

#define PH_PIN 34
const int GREEN = 25, RED = 26, BUZ = 27;

// PROVISIONAL calibration (uncalibrated)
const float V_NEUTRAL = 1.90;
const float SLOPE = 0.30;
const float PH_LOW = 6.5, PH_HIGH = 8.5;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
unsigned long lastRead = 0;
bool timeSynced = false;

float readModuleVoltage() {
  long sum = 0;
  for (int i = 0; i < 30; i++) { sum += analogRead(PH_PIN); delay(10); }
  float raw = sum / 30.0;
  return raw * 3.3 / 4095.0 * 2.0;
}

float voltageToPH(float v) {
  float ph = 7.0 + (V_NEUTRAL - v) / SLOPE;
  return constrain(ph, 0.0, 14.0);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" connected");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // sync the clock
  timeSynced = false;
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to broker... ");
    String id = "aquawatch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(id.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, code ");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(GREEN, OUTPUT); pinMode(RED, OUTPUT); pinMode(BUZ, OUTPUT);
  mqtt.setServer(MQTT_BROKER, 1883);
}

void loop() {
  connectWiFi();
  connectMQTT();
  mqtt.loop();

  if (millis() - lastRead >= 1000) {
    lastRead = millis();
    float v = readModuleVoltage();
    float ph = voltageToPH(v);
    bool safe = (ph >= PH_LOW && ph <= PH_HIGH);

    digitalWrite(GREEN, safe ? HIGH : LOW);
    digitalWrite(RED, safe ? LOW : HIGH);
    digitalWrite(BUZ, safe ? LOW : HIGH);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long ts = 0;
    if (tv.tv_sec > 1700000000) {            // clock is synced
      ts = (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000;
      timeSynced = true;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "{\"voltage\":%.3f,\"ph\":%.2f,\"safe\":%s,\"ts\":%lld}",
             v, ph, safe ? "true" : "false", ts);
    mqtt.publish(TOPIC, msg);

    Serial.printf("module=%.3f V  pH=%.2f  %s  (sent%s)\n", v, ph,
                  safe ? "SAFE" : "ALERT", timeSynced ? ", time ok" : ", no time yet");
  }
}