#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PH_PIN 34
#define TURBIDITY_PIN 35
#define TDS_PIN 32

#define GREEN_LED 25
#define RED_LED 26
#define BUZZER 27

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* MQTT_TOPIC = "aquawatch/demo/waterquality";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void connectWiFi() {
    Serial.print("Connecting to WiFi");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT...");

        String clientId = "AquaWatch-";
        clientId += String(random(0xffff), HEX);

        if (mqttClient.connect(clientId.c_str())) {
            Serial.println("connected");
        } else {
            Serial.print("failed, state=");
            Serial.println(mqttClient.state());
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    Wire.begin(21, 22);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED failed");
    }

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(20, 20);
    display.println("AquaWatch");
    display.setCursor(15, 35);
    display.println("Connecting...");
    display.display();

    connectWiFi();

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

    connectMQTT();

    display.clearDisplay();
    display.setCursor(20, 25);
    display.println("AquaWatch");
    display.setCursor(15, 40);
    display.println("Online");
    display.display();

    delay(1500);
}

void loop() {

    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();

    int phRaw = analogRead(PH_PIN);
    int turbidityRaw = analogRead(TURBIDITY_PIN);
    int tdsRaw = analogRead(TDS_PIN);

    float pH = 4.0 + (phRaw / 4095.0) * 6.0;
    float turbidity = (turbidityRaw / 4095.0) * 100.0;
    float tds = (tdsRaw / 4095.0) * 1000.0;

    bool alert = false;

    if (pH < 6.5 || pH > 8.5) {
        alert = true;
    }

    if (turbidity > 5.0) {
        alert = true;
    }

    if (tds > 500.0) {
        alert = true;
    }

    if (alert) {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        tone(BUZZER, 1000);
    } else {
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);
        noTone(BUZZER);
    }

    String status = alert ? "ALERT" : "SAFE";

    String jsonData = "{";
    jsonData += "\"ph\":" + String(pH, 2) + ",";
    jsonData += "\"turbidity\":" + String(turbidity, 1) + ",";
    jsonData += "\"tds\":" + String(tds, 1) + ",";
    jsonData += "\"status\":\"" + status + "\"";
    jsonData += "}";

    mqttClient.publish(MQTT_TOPIC, jsonData.c_str());

    Serial.println("----- AquaWatch -----");
    Serial.print("pH: ");
    Serial.println(pH, 2);

    Serial.print("Turbidity: ");
    Serial.print(turbidity, 1);
    Serial.println(" NTU");

    Serial.print("TDS: ");
    Serial.print(tds, 1);
    Serial.println(" ppm");

    Serial.print("Status: ");
    Serial.println(status);

    Serial.print("MQTT: ");
    Serial.println(jsonData);

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("AquaWatch");

    display.setCursor(0, 15);
    display.print("pH: ");
    display.println(pH, 2);

    display.setCursor(0, 28);
    display.print("Turb: ");
    display.print(turbidity, 1);
    display.println(" NTU");

    display.setCursor(0, 41);
    display.print("TDS: ");
    display.print(tds, 1);
    display.println(" ppm");

    display.setCursor(0, 54);

    if (alert) {
        display.println("STATUS: ALERT");
    } else {
        display.println("STATUS: SAFE");
    }

    display.display();

    delay(2000);
}