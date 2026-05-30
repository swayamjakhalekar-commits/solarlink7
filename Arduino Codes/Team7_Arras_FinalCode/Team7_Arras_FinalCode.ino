// ======================================================
// TEAM7 FINAL SOLAR TRACKER
// ARRAS FINAL VERSION
// ======================================================

#include <Wire.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Adafruit_INA219.h>
#include <Servo.h>

// ======================================================
// WIFI
// ======================================================

char ssid[] = //"YOUR SSID";
char pass[] = //"YOUR PASSWORD";

// ======================================================
// MQTT
// ======================================================

const char* mqtt_server =
"broker.hivemq.com";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

// ======================================================
// COMPONENTS
// ======================================================

Adafruit_INA219 ina219;

Servo servoH;
Servo servoV;

// ======================================================
// SERVO PINS
// ======================================================

const int SERVO_H_PIN = 9;
const int SERVO_V_PIN = 10;

// ======================================================
// SERVO SETTINGS
// ======================================================

const int CENTER_V = 90;

// smoother movement

const int STEP_DELAY_MIN = 15;
const int STEP_DELAY_MAX = 45;

const int EASE_FRACTION  = 35;

const int SETTLE = 700;

// ======================================================
// HOMING SETTINGS
// ======================================================

// smoother homing

const int HOME_STEP_DEG = 5;
const int HOME_STEP_MS  = 50;

// ======================================================
// SCAN SETTINGS
// ======================================================

// smoother scanning

const int H_SCAN_STEP = 5;
const int V_SCAN_STEP = 5;

const int H_RESCAN_MARGIN = 30;
const int V_RESCAN_MARGIN = 20;

const unsigned long SCAN_INTERVAL =
120000UL;

unsigned long lastScanTime = 0;

// ======================================================
// POSITION VARIABLES
// ======================================================

int currentH = 90;

// INTERNAL RANGE = -50 → +50

int currentV = 0;

// ======================================================
// POWER VARIABLES
// ======================================================

float voltage    = 0;
float current_mA = 0;
float power_mW   = 0;

// ======================================================
// MODE
// ======================================================

bool manualMode = false;

// ======================================================
// MQTT TOPICS
// ======================================================

const char* topic_h_angle =
"cesithmcoil2025/Team7/Arras/position/angle-h";

const char* topic_v_angle =
"cesithmcoil2025/Team7/Arras/position/angle-v";

const char* topic_voltage =
"cesithmcoil2025/Team7/Arras/panel-voltage";

const char* topic_current =
"cesithmcoil2025/Team7/Arras/panel-current";

const char* topic_power =
"cesithmcoil2025/Team7/Arras/panel-power";

const char* topic_status =
"cesithmcoil2025/Team7/Arras/status";

const char* control_h =
"cesithmcoil2025/Team7/Arras/control/angle-h";

const char* control_v =
"cesithmcoil2025/Team7/Arras/control/angle-v";

const char* control_mode =
"cesithmcoil2025/Team7/Arras/control/mode";

// ======================================================
// EASING
// ======================================================

int easeDelay(int moved, int total) {

  if (total == 0)
    return STEP_DELAY_MIN;

  int easeSteps =
    max(1,
    (total * EASE_FRACTION) / 100);

  int remaining = total - moved;

  if (moved < easeSteps) {

    return STEP_DELAY_MAX -
      ((STEP_DELAY_MAX -
      STEP_DELAY_MIN)
      * moved / easeSteps);
  }

  if (remaining < easeSteps) {

    return STEP_DELAY_MIN +
      ((STEP_DELAY_MAX -
      STEP_DELAY_MIN)
      * (easeSteps - remaining)
      / easeSteps);
  }

  return STEP_DELAY_MIN;
}

// ======================================================
// MOVE HORIZONTAL
// ======================================================

void moveH(int target) {

  target = constrain(target, 0, 180);

  if (target == currentH)
    return;

  int total =
    abs(target - currentH);

  int step =
    (target > currentH)
    ? 1 : -1;

  int moved = 0;

  while (currentH != target) {

    currentH += step;

    servoH.write(currentH);

    delay(
      easeDelay(moved++, total)
    );
  }
}

// ======================================================
// MOVE VERTICAL
// ======================================================

