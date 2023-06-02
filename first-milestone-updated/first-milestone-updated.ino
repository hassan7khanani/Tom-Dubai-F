#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#define VIBRATION_PIN 33 
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
#include <Ticker.h>
Ticker timer;
bool publishFlag = false;
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

#define WIFI_SSID "Virus"
#define WIFI_PASSWORD "12345678"

#define MQTT_SERVER "3.132.130.234"
#define MQTT_PORT 4040
#define MQTT_USERNAME "petCeption"
#define MQTT_PASSWORD "D0dg3rs420#"

// #define MQTT_SERVER "192.168.0.250"
// #define MQTT_PORT 1883
// #define MQTT_USERNAME "mqtt_user"
// #define MQTT_PASSWORD " mqtt@pass"



#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

// Vibration times
unsigned long vibrationHighStart = 0;
unsigned long vibrationLowStart = 0;

void publishCallback()
{
  publishFlag = true;
}

void setup_wifi() {
  delay(10);
  // Connect to WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Wifi Connected");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  delay(1000);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT COnnected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      delay(5000);
    }
  }
}



void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(VIBRATION_PIN, INPUT);

  // Start the DS18B20 sensor
  sensors.begin();
  dht.begin();
  timer.attach(10, publishCallback); 

  // Enable wakeup on HIGH for vibration pin
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);

  // Check the reason for waking up
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Woke up due to vibration.");
    // Do whatever needs to be done after waking up due to vibration
    setup_wifi();
    reconnect();
  } else {
    Serial.println("Woke up for some other reason or a fresh boot up.");
    // Do whatever needs to be done after waking up for other reasons or a fresh boot up
    setup_wifi();
    reconnect();
  }
}

void loop() {
  int vibration = digitalRead(VIBRATION_PIN);
  Serial.print("vibration -----    ");
  Serial.println(vibration);
  delay(500);
  if (vibration == HIGH) {
    vibrationLowStart = 0;
    if (vibrationHighStart == 0) 
    {
      vibrationHighStart = millis();
    } 
    else if ((millis() - vibrationHighStart) >= 4000) 
    {
      sensors.requestTemperatures();
      float temperatureC = sensors.getTempCByIndex(0);
      float temperatureF = sensors.getTempFByIndex(0);

      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      if (publishFlag)
  {
      StaticJsonDocument<200> jsonDocument;
      jsonDocument["dht_temp"] = temperature;
      jsonDocument["dht_humidity"] = humidity;
      jsonDocument["ds_tempC"] = temperatureC;
      jsonDocument["ds_tempF"] = temperatureF;
      jsonDocument["pump_status"] = "ON";
      String jsonStr;
      serializeJson(jsonDocument, jsonStr);
      client.publish("reading", jsonStr.c_str());
      Serial.println("published");
      Serial.println("============================Published on mqtt =====================");
    publishFlag = false; // Reset the flag
  }




      delay(1000); 
      client.loop();
      vibrationHighStart = 0;
    }
  } else {
    vibrationHighStart = 0;
    if (vibrationLowStart == 0) {
      vibrationLowStart = millis();
    } else if ((millis() - vibrationLowStart) >= 20000) {
      // Go to sleep
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      Serial.println("Entering deep sleep");
      esp_deep_sleep_start();
    }
  }
}




