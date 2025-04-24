#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 15           // Pin connected to the DHT11 data pin
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 34 // Analog pin for soil moisture sensor
#define WIFI_LED_PIN 26      // Pin for WiFi LED
#define BUZZER_PIN 25        // Pin connected to the buzzer

DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
unsigned long previousPrintMillis = 0;
unsigned long previousBuzzerMillis = 0;
const long interval = 5000;    // 5 seconds for backend updates
const long printInterval = 5000; // 5 seconds for Serial Monitor updates
const long buzzerInterval = 1000; // 5 seconds for buzzer checks

// Wi-Fi credentials
const char* ssid = "Dialog 4G 411";      
const char* password = "8AEF74bF";       
const char* serverUrl = "http://192.168.8.103:8080/PlantSystem/api/sensors/data";



void sendDataToBackend(float temperature, float humidity, float soilMoistureValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    String payload = "{";
    payload += "\"temperature\": " + String(temperature, 2) + ",";
    payload += "\"humidity\": " + String(humidity, 2) + ",";
    payload += "\"soil_moisture\": " + String(soilMoistureValue);
    payload += "}";

    Serial.println("Sending payload:");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server: " + response);
    } else {
      Serial.println("Error sending data to server: " + String(httpResponseCode));
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Initialize pins
  pinMode(WIFI_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    digitalWrite(WIFI_LED_PIN, HIGH);
    delay(100);
    digitalWrite(WIFI_LED_PIN, LOW);
    delay(100);
  }
  Serial.println("Connected to WiFi");
  digitalWrite(WIFI_LED_PIN, HIGH); 
}

void loop() {
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);

 
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }


  unsigned long currentMillis = millis();
  if (currentMillis - previousPrintMillis >= printInterval) {
    previousPrintMillis = currentMillis;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoistureValue);

    if (soilMoistureValue < 3500) { 
      Serial.println("Soil moisture is within standard level.");
    } else {
      Serial.println("Soil is Dry.");
    }
  }

 
  if (currentMillis - previousBuzzerMillis >= buzzerInterval) {
    previousBuzzerMillis = currentMillis;

    if (soilMoistureValue < 3500) {
      digitalWrite(BUZZER_PIN, LOW); 
    } else {
     
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100); 
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendDataToBackend(temperature, humidity, soilMoistureValue);
  }

  
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_LED_PIN, HIGH); 
  } else {
    digitalWrite(WIFI_LED_PIN, LOW); 
  }
}