void moveV(int target) {

  target = constrain(target, -50, 50);

  if (target == currentV)
    return;

  int total =
    abs(target - currentV);

  int step =
    (target > currentV)
    ? 1 : -1;

  int moved = 0;

  while (currentV != target) {

    currentV += step;

    servoV.write(
      CENTER_V + currentV
    );

    delay(
      easeDelay(moved++, total)
    );
  }
}

// ======================================================
// SAFE HOME HORIZONTAL
// ======================================================

void safeHomeH() {

  Serial.println("Homing Horizontal Servo...");

  servoH.write(90);

  delay(500);

  for (int h = 90; h >= 0; h -= HOME_STEP_DEG) {

    servoH.write(h);

    Serial.print("H Position: ");
    Serial.println(h);

    delay(HOME_STEP_MS);
  }

  servoH.write(0);

  delay(300);

  currentH = 0;

  Serial.println("Horizontal Homing Complete");

  delay(700);
}

// ======================================================
// SAFE HOME VERTICAL
// ======================================================

void safeHomeV() {

  Serial.println("Homing Vertical Servo...");

  // stabilize at center first

  servoV.write(CENTER_V);

  delay(700);

  // smooth downward sweep

  for (int v = 0; v >= -50; v -= HOME_STEP_DEG) {

    servoV.write(CENTER_V + v);

    Serial.print("V Position: ");
    Serial.println(v + 50);

    delay(HOME_STEP_MS);
  }

  servoV.write(CENTER_V - 50);

  delay(300);

  currentV = -50;

  Serial.println("Vertical Homing Complete");

  delay(700);
}

// ======================================================
// POWER MEASUREMENT
// ======================================================

void measurePower() {

  voltage =
    ina219.getBusVoltage_V();

  current_mA =
    ina219.getCurrent_mA();

  power_mW =
    voltage * current_mA;
}

// ======================================================
// SERIAL MONITOR UPDATE
// ======================================================

void serialMonitorUpdate() {

  Serial.println("======================");

  Serial.print("MODE: ");

  Serial.println(
    manualMode ?
    "MANUAL" :
    "AUTO"
  );

  Serial.print("H-Angle: ");
  Serial.println(currentH);

  Serial.print("V-Angle: ");
  Serial.println(currentV + 50);

  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.println(" V");

  Serial.print("Current: ");
  Serial.print(current_mA);
  Serial.println(" mA");

  Serial.print("Power: ");
  Serial.print(power_mW);
  Serial.println(" mW");

  Serial.println("======================");
}

// ======================================================
// MQTT PUBLISH
// ======================================================

void publishData() {

  client.publish(
    topic_h_angle,
    String(currentH).c_str()
  );

  client.publish(
    topic_v_angle,
    String(currentV + 50).c_str()
  );

  client.publish(
    topic_voltage,
    String(voltage, 2).c_str()
  );

  client.publish(
    topic_current,
    String(current_mA, 2).c_str()
  );

  client.publish(
    topic_power,
    String(power_mW, 2).c_str()
  );

  client.publish(
    topic_status,
    manualMode ?
    "MANUAL MODE" :
    "AUTO MODE"
  );

  serialMonitorUpdate();
}

// ======================================================
// WIFI CONNECTION (DHCP FIXED)
// ======================================================

void connectWiFi() {

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, pass);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

    if (millis() - start > 15000) {

      Serial.println("\nRetrying WiFi...");

      WiFi.disconnect();

      delay(1000);

      WiFi.begin(ssid, pass);

      start = millis();
    }
  }

  while (WiFi.localIP() == IPAddress(0,0,0,0)) {

    Serial.println("Waiting for DHCP...");

    delay(500);
  }

  Serial.println("\nWiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Signal Strength: ");
  Serial.println(WiFi.RSSI());
}

// ======================================================
// MQTT CALLBACK
// ======================================================

