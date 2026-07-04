#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ---------------- WIFI ----------------
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ---------------- MQTT ----------------
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- OLED ----------------
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ---------------- PINS ----------------
#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27
#define BUZZER 14

// ---------------- WIFI ----------------
void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// ---------------- MQTT ----------------
void reconnect() {
  while (!client.connected()) {
    client.connect("TelecomTowerESP32");
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  setupWiFi();
  client.setServer(mqtt_server, 1883);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

// ---------------- LOOP ----------------
void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  int battery = random(11, 15);
  int temp = random(30, 85);
  int rssi = random(-95, -40);

  String payload = String(battery) + "," + String(temp) + "," + String(rssi);
  client.publish("telecom/tower/data", payload.c_str());

  // ---------------- STATUS LOGIC ----------------
  String status = "GOOD";

  if (battery < 12 || temp > 70 || rssi < -80) {
    status = "CRITICAL";

    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);

  } else if (temp > 60 || rssi < -70) {
    status = "WARNING";

    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, LOW);

  } else {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  }

  // ---------------- OLED DISPLAY ----------------
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("TELECOM TOWER");

  display.setCursor(0, 15);
  display.print("Battery: ");
  display.print(battery);
  display.println("V");

  display.setCursor(0, 25);
  display.print("Temp: ");
  display.print(temp);
  display.println("C");

  display.setCursor(0, 35);
  display.print("RSSI: ");
  display.print(rssi);
  display.println("dBm");

  display.setCursor(0, 50);
  display.print("STATUS: ");
  display.println(status);

  display.display();
  client.setKeepAlive(60);

  delay(3000);
}