#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h> // Include the Arduino JSON library

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;
// GPIO where the S2420 is connected to
const int vibrationSensorPin = 23;

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

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long vibrationStartTime = 0;
unsigned long vibrationEndTime = 0;
unsigned long noVibrationStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(vibrationSensorPin, INPUT);

  // Start the DS18B20 sensor
  sensors.begin();
  dht.begin();
  setupMqtt();
}

void setupMqtt() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("Connected to MQTT");
    // Subscribe to a topic if needed
    // client.subscribe("topic");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
}

void disconnectWifi() {
  // Disconnect Wi-Fi and go into deep sleep mode for 20 seconds
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_sleep_enable_timer_wakeup(20 * 1000000);
  Serial.println("Going to deep sleep");
  esp_deep_sleep_start();
}

void loop() {

  Serial.println("In loop-------------");
  bool vibration = digitalRead(vibrationSensorPin);
  unsigned long currentTime = millis();

  if (vibration) {
    if (vibrationStartTime == 0) {
      vibrationStartTime = currentTime;
    } else if (currentTime - vibrationStartTime >= 4000) {
      Serial.println("####################vibration detected#########################################");
      connectWifi();
      publishSensorReadings();
      vibrationStartTime = 0;
    }
    noVibrationStartTime = 0;
  } else {
    if (noVibrationStartTime == 0) {
      noVibrationStartTime = currentTime;
    } else if (currentTime - noVibrationStartTime >= 20000) {
      Serial.println("--------------No vibration detected---------------------------");

      disconnectWifi();

    }
  }
}

void publishSensorReadings() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  // Read DHT11 sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

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
  client.publish("reading", jsonStr.c_str());

  Serial.println("Message published to MQTT topic 'reading'");
}