void callback(
  char* topic,
  byte* payload,
  unsigned int length
) {

  String message = "";

  for (
    unsigned int i = 0;
    i < length;
    i++
  ) {

    message += (char)payload[i];
  }

  Serial.print("MQTT Message [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // MODE

  if (String(topic) == control_mode) {

    if (message == "AUTO")
      manualMode = false;

    if (message == "MANUAL")
      manualMode = true;
  }

  // HORIZONTAL

  if (String(topic) == control_h) {

    manualMode = true;

    moveH(message.toInt());
  }

  // VERTICAL

  if (String(topic) == control_v) {

    manualMode = true;

    int dashboardValue =
      message.toInt();

    int internalAngle =
      dashboardValue - 50;

    moveV(internalAngle);
  }
}

// ======================================================
// MQTT CONNECT
// ======================================================

void connectMQTT() {

  client.setServer(
    mqtt_server,
    1883
  );

  client.setCallback(callback);

  while (!client.connected()) {

    Serial.println("Connecting to MQTT Broker...");

    String clientId =
      "Team7Arras-" +
      String(random(1000, 9999));

    Serial.print("Client ID: ");
    Serial.println(clientId);

    if (client.connect(clientId.c_str())) {

      Serial.println("MQTT Connected");
      Serial.println("Team7 Online");

      client.subscribe(control_h);
      client.subscribe(control_v);
      client.subscribe(control_mode);

      Serial.println("MQTT Topics Subscribed");

    } else {

      Serial.print("MQTT Failed, rc=");
      Serial.println(client.state());

      Serial.println("Retrying in 3 sec...");

      delay(3000);
    }
  }
}

// ======================================================
// HORIZONTAL SCAN
// ======================================================

void scanBestHorizontal(bool fullScan = false) {

  Serial.println("Searching Best Horizontal Angle...");

  int scanMin =
    fullScan ?
    0 :
    max(0, currentH - H_RESCAN_MARGIN);

  int scanMax =
    fullScan ?
    180 :
    min(180, currentH + H_RESCAN_MARGIN);

  float bestPower = -1;

  int bestAngle = currentH;

  for (int angle = scanMin;
       angle <= scanMax;
       angle += H_SCAN_STEP) {

    moveH(angle);

    delay(SETTLE);

    measurePower();

    Serial.print("H Angle: ");
    Serial.print(angle);

    Serial.print(" | Power: ");
    Serial.print(power_mW);
    Serial.println(" mW");

    if (power_mW > bestPower) {

      bestPower = power_mW;

      bestAngle = angle;
    }
  }

  moveH(bestAngle);

  delay(500);
}

// ======================================================
// VERTICAL SCAN
// ======================================================

void scanBestVertical(bool fullScan = false) {

  Serial.println("Searching Best Vertical Angle...");

  int scanMin =
    fullScan ?
    -50 :
    max(-50, currentV - V_RESCAN_MARGIN);

  int scanMax =
    fullScan ?
    50 :
    min(50, currentV + V_RESCAN_MARGIN);

  float bestPower = -1;

  int bestAngle = currentV;

  for (int angle = scanMin;
       angle <= scanMax;
       angle += V_SCAN_STEP) {

    moveV(angle);

    delay(SETTLE);

    measurePower();

    Serial.print("V Angle: ");
    Serial.print(angle + 50);

    Serial.print(" | Power: ");
    Serial.print(power_mW);
    Serial.println(" mW");

    if (power_mW > bestPower) {

      bestPower = power_mW;

      bestAngle = angle;
    }
  }

  moveV(bestAngle);

  delay(500);
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println("TEAM7 SOLAR TRACKER");
  Serial.println("ARRAS FINAL SMOOTH VERSION");

  randomSeed(millis());

  servoH.attach(SERVO_H_PIN);
  servoV.attach(SERVO_V_PIN);

  delay(700);

  if (!ina219.begin()) {

    Serial.println("INA219 ERROR");

    while(1);
  }

  Serial.println("INA219 Connected");

  safeHomeH();

  safeHomeV();

  connectWiFi();

  connectMQTT();

  scanBestHorizontal(true);

  scanBestVertical(true);

  lastScanTime = millis();
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi Lost!");

    connectWiFi();
  }

  if (!client.connected()) {

    Serial.println("MQTT Lost!");

    connectMQTT();
  }

  client.loop();

  // MANUAL MODE

  if (manualMode) {

    measurePower();

    publishData();

    delay(500);

    return;
  }

  // AUTO MODE

  if (
    millis() - lastScanTime
    >= SCAN_INTERVAL
  ) {

    scanBestHorizontal(false);

    scanBestVertical(false);

    lastScanTime = millis();
  }

  measurePower();

  publishData();

  delay(1000);
}