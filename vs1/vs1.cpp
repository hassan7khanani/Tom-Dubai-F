#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <SW420.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

#define WIFI_SSID "Virus"
#define WIFI_PASSWORD "12345678"

#define MQTT_SERVER "3.132.130.234"
#define MQTT_PORT 4040
#define MQTT_USERNAME "petCeption"
#define MQTT_PASSWORD "D0dg3rs420#"

#define DHTPIN 4       // DHT11 data pin
#define DHTTYPE DHT11  // DHT11 sensor type

#define SW420_PIN 5  // SW420 sensor pin
SW420 sw420(SW420_PIN);

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

bool motionDetected = false;
unsigned long vibrationStartTime = 0;
unsigned long noVibrationStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Start the DS18B20 sensor
  sensors.begin();
  dht.begin();
  Serial.println("\nConnected to Wi-Fi");
  setupMqtt();
}

void setupMqtt() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("\nConnected to MQTT");
    } 
    
    else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");

      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages
}

void loop() {
  if (!motionDetected && !sw420.motionDetected()) {
    // No motion detected
    if (millis() - noVibrationStartTime >= 20000) {
      // If no vibration for 20 seconds, go back to deep sleep
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      esp_sleep_enable_timer_wakeup(1 * 60 * 1000000); // 2 minutes
      Serial.println("Going to deep sleep");
      esp_deep_sleep_start();
    }
  } else {
    // Motion detected or previously detected
    if (!motionDetected) {
      // First time motion detected
      vibrationStartTime = millis();
    }

    motionDetected = true;

    if (millis() - vibrationStartTime >= 4000) {
      // Continuous vibration detected for 4 seconds
      readings();

      delay(1000);  // Wait for a moment to ensure the message is sent
      client.loop();

      // Reset variables
      motionDetected = false;
      noVibrationStartTime = millis();
    }
  }
}

void readings() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
  delay(1000);
  // Read DHT11 sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Create a JSON document
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["dht_temp"] = temperature;
  jsonDocument["dht_humidity"] = humidity;
  jsonDocument["ds_tempC"] = temperatureC;
  jsonDocument["ds_tempF"] = temperatureF;

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(jsonDocument, jsonStr);

  // Publish the JSON string to MQTT topic
  client.publish("readings", jsonStr.c_str());

  Serial.println("Message published to MQTT topic 'readings'");
  Serial.println("");
}
