#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "Casa Vader 1466";
const char *password = "LucianoHouse1";
const String writeAPIKey = "SXABYBTHUASWY5GE";

const int led1Pin = D1;
const int led2Pin = D2;
const int buzzerPin = D5;
const int sensorIRPin = D3;
const int buttonPin = D4;

int buttonState = 0;
int lastButtonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool systemStarted = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conectado a la red WiFi.");

  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(sensorIRPin, INPUT);
  pinMode(buttonPin, INPUT);

  Serial.println("Sistema Listo Para Funcionar");

  int field3Value = getField3FromThingSpeak();
  if (field3Value == 1) {
    activateBalizaBuzzer();
  } else {
    deactivateBalizaBuzzer();
  }

  delay(5000);
}

void loop() {
  int sensorValue = digitalRead(sensorIRPin);

  if (sensorValue == HIGH) {
    Serial.println("Obstáculo detectado");
    sendToThingSpeakField1(1);
  } else {
    Serial.println("Sin obstáculo");
    sendToThingSpeakField1(0);
  }

  int field3Value = getField3FromThingSpeak();

  if (field3Value == 1 && !systemStarted) {
    activateBalizaBuzzer();
    delay(4000);
    deactivateBalizaBuzzer();
    systemStarted = true;
    sendToThingSpeakField2(1);
  } else if (field3Value == 0 && systemStarted) {
    deactivateBalizaBuzzer();
    systemStarted = false;
    sendToThingSpeakField2(0);
  }

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        Serial.println("Sistema de inicio activado");
        activateSystemStart();
      }
    }
  }

  lastButtonState = reading;

  delay(5000);
}

void sendToThingSpeakField1(int value) {
  WiFiClient client;
  String url = "http://api.thingspeak.com/update?api_key=" + writeAPIKey + "&field1=" + String(value);

  HTTPClient http;
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Error al actualizar field1 en ThingSpeak.");
  }

  http.end();
}

int getField3FromThingSpeak() {
  WiFiClient client;
  String url = "http://api.thingspeak.com/channels/2375253/feeds/last.json?api_key=" + writeAPIKey;

  HTTPClient http;
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, http.getString());

    int field3Value = jsonDoc["field3"];
    return field3Value;
  } else {
    Serial.println("Error al leer field3 desde ThingSpeak.");
    return 0;
  }
}

void activateBalizaBuzzer() {
  digitalWrite(led1Pin, HIGH);
  digitalWrite(led2Pin, HIGH);
  digitalWrite(buzzerPin, HIGH);
}

void deactivateBalizaBuzzer() {
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(buzzerPin, LOW);
}

void activateSystemStart() {
  for (int i = 0; i < 2; ++i) {
    activateBalizaBuzzer();
    delay(1000);
    deactivateBalizaBuzzer();
    delay(1000);
  }

  sendToThingSpeakField2(1);
}

void sendToThingSpeakField2(int value) {
  WiFiClient client;
  String url = "http://api.thingspeak.com/update?api_key=" + writeAPIKey + "&field2=" + String(value);

  HTTPClient http;
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Error al actualizar field2 en ThingSpeak.");
  }

  http.end();
}
