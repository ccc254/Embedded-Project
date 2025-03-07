#include <WiFi.h>
#include <MQTT.h>
#include <map>
#include <Wire.h>
#include <LCD_I2C.h>

#define LED 0
#define BUTTON 23

#define DOT_DURATION 300  
#define DASH_DURATION 700  
#define SPACE_BETWEEN_SYMBOLS 300  
#define SPACE_BETWEEN_LETTERS 1000 

const char ssid[] = "Chanom";
const char pass[] = "chanon2547";

const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group27/command";
const char mqtt_client_id[] = "clientId-NTAQuNGdQ5"; 
const int MQTT_PORT = 1883;
String lastPayload = "";

LCD_I2C lcd(0x27, 16, 2);
WiFiClient net;
MQTTClient client;

std::map<char, String> morseMap = {
  {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
  {'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},   {'J', ".---"},
  {'K', "-.-"},  {'L', ".-.."}, {'M', "--"},   {'N', "-."},   {'O', "---"},
  {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
  {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
  {'Z', "--.."}, {' ', " "} 
};

void connect() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");

  Serial.print("Connecting to MQTT broker...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nMQTT connected!");

  client.subscribe(mqtt_topic);
  Serial.println("Subscribed to topic: " + String(mqtt_topic));
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  
  lcd.begin();
  lcd.backlight();
  displayLCD("Connecting...");

  client.begin(mqtt_broker, MQTT_PORT, net);
  client.onMessage(messageReceived);

  connect();
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);
}

void displayLCD(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void blinkLED(int duration) {
  digitalWrite(LED, HIGH);
  delay(duration);
  digitalWrite(LED, LOW);
}

void showMorseOnLCD(String morseCode) {
  displayLCD("Morse Output:");
  lcd.setCursor(0, 1);
  
  for (int i = 0; i < morseCode.length(); i++) {
    lcd.print(morseCode[i]);
    
    if (morseCode[i] == '.') {
      blinkLED(DOT_DURATION);
    } else if (morseCode[i] == '-') {
      blinkLED(DASH_DURATION);
    }
    delay(SPACE_BETWEEN_SYMBOLS);
  }
  delay(SPACE_BETWEEN_LETTERS);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Received: " + topic + " - " + payload);

  if (payload == lastPayload) {
    Serial.println("Duplicate message, ignoring...");
    return;
  }

  lastPayload = payload;
  payload.toUpperCase();
  displayLCD("Msg: " + payload);

  for (int i = 0; i < payload.length(); i++) {
    if (morseMap.find(payload[i]) != morseMap.end()) {
      showMorseOnLCD(morseMap[payload[i]]);
    }
  }
}

void loop() {
  client.loop();
  delay(10);

  if (!client.connected()) {
    connect();
  }

  static unsigned long pressStartTime = 0;
  static bool isPressing = false;
  static String morseSequence = "";

  if (digitalRead(BUTTON) == LOW) { 
    if (!isPressing) {
      pressStartTime = millis();
      isPressing = true;
      digitalWrite(LED, HIGH);
    }
  } else {
    if (isPressing) {
      unsigned long pressDuration = millis() - pressStartTime;
      isPressing = false;
      digitalWrite(LED, LOW);

      if (pressDuration <= DOT_DURATION) {
        morseSequence += ".";
      } else if (pressDuration <= DASH_DURATION) {
        morseSequence += "-";
}

      displayLCD("Morse Output:");
      lcd.setCursor(0, 1);
      lcd.print(morseSequence);
    }
  }

  static unsigned long lastPressTime = 0;
  if (isPressing) {
    lastPressTime = millis();
  } else if (morseSequence.length() > 0 && millis() - lastPressTime > SPACE_BETWEEN_LETTERS) {
    displayLCD("Morse Output:");
    morseSequence = "";
  }
}
