#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

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

      // Subscribe to a topic if needed
      // client.subscribe("topic");
    } else {
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

  // Publish the readings to MQTT topics
  char tempStr[6];
  char humStr[6];
  sprintf(tempStr, "%.1f", temperature);
  sprintf(humStr, "%.1f", humidity);

  client.publish("temperature", tempStr);
  client.publish("humidity", humStr);
  Serial.println("message published to MQTT topics");
  Serial.println("");

  delay(1000);  // Wait for a moment to ensure the messages are sent

  client.loop();

  // // Disconnect Wi-Fi and go into deep sleep mode for 2 minutes
   WiFi.disconnect(true);
   WiFi.mode(WIFI_OFF);
   esp_sleep_enable_timer_wakeup(1 * 60 * 1000000); // 2 minutes
   Serial.println("going to deep sleep");
   esp_deep_sleep_start();
}
